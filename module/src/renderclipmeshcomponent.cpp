/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "RenderClipMeshComponent.h"
#include "mesh.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"
#include "gpubuffer.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <transformcomponent.h>

RTTI_BEGIN_CLASS(nap::RenderClipMeshComponent)
	RTTI_PROPERTY("ShadowMaterialInstance", &nap::RenderClipMeshComponent::mShadowMaterialInstanceResource, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderClipMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	RenderClipMeshComponentInstance::RenderClipMeshComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource)	{ }


	bool RenderClipMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		RenderClipMeshComponent* resource = getComponent<RenderClipMeshComponent>();
		if (!mShadowMaterialInstance.init(*mRenderService, resource->mShadowMaterialInstanceResource, errorState))
			return false;

		UniformStructInstance* mvp_struct = mShadowMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (mvp_struct != nullptr)
		{
			mShadowModelMatUniform = mvp_struct->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);
			mShadowViewMatUniform = mvp_struct->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
			mShadowProjectMatUniform = mvp_struct->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
		}

		return true;
	}


	void RenderClipMeshComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		bool use_shadow_material = (renderTarget.getDepthFormat() != VK_FORMAT_UNDEFINED) &&
			(renderTarget.getColorFormat() == VK_FORMAT_UNDEFINED);

		if (use_shadow_material)
		{
			// Set mvp matrices if present in material
			if (mShadowProjectMatUniform != nullptr)
				mShadowProjectMatUniform->setValue(projectionMatrix);

			if (mShadowViewMatUniform != nullptr)
				mShadowViewMatUniform->setValue(viewMatrix);

			if (mShadowModelMatUniform != nullptr)
				mShadowModelMatUniform->setValue(mTransformComponent->getGlobalTransform());
		}
		else
		{
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
		}

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = use_shadow_material ? mShadowMaterialInstance : getMaterialInstance();
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

		// TODO: move to push/pop cliprect on RenderTarget once it has been ported
		bool has_clip_rect = mClipRect.hasWidth() && mClipRect.hasHeight();
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = mClipRect.getMin().x;
			rect.offset.y = mClipRect.getMin().y;
			rect.extent.width = mClipRect.getWidth();
			rect.extent.height = mClipRect.getHeight();
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mLineWidth);

		// Draw meshes
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);

		// Restore clipping
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = 0;
			rect.offset.y = 0;
			rect.extent.width = renderTarget.getBufferSize().x;
			rect.extent.height = renderTarget.getBufferSize().y;
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}
	}
} 
