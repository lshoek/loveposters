#pragma once

#include <component.h>
#include <componentptr.h>
#include <transformcomponent.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <parametervec.h>

#include "affinetransform.h"

namespace nap
{
	class UpdateTransformComponentInstance;

	/**
	 * UpdateTransformComponent
	 */
	class NAPAPI UpdateTransformComponent : public Component
	{
		RTTI_ENABLE(Component)
			DECLARE_COMPONENT(UpdateTransformComponent, UpdateTransformComponentInstance)
	public:
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterVec3> mPosition;
		ResourcePtr<ParameterVec3> mScale;
		ResourcePtr<ParameterFloat> mAngle;
		bool mEnable = true;
	};


	/**
	 * UpdateTransformComponentInstance	
	 */
	class NAPAPI UpdateTransformComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateTransformComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)	{ }

		/**
		 * Initialize UpdateTransformComponentInstance based on the UpdateTransformComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateTransformComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update UpdateTransformComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		UpdateTransformComponent* mResource = nullptr;
		TransformComponentInstance* mTransformComponent = nullptr;

		std::unique_ptr<AffineTransform> mCachedTransform;
		glm::vec4 mRandomSeed = { 0.0f, 0.0f, 0.0f, 0.0f };

		float mRotationTime = 0.0f;
		float mRotationAccumulator = 0.0f;
		glm::vec2 mTranslationAccumulator = { 0.0f, 0.0f };

		bool mEnabled = true;
	};
}
