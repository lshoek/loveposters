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
	RTTI_PROPERTY("FocusDepth", &nap::MoveCameraComponent::mFocusDepth, nap::rtti::EPropertyMetaData::Default)
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

		glm::vec3 noise = {
			glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.x, mRandomSeed.x)),
			glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.y, mRandomSeed.y)),
			glm::simplex<float>(glm::vec2(movement_speed + mRandomSeed.z, mRandomSeed.z))
		};

		float theta_x = noise.x * mResource->mMoveExtents.x * glm::half_pi<float>();
		float distance = ((noise.z + 1.0f) * 0.5f) * mResource->mMoveExtents.z;
		glm::vec3 polar_translate = glm::angleAxis(theta_x, math::Y_AXIS) * glm::vec3(0.0f, 0.0f, distance);
		glm::vec3 height_translate = { 0.0f, noise.y * mResource->mMoveExtents.y, 0.0f };

		auto translate = mCachedTransform->mTranslate + height_translate  + polar_translate;
		mTransformComponent->setTranslate(translate);

		// Focus
		glm::vec3 focus_point = { 0.0f, 0.0f, -mResource->mFocusDepth };
		glm::mat3 orient_mat = glm::lookAt(translate, focus_point, math::Y_AXIS);
		const auto lookat_quat = glm::quat_cast(glm::transpose(orient_mat));
		mTransformComponent->setRotate(lookat_quat);
	}
}
