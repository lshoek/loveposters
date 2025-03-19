/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderblurcomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "gpubuffer.h"
#include "renderglobals.h"
#include "blurshader.h"
#include "textureutils.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <orthocameracomponent.h>

// nap::RenderBlurComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderBlurComponent, "Applies a blur effect to a texture")
	RTTI_PROPERTY("Texture",	&nap::RenderBlurComponent::mTexture,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Samples",	&nap::RenderBlurComponent::mSamples,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::RenderBlurComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderBlurComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION("draw", &nap::RenderBlurComponentInstance::draw)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


/**
 * Creates a model matrix based on the dimensions of the given target.
 */
static void computeModelMatrix(const nap::IRenderTarget& target, const glm::vec2 contentSize, bool preserveAspect, glm::mat4& outMatrix)
{
	// Transform to middle of target
	glm::vec2 target_size = target.getBufferSize();
	outMatrix = glm::translate(glm::mat4(), { target_size.x * 0.5f, target_size.y * 0.5f, 0.0f });

	if (!preserveAspect)
	{
		// Scale to fit target
		outMatrix = glm::scale(outMatrix, { target_size.x, target_size.y, 1.0f });
	}
	else
	{
		// Scale to preserve aspect
		float content_ratio	= contentSize.x / contentSize.y;
		float target_ratio	= target_size.x / target_size.y;

		if (target_ratio < content_ratio)
			target_size.y = target_size.x / content_ratio;
		else
			target_size.x = target_size.y * content_ratio;

		outMatrix = glm::scale(outMatrix, { target_size.x, target_size.y, 1.0f });
	}
}


namespace nap
{
	RenderBlurComponentInstance::RenderBlurComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mMesh(*entity.getCore())
	{
		mSideTexture = std::make_unique<RenderTexture2D>(*entity.getCore());
		mRenderTarget = { std::make_unique<RenderTarget>(*entity.getCore()), std::make_unique<RenderTarget>(*entity.getCore()) };
	}


	bool RenderBlurComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		auto* resource = getComponent<RenderBlurComponent>();
		mTexture = resource->mTexture.get(); assert(mTexture != nullptr);

		// Create side texture
		mSideTexture->mWidth = resource->mTexture->getWidth();
		mSideTexture->mHeight = resource->mTexture->getHeight();
		mSideTexture->mColorFormat = resource->mTexture->mColorFormat;
		mSideTexture->mUsage = resource->mTexture->mUsage;
		if (!mSideTexture->init(errorState))
		{
			errorState.fail("%s: Failed to initialize internal render texture", mID.c_str());
			return false;
		}

		// RenderTexture
		std::vector<RenderTexture2D*> textures = { mSideTexture.get(), mTexture };
		for (uint i = 0; i < mRenderTarget.size(); i++)
		{
			// RenderTarget
			auto& target = mRenderTarget[i];
			auto& tex = textures[i];
			target->mColorTexture = tex;
			target->mClearColor = tex->mClearColor;
			target->mSampleShading = false;
			target->mRequestedSamples = ERasterizationSamples::One;
			if (!target->init(errorState))
			{
				errorState.fail("%s: Failed to initialize internal render target", target->mID.c_str());
				return false;
			}
		}

		// Create material
		Material* mtl = nullptr;
		switch(resource->mSamples)
		{
			case EBlurSamples::X5:
				mtl = mRenderService->getOrCreateMaterial<BlurShader<EBlurSamples::X5>>(errorState);
				break;
			case EBlurSamples::X9:
				mtl = mRenderService->getOrCreateMaterial<BlurShader<EBlurSamples::X9>>(errorState);
				break;
			case EBlurSamples::X13:
				mtl = mRenderService->getOrCreateMaterial<BlurShader<EBlurSamples::X13>>(errorState);
				break;
			default: assert(false);
		}
		if (!errorState.check(mtl != nullptr, "%s: unable to get or create blur material", resource->mID.c_str()))
			return false;

		// Create material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = mtl;
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

        // Initialize unwrap mesh
        mMesh.mPolygonMode = EPolygonMode::Fill;
        mMesh.mUsage = EMemoryUsage::Static;
		mMesh.mCullMode = ECullMode::Back;
		mMesh.mSize = { 1.0f, 1.0f };
		mMesh.mPosition = { 0.0f, 0.0f };
		mMesh.mColumns = 1;
		mMesh.mRows = 1;

		if (!mMesh.init(errorState))
			return false;

		// Get MVP struct
		auto* mvp = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mvp != nullptr, "%s: Unable to find uniform mvp struct: %s in material: %s", mID.c_str(), uniform::mvpStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		mModelMatrixUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);
		if (mModelMatrixUniform == nullptr)
			return false;

		mViewMatrixUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
		if (mViewMatrixUniform == nullptr)
			return false;

		mProjectMatrixUniform = mvp->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
		if (mProjectMatrixUniform == nullptr)
			return false;

        // Get color texture sampler
        mColorTextureSampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::blur::sampler::colorTexture);
        if (!errorState.check(mColorTextureSampler != nullptr, "Missing uniform sampler2D 'colorTexture'"))
            return false;

       mColorTextureSampler->setTexture(*resource->mTexture);

		// Get UBO struct
		auto* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::blur::uboStruct);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find uniform UBO struct: %s in material: %s", mID.c_str(), uniform::blur::uboStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		mTextureSizeUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>("textureSize");
		mDirectionUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>("direction");

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mRenderService->createRenderableMesh(mMesh, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		return true;
	}


	void RenderBlurComponentInstance::draw()
	{
        mTextureSizeUniform->setValue(mTexture->getSize());

        // Horizontal
		mColorTextureSampler->setTexture(*mTexture);
        mDirectionUniform->setValue({ 1.0f, 0.0f });

		auto& horizontal = mRenderTarget.front();
		const auto proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(
			0.0f, (float)horizontal->getBufferSize().x, 0.0f, (float)horizontal->getBufferSize().y);

		horizontal->beginRendering();
		onDraw(*horizontal, mRenderService->getCurrentCommandBuffer(), glm::identity<glm::mat4>(), proj_matrix);
		horizontal->endRendering();

		// Vertical
		mColorTextureSampler->setTexture(horizontal->getColorTexture());
		mDirectionUniform->setValue({ 0.0f, 1.0f });

		auto& vertical = mRenderTarget.back();
		vertical->beginRendering();
		onDraw(*vertical, mRenderService->getCurrentCommandBuffer(), glm::identity<glm::mat4>(), proj_matrix);
		vertical->endRendering();
	}


	void RenderBlurComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Update the model matrix so that the plane mesh is of the same size as the render target
		if (mModelMatrixUniform != nullptr)
		{
			glm::mat4 model_matrix;
			computeModelMatrix(renderTarget, mTexture->getSize(), false, model_matrix);
			mModelMatrixUniform->setValue(model_matrix);
		}

		if (mViewMatrixUniform != nullptr)
			mViewMatrixUniform->setValue(viewMatrix);

		if (mProjectMatrixUniform != nullptr)
			mProjectMatrixUniform->setValue(projectionMatrix);

		// Get valid descriptor set
		const auto& descriptor_set = mMaterialInstance.update();

		// Gather draw info
		const auto& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		const auto& mesh = mesh_instance.getGPUMesh();

		// Get pipeline to to render with
		utility::ErrorState error_state;
        auto pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind buffers and draw
		const auto& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const auto& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

        const auto& index_buffer = mesh.getIndexBuffer(0);
        vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
	}
}
