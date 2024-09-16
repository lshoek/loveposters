/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "RenderMultiVideoComponent.h"
#include "videoshader.h"

// External Includes
#include <entity.h>
#include <orthocameracomponent.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <glm/gtc/matrix_transform.hpp>

// nap::rendervideototexturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderMultiVideoComponent, "Renders the output of multiple video players directly to texture without having to define a render target, shader or mesh")
	RTTI_PROPERTY("OutputTexture",		&nap::RenderMultiVideoComponent::mOutputTexture,			nap::rtti::EPropertyMetaData::Required,	"The texture to render output to")
	RTTI_PROPERTY("VideoPlayers",		&nap::RenderMultiVideoComponent::mVideoPlayers,				nap::rtti::EPropertyMetaData::Required, "The video player to render to texture")
	RTTI_PROPERTY("Samples",			&nap::RenderMultiVideoComponent::mRequestedSamples,			nap::rtti::EPropertyMetaData::Default,	"The number of rasterization samples")
	RTTI_PROPERTY("ClearColor",			&nap::RenderMultiVideoComponent::mClearColor,				nap::rtti::EPropertyMetaData::Default,	"Initial target clear color")
	RTTI_PROPERTY("Offset",				&nap::RenderMultiVideoComponent::mOffset,					nap::rtti::EPropertyMetaData::Default,  "Video offset")
	RTTI_PROPERTY("Scale",				&nap::RenderMultiVideoComponent::mScale,					nap::rtti::EPropertyMetaData::Default,	"Video scale")
	RTTI_PROPERTY("Blend",				&nap::RenderMultiVideoComponent::mBlendValue,				nap::rtti::EPropertyMetaData::Required,	"Video blend value")
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderMultiVideoComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Default,	"Material instance resource")
RTTI_END_CLASS

// nap::rendervideototexturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderMultiVideoComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	/**
	 * Creates a model matrix based on the dimensions of the given target.
	 */
	static void computeModelMatrix(const nap::IRenderTarget& target, glm::mat4& outMatrix)
	{
		// Transform to middle of target
		glm::ivec2 tex_size = target.getBufferSize();
		outMatrix = glm::translate(glm::mat4(), glm::vec3(
			tex_size.x / 2.0f,
			tex_size.y / 2.0f,
			0.0f));

		// Scale to fit target
		outMatrix = glm::scale(outMatrix, glm::vec3(tex_size.x, tex_size.y, 1.0f));
	}


	RenderMultiVideoComponentInstance::RenderMultiVideoComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mTarget(*entity.getCore()),
		mPlane(*entity.getCore())	{ }


	bool RenderMultiVideoComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		RenderMultiVideoComponent* resource = getComponent<RenderMultiVideoComponent>();

		// Extract player
		for (uint i = 0; i < resource->mVideoPlayers.size(); i++)
		{
			auto& player = resource->mVideoPlayers[i];
			if (!errorState.check(player != nullptr, "VideoPlayer NULL found"))
				return false;

			mPlayers[i] = player.get();
			mVideoMap.emplace(player.get(), i);
		}

		// Extract output texture to render to and make sure format is correct
		mOutputTexture = resource->mOutputTexture.get();
		if (!errorState.check(mOutputTexture != nullptr, "%s: no output texture", resource->mID.c_str()))
			return false;

		if (!errorState.check(mOutputTexture->mColorFormat == RenderTexture2D::EFormat::RGBA8, "%s: output texture color format is not RGBA8", resource->mID.c_str()))
			return false;

		// Setup render target and initialize
		mTarget.mClearColor = resource->mClearColor.convert<RGBAColorFloat>();
		mTarget.mColorTexture  = resource->mOutputTexture;
		mTarget.mSampleShading = true;
		mTarget.mRequestedSamples = resource->mRequestedSamples;
		if (!mTarget.init(errorState))
			return false;

		// Now create a plane and initialize it
		// The plane is positioned on update based on current texture output size
		mPlane.mSize = glm::vec2(1.0f, 1.0f);
		mPlane.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		mPlane.mCullMode = ECullMode::Back;
		mPlane.mUsage = EMemoryUsage::Static;
		mPlane.mColumns = 1;
		mPlane.mRows = 1;

		if (!mPlane.init(errorState))
			return false;

		// Extract render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mRenderService != nullptr);

		// Initialize video material instance, used for rendering video
		if (!mMaterialInstance.init(*mRenderService, resource->mMaterialInstanceResource, errorState))
			return false;

		// Ensure the mvp struct is available
		mMVPStruct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform MVP struct: %s in material: %s",
			this->mID.c_str(), uniform::mvpStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get all matrices
		mModelMatrixUniform = ensureUniform(uniform::modelMatrix, errorState);
		mProjectMatrixUniform = ensureUniform(uniform::projectionMatrix, errorState);
		mViewMatrixUniform = ensureUniform(uniform::viewMatrix, errorState);

		if (mModelMatrixUniform == nullptr || mProjectMatrixUniform == nullptr || mViewMatrixUniform == nullptr)
			return false;

		// Fetch blend and scale uniform
		auto* ubo_struct = mMaterialInstance.getOrCreateUniform("UBO");
		if (ubo_struct != nullptr)
		{
			mBlendValueUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>("blend");
			if (!errorState.check(mBlendValueUniform != nullptr, "Uniform `blend` missing from UBO"))
				return false;

			auto* offset_uniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>("offset");
			if (!errorState.check(offset_uniform != nullptr, "Uniform `offset` missing from UBO"))
				return false;
			offset_uniform->setValue(resource->mOffset);

			auto* scale_uniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>("scale");
			if (!errorState.check(scale_uniform != nullptr, "Uniform `scale` missing from UBO"))
				return false;
			scale_uniform->setValue(resource->mScale);
		}
		mBlendValueParam = resource->mBlendValue.get();

		// Get sampler inputs to update from video material
		mYSamplerA = ensureSampler("yTextureA", errorState);
		mUSamplerA = ensureSampler("uTextureA", errorState);
		mVSamplerA = ensureSampler("vTextureA", errorState);

		// Get sampler inputs to update from video material
		mYSamplerB = ensureSampler("yTextureB", errorState);
		mUSamplerB = ensureSampler("uTextureB", errorState);
		mVSamplerB = ensureSampler("vTextureB", errorState);

		if (mYSamplerA == nullptr || mUSamplerA == nullptr || mVSamplerA == nullptr ||
			mYSamplerB == nullptr || mUSamplerB == nullptr || mVSamplerB == nullptr)
			return false;

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mRenderService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		// Listen to video selection changes & update textures on init
		for (auto& player : resource->mVideoPlayers)
		{
			player->VideoChanged.connect(mVideoChangedSlot);
			videoChanged(*player);
		}

		return true;
	}


	bool RenderMultiVideoComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	nap::Texture2D& RenderMultiVideoComponentInstance::getOutputTexture()
	{
		return mTarget.getColorTexture();
	}


	void RenderMultiVideoComponentInstance::draw()
	{
		// Get current command buffer, should be headless.
		VkCommandBuffer command_buffer = mRenderService->getCurrentCommandBuffer();

		// Create orthographic projection matrix
		glm::ivec2 size = mTarget.getBufferSize();

		// Create projection matrix
		glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)size.x, 0.0f, (float)size.y);

		// Call on draw
		mTarget.beginRendering();
		onDraw(mTarget, command_buffer, glm::mat4(), proj_matrix);
		mTarget.endRendering();
	}


	void RenderMultiVideoComponentInstance::update(double deltaTime)
	{
		mBlendValueUniform->setValue(mBlendValueParam->mValue);
	}


	void RenderMultiVideoComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Update the model matrix so that the plane mesh is of the same size as the render target
		computeModelMatrix(renderTarget, mModelMatrix);
		mModelMatrixUniform->setValue(mModelMatrix);

		// Update matrices, projection and model are required
		mProjectMatrixUniform->setValue(projectionMatrix);
		mViewMatrixUniform->setValue(viewMatrix);

		// Get valid descriptor set
		const DescriptorSet& descriptor_set = mMaterialInstance.update();

		// Gather draw info
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind buffers and draw
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();

		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}
	}


	nap::UniformMat4Instance* RenderMultiVideoComponentInstance::ensureUniform(const std::string& uniformName, utility::ErrorState& error)
	{
		assert(mMVPStruct != nullptr);
		UniformMat4Instance* found_uniform = mMVPStruct->getOrCreateUniform<UniformMat4Instance>(uniformName);
		if (!error.check(found_uniform != nullptr,
			"%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(),
			mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;
		return found_uniform;
	}


	nap::Sampler2DInstance* RenderMultiVideoComponentInstance::ensureSampler(const std::string& samplerName, utility::ErrorState& error)
	{
		Sampler2DInstance* found_sampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(samplerName);
		if (!error.check(found_sampler != nullptr,
			"%s: unable to find sampler: %s in material: %s", this->mID.c_str(), samplerName.c_str(),
			mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;
		return found_sampler;
	}

	
	void RenderMultiVideoComponentInstance::videoChanged(VideoPlayer& player)
	{
		auto it = mVideoMap.find(&player);
		assert(it != mVideoMap.end());

		if (it->second == 0)
		{
			mYSamplerA->setTexture(player.getYTexture());
			mUSamplerA->setTexture(player.getUTexture());
			mVSamplerA->setTexture(player.getVTexture());
		}
		else
		{
			mYSamplerB->setTexture(player.getYTexture());
			mUSamplerB->setTexture(player.getUTexture());
			mVSamplerB->setTexture(player.getVTexture());
		}
	}
}
