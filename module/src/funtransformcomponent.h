#pragma once

#include <component.h>
#include <componentptr.h>
#include <transformcomponent.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

#include "affinetransform.h"

namespace nap
{
	class FunTransformComponentInstance;

	/**
	 * FunTransformComponent
	 */
	class NAPAPI FunTransformComponent : public Component
	{
		RTTI_ENABLE(Component)
			DECLARE_COMPONENT(FunTransformComponent, FunTransformComponentInstance)
	public:
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterFloat> mMovementParam;
		ResourcePtr<ParameterFloat> mIntensityParam;

		ResourcePtr<ParameterFloat> mRotationIntensityParam;
		ResourcePtr<ParameterFloat> mScaleIntensityParam;
		ResourcePtr<ParameterFloat> mTranslateXIntensityParam;
		ResourcePtr<ParameterFloat> mTranslateYIntensityParam;

		ResourcePtr<ParameterFloat> mRotationAccumulatorIntensityParam;
		ResourcePtr<ParameterFloat> mScaleAccumulatorIntensityParam;

		float mMultiplyRotation = 1.0f;
		float mMultiplyScale = 1.0f;
		glm::vec2 mMultiplyTranslation = { 1.0f, 1.0f };

		bool mEnable = true;
	};


	/**
	 * FunTransformComponentInstance	
	 */
	class NAPAPI FunTransformComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FunTransformComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)	{ }

		/**
		 * Initialize FunTransformComponentInstance based on the FunTransformComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the FunTransformComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update FunTransformComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 *
		 */
		void enable(bool enable) { mEnabled = enable; }

	private:
		FunTransformComponent* mResource = nullptr;
		TransformComponentInstance* mTransformComponent = nullptr;

		std::unique_ptr<AffineTransform> mCachedTransform;
		glm::vec4 mRandomSeed;

		float mRotationTime = 0.0f;
		float mRotationAccumulator = 0.0f;
		float mScaleAccumulator = 0.0f;
		glm::vec2 mTranslationAccumulator = { 0.0f, 0.0f };

		bool mEnabled = true;
	};
}
