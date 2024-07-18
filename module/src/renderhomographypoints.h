/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <rendercomponent.h>
#include <mesh.h>
#include <renderablemesh.h>
#include <materialinstance.h>

namespace nap
{
    // Forward Declares
    class RenderHomographyPointsInstance;
	class HomographyComponentInstance;
	class TransformComponentInstance;
	class RenderService;
	class PointMesh;

	/**
	 * RenderHomographyPoints
	 */
    class RenderHomographyPoints : public RenderableComponent
    {
        RTTI_ENABLE(RenderableComponent)
        DECLARE_COMPONENT(RenderHomographyPoints, RenderHomographyPointsInstance)
    public:     
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		RGBColorFloat mColor = { 1.0f, 1.0f, 1.0f };
	};


	/**
	 * RenderHomographyPointsInstance
	 */
    class NAPAPI RenderHomographyPointsInstance : public RenderableComponentInstance
    {
		RTTI_ENABLE(RenderableComponentInstance)
    public:
		RenderHomographyPointsInstance(EntityInstance& entity, Component& resource);

        virtual bool init(utility::ErrorState& errorState) override;

        virtual void update(double deltaTime) override;

		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

    private:
        bool updateMeshInstance(utility::ErrorState& errorState);

		HomographyComponentInstance*			mHomographyComponent = nullptr;
		RenderService*							mRenderService = nullptr;
		TransformComponentInstance*				mTransformComponent;				///< Cached pointer to transform

		std::unique_ptr<PointMesh>				mPointMesh;							///< The mesh to render
		MaterialInstance						mMaterialInstance;					///< The MaterialInstance as created from the resource. 
		MaterialInstanceResource				mMaterialInstanceResource;			///< Resource used to initialize the material instance
		math::Rect								mClipRect;							///< Clipping rectangle for this instance, in pixel coordinates
		RenderableMesh							mRenderableMesh;					///< The currently active renderable mesh, either set during init() or set by setMesh.

		UniformMat4Instance*					mModelMatUniform = nullptr;			///< Pointer to the model matrix uniform
		UniformMat4Instance*					mViewMatUniform = nullptr;			///< Pointer to the view matrix uniform
		UniformMat4Instance*					mProjectMatUniform = nullptr;		///< Pointer to the projection matrix uniform
		UniformMat4Instance*					mNormalMatrixUniform = nullptr;		///< Pointer to the normal matrix uniform
		UniformVec3Instance*					mCameraWorldPosUniform = nullptr;	///< Pointer to the camera world position uniform

		Vec3VertexAttribute*					mPointsAttribute = nullptr;

		glm::vec2								mTopLeft;
		glm::vec2								mTopRight;
		glm::vec2								mBottomRight;
		glm::vec2								mBottomLeft;
    };
}
