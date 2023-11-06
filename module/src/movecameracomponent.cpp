#include "movecameracomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>
#include <orthocameracomponent.h>

// nap::MoveOrthoCameraComponent run time class definition 
RTTI_BEGIN_CLASS(nap::MoveCameraComponent)
	RTTI_PROPERTY("Movement", &nap::MoveCameraComponent::mMovementParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity", &nap::MoveCameraComponent::mIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MultiplyIntensity", &nap::MoveCameraComponent::mMultiplyIntensity, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MoveExtents", &nap::MoveCameraComponent::mMoveExtents, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Enable", &nap::MoveCameraComponent::mEnable, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::MoveOrthoCameraComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MoveCameraComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void MoveCameraComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(CameraComponent));
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	bool MoveCameraComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<MoveCameraComponent>();
		mTransformComponent = &getEntityInstance()->getComponent<TransformComponentInstance>();
		mCachedTransform = std::make_unique<AffineTransform>(*mTransformComponent);
		mRandomSeed = { glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f) };

		return true;
	}


	void MoveCameraComponentInstance::update(double deltaTime)
	{
		if (!mResource->mEnable)
			return;

		mMovementTime += static_cast<float>(deltaTime) * mResource->mIntensityParam->mValue;

		float movement_speed = mMovementTime * mResource->mMultiplyIntensity;
		glm::vec3 move_translate = {
			glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.x, mRandomSeed.x)),
			glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.y, mRandomSeed.y)),
			glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.z, mRandomSeed.z))
		};
		auto translate = mCachedTransform->mTranslate + move_translate * mResource->mMoveExtents;
		mTransformComponent->setTranslate(translate);
	}
}
