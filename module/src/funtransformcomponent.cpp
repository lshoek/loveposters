#include "funtransformcomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>

// nap::FunTransformComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FunTransformComponent)
	RTTI_PROPERTY("Movement", &nap::FunTransformComponent::mMovementParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity", &nap::FunTransformComponent::mIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RotationIntensity", &nap::FunTransformComponent::mRotationIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RotationAccumulatorIntensity", &nap::FunTransformComponent::mRotationAccumulatorIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TranslateXIntensity", &nap::FunTransformComponent::mTranslateXIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TranslateYIntensity", &nap::FunTransformComponent::mTranslateYIntensityParam, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MultiplyRotation", &nap::FunTransformComponent::mMultiplyRotation, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MultiplyTranslation", &nap::FunTransformComponent::mMultiplyTranslation, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RandomOffset", &nap::FunTransformComponent::mRandomOffset, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Enable", &nap::FunTransformComponent::mEnable, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::FunTransformComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FunTransformComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	static const float sMaxRotationDeviation = 0.125f;
	static const float sMaxScaleDeviation = 0.25f;
	static const float sMaxTranslateDeviation = 0.125f;


	void FunTransformComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	bool FunTransformComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<FunTransformComponent>();
		mEnabled = mResource->mEnable;

		mTransformComponent = &getEntityInstance()->getComponent<TransformComponentInstance>();
		mCachedTransform = std::make_unique<AffineTransform>(*mTransformComponent);
		mRandomize = mResource->mRandomOffset;
		randomize(mRandomize);

		return true;
	}


	void FunTransformComponentInstance::randomize(bool enable)
	{
		static const float rand_max = 1000.0f;
		if (enable)
		{
			mRandomSeed = {
				glm::linearRand<float>(0.0f, rand_max),
				glm::linearRand<float>(0.0f, rand_max),
				glm::linearRand<float>(0.0f, rand_max),
				glm::linearRand<float>(0.0f, rand_max)
			};
			return;
		}
		mRandomSeed = { 0.0f, 0.0f, 0.0f, 0.0f };
	}


	void FunTransformComponentInstance::update(double deltaTime)
	{
		if (!mEnabled)
			return;

		float delta_time = static_cast<float>(deltaTime);
		const float movement = mResource->mMovementParam->mValue * mResource->mIntensityParam->mValue;

		// Rotation
		if (mResource->mMultiplyRotation > 0.0f)
		{
			mRotationAccumulator += movement * mResource->mRotationAccumulatorIntensityParam->mValue * static_cast<float>(deltaTime);
			mRotationTime += delta_time * mResource->mRotationIntensityParam->mValue;
			float input = mRotationTime + mRotationAccumulator;

			float theta = glm::simplex<float>(glm::vec2(input + mRandomSeed.x, mRandomSeed.x) * sMaxRotationDeviation) * 0.5f * glm::pi<float>();
			auto rotate = glm::rotate(glm::identity<glm::quat>(), theta * 0.25f, math::Z_AXIS) * mResource->mMultiplyRotation;
			mTransformComponent->setRotate(rotate);
		}

		// Translation
		if (mResource->mMultiplyTranslation.x > 0.0f || mResource->mMultiplyTranslation.y > 0.0f)
		{
			mTranslationAccumulator += movement * glm::vec2(mResource->mTranslateXIntensityParam->mValue, mResource->mTranslateYIntensityParam->mValue) * static_cast<float>(deltaTime);
			auto move_translate = glm::vec2(glm::simplex<float>(glm::vec2(mTranslationAccumulator.x + mRandomSeed.z, mRandomSeed.z)), glm::simplex<float>(glm::vec2(mTranslationAccumulator.y + mRandomSeed.w, mRandomSeed.w))) * sMaxTranslateDeviation;
			auto translate = mCachedTransform->mTranslate + glm::vec3(move_translate.x, move_translate.y, 0.0f);
			mTransformComponent->setTranslate(translate);
		}
	}
}
