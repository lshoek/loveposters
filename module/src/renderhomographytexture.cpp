/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderhomographytexture.h"
#include "homographycomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "gpubuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"
#include "textureutils.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>
#include <textureshader.h>

// nap::RenderHomographyTexture run time class definition 
RTTI_BEGIN_CLASS(nap::RenderHomographyTexture)
	RTTI_PROPERTY("MaterialInstance", &nap::RenderHomographyTexture::mMaterialInstanceResource, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::RenderHomographyTextureInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderHomographyTextureInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderHomographyTexture
	//////////////////////////////////////////////////////////////////////////

	void RenderHomographyTexture::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(HomographyComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderHomographyTextureInstance
	//////////////////////////////////////////////////////////////////////////

	RenderHomographyTextureInstance::RenderHomographyTextureInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mEmptyMesh(std::make_unique<EmptyMesh>(*entity.getCore()))
	{ }


	bool RenderHomographyTextureInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		mResource = getComponent<RenderHomographyTexture>();

		// Get render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();

		// Get homography component
		mHomographyComponent = getEntityInstance()->findComponent<HomographyComponentInstance>();
		if (!errorState.check(mHomographyComponent != nullptr, "%s: missing homography component", mID.c_str()))
			return false;

		// Initialize material based on resource
		if (!mMaterialInstance.init(*mRenderService, mResource->mMaterialInstanceResource, errorState))
			return false;

		// Get UBO struct
		UniformStructInstance* ubo_struct = mMaterialInstance.getOrCreateUniform("UBO");
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find uniform UBO struct: %s in material: %s", this->mID.c_str(), "UBO", mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get frame buffer size uniform
		mHomographyMatrixUniform = ubo_struct->getOrCreateUniform<UniformMat4Instance>("homographyMatrix");
		if (!errorState.check(mHomographyMatrixUniform != nullptr, "Missing uniform mat4 'homographyMatrix' in uniform UBO"))
			return false;

		// Create empty mesh
		if (!mEmptyMesh->init(errorState))
			return false;

		return true;
	}


	void RenderHomographyTextureInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		if (!isVisible())
			return;

		mHomographyMatrixUniform->setValue(mHomographyComponent->getInverseHomographyMatrix());

		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, *mEmptyMesh, mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		const DescriptorSet& descriptor_set = mMaterialInstance.update();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}
}
