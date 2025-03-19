/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <animation2dcomponent.h>
#include <renderablemeshcomponent.h>
#include <planemesh.h>
#include <nap/resourceptr.h>
#include <componentptr.h>

namespace nap
{
	// Forward declares
	class RenderAnimation2DComponentInstance;
	class BlockTreeComponentInstance;

	/**
	 * RenderAnimation2DComponent
	 */
	class NAPAPI RenderAnimation2DComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderAnimation2DComponent, RenderAnimation2DComponentInstance)

	public:
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const;

		MaterialInstanceResource		mMaterialInstanceResource;			///< Resource used to initialize the material instance
		RGBColorFloat					mColor = { 1.0f, 1.0f, 1.0f };		///< Property: 'Color' box draw color
		float 							mLineWidth = 1.0f;					///< Property: 'LineWidth' frustrum line width
		float 							mOpacity = 1.0f;					///< Property: 'Opacity' box alpha
	};


	/**
	 * RenderAnimation2DComponentInstance
	 */
	class NAPAPI RenderAnimation2DComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		// Default constructor
		RenderAnimation2DComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Update
		 * @param deltaTime
		 */
		void update(double deltaTime) override;

		/**
		 * onDraw() override
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Returns the program used to render the mesh.
		 *
		 * TODO: This should be private, but our current RTTI implementation 'mangles' class name-spaces,
		 * causing the RTTR_REGISTRATION_FRIEND macro to fail -> needs to be fixed.
		 * It is therefore not recommended to use this function at runtime, use 'getMaterialInstance' instead!
		 *
		 * @return material handle
		 */
		MaterialInstance* getOrCreateMaterial() { return &mMaterialInstance; }

	private:
		RenderAnimation2DComponent*		mResource = nullptr;				///< Reference to resource
		Animation2DComponentInstance*	mAnimation = nullptr;				///< Reference to animation

		RenderService&					mRenderService;						///< Reference to render service
		TransformComponentInstance*		mTransform = nullptr;				///< Component transform

		PlaneMesh						mPlaneMesh;
		RenderableMesh					mRenderableMesh;					///< The renderable box mesh
		MaterialInstance				mMaterialInstance;					///< The MaterialInstance as created from the resource.

		UniformStructInstance*			mMVPStruct = nullptr;				///< model view projection struct
		UniformMat4Instance*			mModelMatUniform = nullptr;			///< Pointer to the model matrix uniform
		UniformMat4Instance*			mViewMatUniform = nullptr;			///< Pointer to the view matrix uniform
		UniformMat4Instance*			mProjectMatUniform = nullptr;		///< Pointer to the projection matrix uniform

		UniformStructInstance*			mUBOStruct = nullptr;				///< UBO struct
		UniformVec3Instance*			mColorUniform = nullptr;			///< Constant color uniform
		UniformFloatInstance*			mAlphaUniform = nullptr;			///< Alpha uniform

		Sampler2DInstance*				mSamplerUniform = nullptr;
	};
}
