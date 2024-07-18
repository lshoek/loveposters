/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "renderhomographypoints.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <meshutils.h>
#include <renderglobals.h>
#include <renderservice.h>
#include <constantshader.h>
#include <transformcomponent.h>

// Local includes
#include "homographycomponent.h"

RTTI_BEGIN_CLASS(nap::RenderHomographyPoints)
	RTTI_PROPERTY("Color", &nap::RenderHomographyPoints::mColor, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderHomographyPointsInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderHomographyPoints
	//////////////////////////////////////////////////////////////////////////

	class PointMesh : public IMesh
	{
	public:
		PointMesh(Core& core) :
			mRenderService(core.getService<RenderService>())
		{ }

		bool init(utility::ErrorState& errorState)
		{
			assert(mRenderService != nullptr);
			mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

			// Because the mesh is populated dynamically we set the initial amount of vertices to be 0
			mMeshInstance->setNumVertices(4);
			mMeshInstance->setUsage(EMemoryUsage::DynamicWrite);
			mMeshInstance->setDrawMode(EDrawMode::Points);
			mMeshInstance->setPolygonMode(EPolygonMode::Point);
			mMeshInstance->setCullMode(ECullMode::None);

			// Create the necessary attributes
			Vec3VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);

			std::vector<glm::vec3> data(4, { 0.0f, 0.0f, 0.0f});
			position_attribute.setData(data);

			// Reserve vertices for attribute data
			mMeshInstance->reserveVertices(4);

			// Initialize our instance
			return mMeshInstance->init(errorState);
		}

		/**
		* @return MeshInstance as created during init().
		*/
		virtual MeshInstance& getMeshInstance()	override { return *mMeshInstance; }

		/**
		* @return MeshInstance as created during init().
		*/
		virtual const MeshInstance& getMeshInstance() const	override { return *mMeshInstance; }

	private:
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;			///< The mesh instance to construct
		nap::RenderService* mRenderService = nullptr;					///< Handle to the render service
	};


	//////////////////////////////////////////////////////////////////////////
	// RenderHomographyPoints
	//////////////////////////////////////////////////////////////////////////

	void RenderHomographyPoints::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(HomographyComponent));
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderHomographyPointsInstance
	//////////////////////////////////////////////////////////////////////////

	RenderHomographyPointsInstance::RenderHomographyPointsInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mPointMesh(std::make_unique<PointMesh>(*entity.getCore())),
		mRenderService(entity.getCore()->getService<RenderService>())
	{ }
	

    bool RenderHomographyPointsInstance::init(utility::ErrorState& errorState)
    {
		// Initialize base
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Ensure there is a transform component
		mHomographyComponent = getEntityInstance()->findComponent<HomographyComponentInstance>();
		if (!errorState.check(mHomographyComponent != nullptr, "%s: missing homography component", mID.c_str()))
			return false;

		// Ensure there is a transform component
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Initialize material based on resource
		auto* resource = getComponent<RenderHomographyPoints>();

		if (!mPointMesh->init(errorState))
			return false;

		mPointsAttribute = &mPointMesh->getMeshInstance().getOrCreateAttribute<glm::vec3>(vertexid::position);
		assert(mPointsAttribute->getCount() >= 4);

		if (!updateMeshInstance(errorState))
			return false;

		// Create fade material
		Material* material = mRenderService->getOrCreateMaterial<ConstantShader>(errorState);
		if (!errorState.check(material != nullptr, "%s: unable to get or create constant material", mID.c_str()))
			return false;

		// Create resource for the fade material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = material;

		// Create constant material instance
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		mRenderableMesh = mRenderService->createRenderableMesh(*mPointMesh, mMaterialInstance, errorState);
		if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
			return false;

		// Get uniforms
		auto uniform_struct = mMaterialInstance.getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(uniform_struct != nullptr, "%s: Unable to find uniform struct: %s in shader: %s",
			mID.c_str(), uniform::constant::uboStruct, RTTI_OF(ConstantShader).get_name().data()))
			return false;

		// Store alpha (updated at runtime)
		auto* alpha = uniform_struct->getOrCreateUniform<UniformFloatInstance>(uniform::constant::alpha);
		if (!errorState.check(alpha != nullptr, "%s: Unable to find uniform: %s in shader: %s",
			mID.c_str(), uniform::constant::alpha, RTTI_OF(ConstantShader).get_name().data()))
			return false;

		alpha->setValue(1.0f);

		// Set color
		auto* color = uniform_struct->getOrCreateUniform<UniformVec3Instance>(uniform::constant::color);
		if (!errorState.check(color != nullptr, "%s: Unable to find uniform: %s in shader: %s",
			mID.c_str(), uniform::constant::color, RTTI_OF(ConstantShader).get_name().data()))
			return false;

		color->setValue(resource->mColor);

        return true;
    }


    void RenderHomographyPointsInstance::update(double deltaTime)
    {
		utility::ErrorState error_state;
		if (!updateMeshInstance(error_state))
			nap::Logger::error(error_state.toString());
    }


    bool RenderHomographyPointsInstance::updateMeshInstance(utility::ErrorState& errorState)
    {
		auto top_left = mHomographyComponent->getTopLeft();
		auto top_right = mHomographyComponent->getTopRight();
		auto bottom_right = mHomographyComponent->getBottomRight();
		auto bottom_left = mHomographyComponent->getBottomLeft();

		bool points_changed = top_left != mTopLeft || top_right != mTopRight || bottom_right != mBottomRight || bottom_left != mBottomLeft;

		if (points_changed)
		{
			mTopLeft = top_left;
			mTopRight = top_right;
			mBottomRight = bottom_right;
			mBottomLeft = bottom_left;

			mPointsAttribute->getData()[0] = { mTopLeft.x,		mTopLeft.y, 0.0f };
			mPointsAttribute->getData()[1] = { mTopRight.x,		mTopRight.y, 0.0f };
			mPointsAttribute->getData()[2] = { mBottomRight.x,	mBottomRight.y, 0.0f };
			mPointsAttribute->getData()[3] = { mBottomLeft.x,	mBottomLeft.y, 0.0f };

			if (!mPointMesh->getMeshInstance().update(errorState))
				return false;
		}
		return true;
    }


	void RenderHomographyPointsInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Set mvp matrices if present in material
		if (mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		if (mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if (mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		if (mNormalMatrixUniform != nullptr)
			mNormalMatrixUniform->setValue(glm::transpose(glm::inverse(mTransformComponent->getGlobalTransform())));

		if (mCameraWorldPosUniform != nullptr)
			mCameraWorldPosUniform->setValue(math::extractPosition(glm::inverse(viewMatrix)));

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = mMaterialInstance;
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

		// Draw meshes
		MeshInstance& mesh_instance = mPointMesh->getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();
		vkCmdDraw(commandBuffer, mesh_instance.getNumVertices(), 1, 0, 0);
	}
}
