#include "movecameracomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>
#include <orthocameracomponent.h>

// nap::MoveOrthoCameraComponent run time class definition 
RTTI_BEGIN_CLASS(nap::MoveCameraComponent)
	RTTI_PROPERTY("Movement", &nap::MoveCameraComponent::mMovementParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity", &nap::MoveCameraComponent::mIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Offset", &nap::MoveCameraComponent::mOffsetParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RotationHorizontal", &nap::MoveCameraComponent::mRotationHorizontal, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotationVertical", &nap::MoveCameraComponent::mRotationVertical, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AnchorDistance", &nap::MoveCameraComponent::mAnchorDistance, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Pan", &nap::MoveCameraComponent::mPan, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShiftHorizontal", &nap::MoveCameraComponent::mShiftHorizontal, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShiftVertical", &nap::MoveCameraComponent::mShiftVertical, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotateClockSpeed", &nap::MoveCameraComponent::mRotateClockSpeed, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShiftClockSpeed", &nap::MoveCameraComponent::mShiftClockSpeed, nap::rtti::EPropertyMetaData::Default)
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
		mRandomSeed = {
			glm::linearRand<float>(0.0f, 1000.0f),
			glm::linearRand<float>(0.0f, 1000.0f),
			glm::linearRand<float>(0.0f, 1000.0f),
		};
		mRandomSeedShift = {
			glm::linearRand<float>(0.0f, 1000.0f),
			glm::linearRand<float>(0.0f, 1000.0f)
		};
		return true;
	}


	void MoveCameraComponentInstance::update(double deltaTime)
	{
		if (!mResource->mEnable)
			return;

		mMovementTime += static_cast<float>(deltaTime) * mResource->mIntensityParam->mValue;

		const float shift_speed = mMovementTime * mResource->mShiftClockSpeed;
		const glm::vec2 noise_shift = {
			glm::simplex<float>(glm::vec2(shift_speed + mRandomSeedShift.x, mRandomSeedShift.x)),
			glm::simplex<float>(glm::vec2(shift_speed + mRandomSeedShift.y, mRandomSeedShift.y)),
		};
		const auto shift = noise_shift * glm::vec2(mResource->mShiftHorizontal, mResource->mShiftVertical);
		const auto anchor = mCachedTransform->mTranslate + mResource->mOffsetParam->mValue + glm::vec3(shift, 0.0f);

		const float rotate_speed = mMovementTime * mResource->mRotateClockSpeed;
		const glm::vec3 noise = {
			glm::simplex<float>(glm::vec2(rotate_speed + mRandomSeed.x, mRandomSeed.x)),
			glm::simplex<float>(glm::vec2(rotate_speed + mRandomSeed.y, mRandomSeed.y)),
			glm::simplex<float>(glm::vec2(rotate_speed + mRandomSeed.z, mRandomSeed.z))
		};
		const float yaw = noise.x * mResource->mRotationHorizontal * glm::half_pi<float>();
		const float pitch = noise.y * mResource->mRotationVertical * glm::half_pi<float>();

		const glm::mat4 yaw_rotation = glm::rotate(yaw, math::Y_AXIS);
		const glm::vec3& right = mTransformComponent->getLocalTransform()[0];
		const glm::mat4 pitch_rotation = glm::rotate(pitch, right);
		const auto displacement = glm::vec3(0.0f, 0.0f, mResource->mAnchorDistance + (noise.z * 0.5f + 0.5f) * mResource->mPan);
		const glm::mat4 pivot_transform = yaw_rotation * pitch_rotation * glm::translate(displacement);

		const glm::quat rotate = glm::normalize(glm::quat_cast(pivot_transform));
		const glm::vec4 translate = pivot_transform[3] + glm::vec4(anchor, 0.0f);

		mTransformComponent->setTranslate(translate);
		mTransformComponent->setRotate(rotate);
	}
}
