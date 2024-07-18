/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rendercomponent.h>
#include <componentptr.h>
#include <perspcameracomponent.h>
#include <materialinstance.h>
#include <renderablemesh.h>
#include <emptymesh.h>
#include <parameternumeric.h>

namespace nap
{
	// Forward Declares
	class RenderHomographyTextureInstance;
	class HomographyComponentInstance;

	/**
	 * RenderHomographyTexture
	 */
	class NAPAPI RenderHomographyTexture : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderHomographyTexture, RenderHomographyTextureInstance)
	public:
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		MaterialInstanceResource mMaterialInstanceResource; ///< Property: 'MaterialInstance' Instance of the material, used to override uniforms for this instance
	};


	/**
	 * RenderHomographyTextureInstance
	 */
	class NAPAPI RenderHomographyTextureInstance : public  RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderHomographyTextureInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderHomographyTextureInstance based on the RenderHomographyTexture resource.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderHomographyTextureInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Draws the effect full screen to the currently active render target
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix ignored
		 * @param projectionMatrix ignored
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		RenderHomographyTexture*		mResource = nullptr;
		HomographyComponentInstance*	mHomographyComponent = nullptr;
		RenderService*					mRenderService = nullptr;			///< Render service

		MaterialInstance				mMaterialInstance;					///< The MaterialInstance as created from the resource.
		UniformMat4Instance*			mHomographyMatrixUniform = nullptr; ///< Homography matrix uniform
		std::unique_ptr<EmptyMesh>		mEmptyMesh;							///< Empty mesh
	};
}
