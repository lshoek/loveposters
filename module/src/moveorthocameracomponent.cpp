#include "moveorthocameracomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>
#include <orthocameracomponent.h>

// nap::MoveOrthoCameraComponent run time class definition 
RTTI_BEGIN_CLASS(nap::MoveOrthoCameraComponent)
	RTTI_PROPERTY("Movement", &nap::MoveOrthoCameraComponent::mMovementParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity", &nap::MoveOrthoCameraComponent::mIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MultiplyIntensity", &nap::MoveOrthoCameraComponent::mMultiplyIntensity, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MoveExtents", &nap::MoveOrthoCameraComponent::mMoveExtents, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Enable", &nap::MoveOrthoCameraComponent::mEnable, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::MoveOrthoCameraComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MoveOrthoCameraComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void MoveOrthoCameraComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(OrthoCameraComponent));
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	bool MoveOrthoCameraComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<MoveOrthoCameraComponent>();
		mTransformComponent = &getEntityInstance()->getComponent<TransformComponentInstance>();
		mCachedTransform = std::make_unique<AffineTransform>(*mTransformComponent);
		mRandomSeed = { glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f) };

		return true;
	}


	void MoveOrthoCameraComponentInstance::update(double deltaTime)
	{
		if (!mResource->mEnable)
			return;

		mMovementTime += static_cast<float>(deltaTime) * mResource->mIntensityParam->mValue;

		float movement_speed = mMovementTime * mResource->mMultiplyIntensity;
		auto move_translate = glm::vec2(glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.x, mRandomSeed.x)), glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.y, mRandomSeed.y)));
		auto translate = mCachedTransform->mTranslate + glm::vec3(move_translate.x, move_translate.y, 0.0f) * glm::vec3(mResource->mMoveExtents, 1.0f);
		mTransformComponent->setTranslate(translate);
	}
}
