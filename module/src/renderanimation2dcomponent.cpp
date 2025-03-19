/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderanimation2dcomponent.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <descriptorsetcache.h>
#include <meshutils.h>
#include <transformcomponent.h>
#include <textureshader.h>

RTTI_BEGIN_CLASS(nap::RenderAnimation2DComponent)
	RTTI_PROPERTY("MaterialInstanceResource",		&nap::RenderAnimation2DComponent::mMaterialInstanceResource,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LineWidth",						&nap::RenderAnimation2DComponent::mLineWidth,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",							&nap::RenderAnimation2DComponent::mColor,							nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Opacity",						&nap::RenderAnimation2DComponent::mOpacity,							nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderAnimation2DComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION(nap::material::instance::getOrCreateMaterial, &nap::RenderAnimation2DComponentInstance::getOrCreateMaterial)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Creates the uniform with the given name, logs an error when not available.
	 * @return the uniform, nullptr if not available.
	 */
	template<typename T>
	static T* getUniform(const std::string& uniformName, UniformStructInstance& uniformStruct, utility::ErrorState& error)
	{
		T* found_uniform = uniformStruct.getOrCreateUniform<T>(uniformName);
		return error.check(found_uniform != nullptr,
			"Unable to get or create uniform with name: %s in struct: %s", uniformName.c_str(), uniformStruct.getDeclaration().mName.c_str()) ?
			found_uniform : nullptr;
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderAnimation2DComponent
	//////////////////////////////////////////////////////////////////////////

	void RenderAnimation2DComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
		components.push_back(RTTI_OF(Animation2DComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderAnimation2DComponentInstance
	//////////////////////////////////////////////////////////////////////////

	RenderAnimation2DComponentInstance::RenderAnimation2DComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(*entity.getCore()->getService<RenderService>()),
		mPlaneMesh(*entity.getCore())
	{ }


	bool RenderAnimation2DComponentInstance::init(utility::ErrorState& errorState)
	{
		// Cache resource
		mResource = getComponent<RenderAnimation2DComponent>();

		// Ensure there is an animation component
		mAnimation = getEntityInstance()->findComponent<Animation2DComponentInstance>();
		if (!errorState.check(mAnimation != nullptr, "%s: missing animation component", mID.c_str()))
			return false;

		// Ensure there is a transform component
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Initialize base class
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Create constant material instance
		if (!mMaterialInstance.init(mRenderService, mResource->mMaterialInstanceResource, errorState))
			return false;

		// Since the material can't be changed at run-time, cache the matrices to set on draw
		// If the struct is found, we expect the matrices with those names to be there
		// Ensure the mvp struct is available
		mMVPStruct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform MVP struct: %s in shader: %s",
			this->mID.c_str(), uniform::mvpStruct, RTTI_OF(TextureShader).get_name().data()))
			return false;

		// Get all matrices
		mModelMatUniform = getUniform<UniformMat4Instance>(uniform::modelMatrix, *mMVPStruct, errorState);
		mProjectMatUniform = getUniform<UniformMat4Instance>(uniform::projectionMatrix, *mMVPStruct, errorState);
		mViewMatUniform = getUniform<UniformMat4Instance>(uniform::viewMatrix, *mMVPStruct, errorState);
		if (mModelMatUniform == nullptr || mProjectMatUniform == nullptr || mViewMatUniform == nullptr)
			return false;

		// Get all constant uniforms
		mUBOStruct = mMaterialInstance.getOrCreateUniform(uniform::texture::uboStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform struct: %s in shader: %s",
			mID.c_str(), uniform::texture::uboStruct, RTTI_OF(TextureShader).get_name().data()))
			return false;

		mColorUniform = getUniform<UniformVec3Instance>(uniform::texture::color, *mUBOStruct, errorState);
		mAlphaUniform = getUniform<UniformFloatInstance>(uniform::texture::alpha, *mUBOStruct, errorState);
		if (mColorUniform == nullptr || mAlphaUniform == nullptr)
			return false;

		// Set color & opacity
		mColorUniform->setValue(mResource->mColor.toVec3());
		mAlphaUniform->setValue(mResource->mOpacity);

		// Sampler
		mSamplerUniform = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::texture::sampler::colorTexture); assert(mSamplerUniform != nullptr);

		// Initialize frustum mesh
		mPlaneMesh.mUsage = EMemoryUsage::Static;
		if (!errorState.check(mPlaneMesh.init(errorState), "Unable to create frustrum mesh"))
			return false;

		// Create mesh / material combo that can be rendered to target
		mRenderableMesh = mRenderService.createRenderableMesh(mPlaneMesh, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		return true;
	}


	void RenderAnimation2DComponentInstance::update(double deltaTime)
	{

	}


	void RenderAnimation2DComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
			mModelMatUniform->setValue(mTransform->getGlobalTransform());

		mSamplerUniform->setTexture(mAnimation->getFrame());

		// Acquire new / unique descriptor set before rendering
		auto& mat_instance = mMaterialInstance;
		const auto& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService.getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const auto& vertex_buffers = mRenderableMesh.getVertexBuffers();
		const auto& offsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());

		// Draw meshes
		auto& mesh_instance = mPlaneMesh.getMeshInstance();
		auto& mesh = mesh_instance.getGPUMesh();

		const auto& index_buffer = mesh.getIndexBuffer(0);
		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
	}
}
