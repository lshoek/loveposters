#pragma once

#include <component.h>
#include <parameternumeric.h>

#include "affinetransform.h"

namespace nap
{
	class MoveCameraComponentInstance;

	/**
	 *	MoveOrthoCameraComponent
	 */
	class NAPAPI MoveCameraComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(MoveCameraComponent, MoveCameraComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterFloat> mMovementParam;
		ResourcePtr<ParameterFloat> mIntensityParam;

		glm::vec3 mMoveExtents = { 1.0f, 1.0f, 0.0f };
		float mMultiplyIntensity = 1.0f;
		float mFocusDepth = -1.0f;
		bool mEnable = true;
	};


	/**
	 * MoveOrthoCameraComponentInstance	
	 */
	class NAPAPI MoveCameraComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		MoveCameraComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize MoveOrthoCameraComponentInstance based on the MoveOrthoCameraComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the MoveOrthoCameraComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update MoveOrthoCameraComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		MoveCameraComponent* mResource = nullptr;
		TransformComponentInstance* mTransformComponent = nullptr;

		std::unique_ptr<AffineTransform> mCachedTransform;
		glm::vec4 mRandomSeed;

		float mMovementTime = 0.0f;
		glm::vec2 mTranslationAccumulator = { 0.0f, 0.0f };
	};
}
