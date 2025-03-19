/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "blurshader.h"

// External includes
#include <rendercomponent.h>
#include <renderablemesh.h>
#include <planemesh.h>

namespace nap
{
	// Forward Declares
	class RenderBlurComponentInstance;

	/**
	 * Pre- or post-processing effect that blurs the input texture at a downsampled resolution into internally managed
	 * rendertargets.
	 *
	 * Resource-part of RenderBlurComponentInstance.
	 */
	class NAPAPI RenderBlurComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderBlurComponent, RenderBlurComponentInstance)
	public:
		ResourcePtr<RenderTexture2D> mTexture; 		///< Property: "Texture"
		EBlurSamples mSamples = EBlurSamples::X5;	///< Property: "Samples"
	};


	/**
	 * Pre- or post-processing effect that blurs the input texture at a downsampled resolution into internally managed
	 * rendertargets.
	 *
	 * This component manages its own render target and plane to render to.
	 * The plane is automatically scaled to fit the bounds of the render target.
	 *
	 * `InputTexture` is blitted to an internally managed render target, and then blurred based on the specified pass
	 * count. Each blur 'pass' then comprises two passes; horizontal and vertical. The gaussian sampling kernel size can
	 * be specified with the 'Kernel' property. Each subsequent pass performs a horizontal and vertical blur at half the
	 * resolution of the former pass, with the first being at half the resolution of `InputTexture`.
	 * 
	 * When the bloom passes have been completed, the result is blitted to `OutputTexture`. `InputTexture` and
	 * `OutputTexture` are allowed to refer to the same `nap::RenderTexture`.
	 *
	 * Simply declare the component in json and call RenderBlurComponentInstance::draw() in the render part of your
	 * application, in between nap::RenderService::beginHeadlessRecording() and nap::RenderService::endHeadlessRecording().
	 *
	 * This component uses the default naprender blur shader and automatically sets its shader variables based on this
	 * component's properties configuration. 
	 */
	class NAPAPI RenderBlurComponentInstance : public  RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderBlurComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderBlurComponentInstance based on the RenderBlurComponent resource.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderBlurComponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Renders the effect to the output texture, without having to define a render target or mesh.
		 * Call this in your application render() call inbetween nap::RenderService::beginHeadlessRecording()
		 * and nap::RenderService::endHeadlessRecording().
		 * Do not call this function outside of a headless recording pass i.e. when rendering to a window.
		 * The result is rendered into a dynamically created output texture, which is accessible through
		 * RenderBlurComponentInstance::getOutputTexture().
		 * Alternatively, you can use the render service to render this component, see onDraw().
		 */
		void draw();

		/**
		 * Returns the output texture with the bloom effect applied.
		 * The size of this texture equals { input_width/2^PassCount, input_height/2^PassCount }
		 * @return the bloom texture created from the specified input texture
		 */
		Texture2D& getOutputTexture() { return *mTexture; }

	protected:
		/**
		 * Draws the effect full screen to the currently active render target,
		 * when the view matrix = identity.
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix often the camera world space location
		 * @param projectionMatrix often the camera projection matrix
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		using DoubleBufferedRenderTarget = std::array<std::unique_ptr<RenderTarget>, 2>;


		DoubleBufferedRenderTarget 	mRenderTarget;						///< Internally managed render target
		std::unique_ptr<RenderTexture2D> mSideTexture;

		MaterialInstanceResource	mMaterialInstanceResource;			///< Instance of the material, used to override uniforms for this instance
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.

		RenderableMesh				mRenderableMesh;					///< Valid Plane / Material combination
		PlaneMesh					mMesh;								///< Empty mesh

		UniformMat4Instance*		mModelMatrixUniform = nullptr;		///< Name of the model matrix uniform in the shader
		UniformMat4Instance*		mViewMatrixUniform = nullptr;		///< View matrix uniform
		UniformMat4Instance*		mProjectMatrixUniform = nullptr;	///< Name of the projection matrix uniform in the shader

        Sampler2DInstance*			mColorTextureSampler = nullptr;		///< Sampler instance for color textures in the blur material
        UniformVec2Instance*		mDirectionUniform = nullptr;		///< Direction uniform of the blur material
        UniformVec2Instance*		mTextureSizeUniform = nullptr;		///< Texture size uniform of the blur material

		RenderTexture2D*			mTexture = nullptr;
	};
}
