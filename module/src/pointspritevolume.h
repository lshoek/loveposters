#pragma once
#include "particlemesh.h"

// External Includes
#include <renderablemesh.h>
#include <renderablemeshcomponent.h>
#include <componentptr.h>
#include <uniforminstance.h>
#include <materialinstance.h>
#include <parameternumeric.h>

namespace nap
{
	// Forward declares
	class PointSpriteVolumeInstance;
	class TransformComponentInstance;

	class PointSpriteVolume : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(PointSpriteVolume, PointSpriteVolumeInstance)

	public:
		ResourcePtr<ParameterFloat> mPointSize;
		ResourcePtr<ParameterFloat> mPointScale;
		ResourcePtr<ParameterFloat> mPointScaleIntensity;
		ResourcePtr<ParameterFloat> mTimeScale;
	};



	class PointSpriteVolumeInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)

	public:
		PointSpriteVolumeInstance(EntityInstance& entity, Component& resource);

		virtual bool init(nap::utility::ErrorState& errorState) override;
		virtual void update(double deltaTime) override;

		/**
		* Draws a volume of point sprites 
		* @param viewMatrix the camera world space location
		* @param projectionMatrix the camera projection matrix
		*/
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		PointSpriteVolume* mResource = nullptr;

		RenderService* mRenderService = nullptr;
		TransformComponentInstance* mTransform = nullptr;

		UniformFloatInstance* mPointSizeUniform = nullptr;
		UniformFloatInstance* mPointScaleUniform = nullptr;
		UniformFloatInstance* mElapsedTimeUniform = nullptr;

		uint mCount = 1;
		float mElapsedClockTime = 0.0f;
	};
}
