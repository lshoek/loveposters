#include "LevelMeterParameterComponent.h"

// External Includes
#include <entity.h>

// nap::LevelMeterParameterComponent run time class definition 
RTTI_BEGIN_CLASS(nap::LevelMeterParameterComponent)
	RTTI_PROPERTY("LevelMeter",				&nap::LevelMeterParameterComponent::mLevelMeter,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LevelMeterParameter",	&nap::LevelMeterParameterComponent::mLevelMeterParam,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MultiplyParameter",		&nap::LevelMeterParameterComponent::mMultiplyParam,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SmoothTime",				&nap::LevelMeterParameterComponent::mSmoothtime,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::LevelMeterParameterComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LevelMeterParameterComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void LevelMeterParameterComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool LevelMeterParameterComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<LevelMeterParameterComponent>();
		mLevelSmoother.mSmoothTime = mResource->mSmoothtime;
		return true;
	}


	void LevelMeterParameterComponentInstance::update(double deltaTime)
	{
		float multiply = mResource->mMultiplyParam != nullptr ? mResource->mMultiplyParam->mValue : 1.0f;
		float level = mLevelSmoother.update(mLevelMeter->getLevel() * multiply, static_cast<float>(deltaTime));
		mResource->mLevelMeterParam->setValue(level);
	}
}
