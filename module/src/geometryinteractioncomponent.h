#pragma once

// External includes
#include <renderablemeshcomponent.h>
#include <perspcameracomponent.h>
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <inputevent.h>
#include <renderwindow.h>
#include <spheremesh.h>

namespace nap
{
	// Forward declares
	class GeometryInteractionComponentInstance;
	class RenderService;

	/**
	 * GeometryInteractionComponent
	 */
	class NAPAPI GeometryInteractionComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(GeometryInteractionComponent, GeometryInteractionComponentInstance)
	public:
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		virtual bool init(utility::ErrorState& errorState) override;

		ComponentPtr<PerspCameraComponent>		mCamera;
		ResourcePtr<RenderWindow>				mRenderWindow;
		RGBColorFloat							mColor = { 1.0f, 1.0f, 1.0f };

		std::vector<ComponentPtr<RenderableMeshComponent>> mInteractionGeometries;
	};


	/**
	 * GeometryInteractionComponentInstance
	 */
	class NAPAPI GeometryInteractionComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		GeometryInteractionComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * @param error should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Renders the model from the ModelResource, using the material on the ModelResource.
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		virtual void onDestroy() override { getComponent<GeometryInteractionComponent>()->mMaterialInstanceResource.mMaterial = nullptr; }

		/**
		 * 
		 */
		bool getIntersectionData(RenderableMeshComponentInstance* outComp, glm::vec3& outPosition, glm::vec3& outUV) const;

	private:
		RenderService* mRenderService = nullptr;

		// Mesh
		std::unique_ptr<SphereMesh> mSphereMesh;

		// Events
		void onMouseDown(const PointerPressEvent& pressEvent);
		void onMouseMove(const PointerMoveEvent& moveEvent);
		void onMouseUp(const PointerReleaseEvent& releaseEvent);	

		ComponentInstancePtr<PerspCameraComponent> mCamera = { this, &nap::GeometryInteractionComponent::mCamera };
		std::vector<ComponentInstancePtr<RenderableMeshComponent>> mGeometries = initComponentInstancePtr(this, &nap::GeometryInteractionComponent::mInteractionGeometries);

		bool									mIntersects = false;
		glm::vec3								mIntersectionWorldPosition;
		glm::vec3								mIntersectionUV;
		RenderableMeshComponentInstance*		mIntersectionGeometry = nullptr;

		bool									mMousePressed = false;
		glm::ivec2								mMousePosition = {0, 0};
	};
}
