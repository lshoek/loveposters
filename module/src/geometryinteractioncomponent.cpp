// Local Includes
#include "geometryinteractioncomponent.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <entity.h>
#include <inputservice.h>
#include <renderservice.h>
#include <triangleiterator.h>
#include <renderglobals.h>
#include <meshutils.h>
#include <constantshader.h>
#include <inputcomponent.h>

RTTI_BEGIN_CLASS(nap::GeometryInteractionComponent)
	RTTI_PROPERTY("Camera",						&nap::GeometryInteractionComponent::mCamera,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderWindow",				&nap::GeometryInteractionComponent::mRenderWindow,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("InteractionGeometries",		&nap::GeometryInteractionComponent::mInteractionGeometries,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GeometryInteractionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static void renderMesh(RenderService& renderService, RenderService::Pipeline& pipeline, RenderableMesh& renderableMesh)
	{
		// Get material to render with and descriptors for material
		MaterialInstance& mat_instance = renderableMesh.getMaterialInstance();
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Bind descriptor set for next draw call
		VkCommandBuffer command_buffer = renderService.getCurrentCommandBuffer();
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertexBuffers = renderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = renderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(command_buffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

		// Get mesh to draw
		MeshInstance& mesh_instance = renderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Draw individual shapes inside mesh
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(command_buffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(command_buffer, index_buffer.getCount(), 1, 0, 0, 0);
		}
	}


	static bool findIntersection(const glm::vec3& rayOrig, const glm::vec3& rayDir, const MeshInstance& mesh, const TransformComponentInstance& worldTransform, glm::vec3& outIntersection, glm::vec3& outUV)
	{
		// Create the triangle iterator
		TriangleIterator it(mesh);

		// Used by intersection call
		TriangleData<glm::vec3> tri_verts;

		// Get atttributes
		const VertexAttribute<glm::vec3>& verts = mesh.getAttribute<glm::vec3>(vertexid::position);
		const VertexAttribute<glm::vec3>& uvs = mesh.getAttribute<glm::vec3>(vertexid::getUVName(0));

		while (!it.isDone())
		{
			// Use the indices to get the vertex positions
			Triangle tri = it.next();

			tri_verts[0] = (math::objectToWorld(verts[tri[0]], worldTransform.getGlobalTransform()));
			tri_verts[1] = (math::objectToWorld(verts[tri[1]], worldTransform.getGlobalTransform()));
			tri_verts[2] = (math::objectToWorld(verts[tri[2]], worldTransform.getGlobalTransform()));

			glm::vec3 bary_coord;
			if (utility::intersect(rayOrig, rayDir, tri_verts, bary_coord))
			{
				TriangleData<glm::vec3> tri_vert = tri.getVertexData(verts);
				outIntersection = utility::interpolateVertexAttr<glm::vec3>(tri_vert, bary_coord);

				TriangleData<glm::vec3> tri_uv = tri.getVertexData(uvs);
				outUV = utility::interpolateVertexAttr<glm::vec3>(tri_uv, bary_coord);
				return true;
			}
		}
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// Constants
	//////////////////////////////////////////////////////////////////////////

	namespace uniform
	{
		constexpr const char* UBO = "UBO";
		constexpr const char* FRAGUBO = "FRAGUBO";
		constexpr const char* color = "color";
	}


	//////////////////////////////////////////////////////////////////////////
	// GeometryInteractionComponent
	//////////////////////////////////////////////////////////////////////////

	void GeometryInteractionComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(PointerInputComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// GeometryInteractionComponentInstance
	//////////////////////////////////////////////////////////////////////////

	GeometryInteractionComponentInstance::GeometryInteractionComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mSphereMesh(std::make_unique<SphereMesh>(*entity.getCore())) { }


	bool GeometryInteractionComponent::init(utility::ErrorState& errorState)
	{
		// Enforce nap::ConstantShader
		if (!errorState.check(mMaterialInstanceResource.mMaterial->getShader().get_type() == RTTI_OF(ConstantShader), "%s: Property 'Shader' must be of type 'nap::ConstantShader'", mID.c_str()))
			return false;

		return true;
	}


	bool GeometryInteractionComponentInstance::init(utility::ErrorState& errorState)
	{
		// Init base class
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Fetch resource
		GeometryInteractionComponent* resource = getComponent<GeometryInteractionComponent>();

		// Ensure mesh is null
		if (!errorState.check(resource->mMesh == nullptr, "%s: Mesh must be NULL", mID.c_str()))
			return false;

		// Fetch render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();

		// Create sphere mesh
		mSphereMesh->mCullMode = ECullMode::Back;
		mSphereMesh->mPolygonMode = EPolygonMode::Line;
		mSphereMesh->mUsage = EMemoryUsage::Static;
		mSphereMesh->mRings = 16;
		mSphereMesh->mSectors = 16;
		mSphereMesh->mRadius = 0.5f * 0.25f;

		// Init sphere
		if (!mSphereMesh->init(errorState))
			return false;

		// Initialize material based on resource
		if (!mMaterialInstance.init(*mRenderService, resource->mMaterialInstanceResource, errorState))
			return false;

		// Bind the coord mesh to the material
		auto renderable_mesh = mRenderService->createRenderableMesh(*mSphereMesh, mMaterialInstance, errorState);
		if (!renderable_mesh.isValid())
			return false;

		// Set the renderable mesh
		setMesh(renderable_mesh);

		// Ensure there is a transform component
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Copy cliprect. Any modifications are done per instance
		mClipRect = resource->mClipRect;

		// Copy line width, ensure it's supported
		mLineWidth = resource->mLineWidth;
		if (mLineWidth > 1.0f && !mRenderService->getWideLinesSupported())
		{
			nap::Logger::warn("Unsupported line width: %.02f", mLineWidth);
			mLineWidth = 1.0f;
		}

		// Since the material can't be changed at run-time, cache the matrices to set on draw
		// If the struct is found, we expect the matrices with those names to be there
		UniformStructInstance* mvp = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (mvp != nullptr)
		{
			mModelMatUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);
			mViewMatUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
			mProjectMatUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
		}

		// Update uniforms
		UniformStructInstance* ubo = getMaterialInstance().getOrCreateUniform(uniform::UBO);
		if (ubo != nullptr)
		{
			ubo->getOrCreateUniform<UniformVec3Instance>(uniform::color)->setValue(resource->mColor.toVec3());
			ubo->getOrCreateUniform<UniformFloatInstance>("alpha")->setValue(1.0f);
		}

		// Ensure there is a pointer input component
		auto* pointer_comp = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_comp != nullptr, "%s: missing pointer input component", mID.c_str()))
			return false;

		// Ensure there are interaction geoms
		if (!errorState.check(!resource->mInteractionGeometries.empty(), "%s: missing interaction geometries", mID.c_str()))
			return false;

		pointer_comp->pressed.connect(std::bind(&GeometryInteractionComponentInstance::onMouseDown, this, std::placeholders::_1));
		pointer_comp->moved.connect(std::bind(&GeometryInteractionComponentInstance::onMouseMove, this, std::placeholders::_1));
		pointer_comp->released.connect(std::bind(&GeometryInteractionComponentInstance::onMouseUp, this, std::placeholders::_1));

		return true;
	}


	void GeometryInteractionComponentInstance::onMouseMove(const PointerMoveEvent& moveEvent)
	{
		// Update mouse position
		mMousePosition = { moveEvent.mX, moveEvent.mY };

		// Fetch resource
		GeometryInteractionComponent* resource = getComponent<GeometryInteractionComponent>();

		// Get ray from screen in to scene (world space)
		// The result is a normal pointing away from the camera in to the scene
		// The window is used to provide the viewport
		glm::vec3 screen_to_world_ray = mCamera->rayFromScreen(mMousePosition, resource->mRenderWindow->getRect());

		// World space camera position
		glm::vec3 cam_pos = math::extractPosition(mCamera->getEntityInstance()->getComponent<TransformComponentInstance>().getGlobalTransform());

		// Find one intersection
		glm::vec3 intersection, uv;
		bool intersects = false;
		for (const auto& geom : mGeometries)
		{
			// Get object to world transformation matrix
			TransformComponentInstance& world_xform = geom->getEntityInstance()->getComponent<TransformComponentInstance>();

			// Get mesh instance
			auto& mesh_comp = geom->getEntityInstance()->getComponent<RenderableMeshComponentInstance>();
			const auto& mesh = mesh_comp.getMeshInstance();

			// Perform intersection test, walk over every triangle in the mesh		
			intersects = findIntersection(cam_pos, screen_to_world_ray, mesh, world_xform, intersection, uv);
			if (intersects)
			{
				mIntersects = intersects;
				mIntersectionWorldPosition = intersection;
				mIntersectionUV = uv;
				mIntersectionGeometry = static_cast<RenderableMeshComponentInstance*>(geom->get_ptr());
				break;
			}
		}
	}


	void GeometryInteractionComponentInstance::onMouseDown(const PointerPressEvent& pressEvent)
	{
		mMousePressed = true;
	}


	void GeometryInteractionComponentInstance::onMouseUp(const PointerReleaseEvent& pressEvent)
	{
		mMousePressed = false;
	}


	bool GeometryInteractionComponentInstance::getIntersectionData(RenderableMeshComponentInstance* outComp, glm::vec3& outPosition, glm::vec3& outUV) const
	{
		if (mIntersects)
		{
			outComp = mIntersectionGeometry;
			outPosition = mIntersectionWorldPosition;
			outUV = mIntersectionUV;
			return true;
		}
		return false;
	}


	void GeometryInteractionComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		if (!mIntersects)
			return;

		mProjectMatUniform->setValue(projectionMatrix);
		mViewMatUniform->setValue(viewMatrix);
		mModelMatUniform->setValue(glm::translate(mTransformComponent->getGlobalTransform(), mIntersectionWorldPosition));

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		renderMesh(*mRenderService, pipeline, mRenderableMesh);
	}
}
