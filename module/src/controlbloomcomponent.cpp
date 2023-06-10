#include "controlbloomcomponent.h"

// External Includes
#include <entity.h>

// nap::ControlBloomComponent run time class definition 
RTTI_BEGIN_CLASS(nap::ControlBloomComponent)
	RTTI_PROPERTY("Movement", &nap::ControlBloomComponent::mMovementParam, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Intensity", &nap::ControlBloomComponent::mIntensityParam, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RenderToTexture", &nap::ControlBloomComponent::mRenderToTextureComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::ControlBloomComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControlBloomComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ControlBloomComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
	}


	bool ControlBloomComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<ControlBloomComponent>();

		auto* uni_struct = mRenderToTextureComponent->getMaterialInstance().getOrCreateUniform("FRAGUBO");
		if (!errorState.check(uni_struct != nullptr, "%s: Incompatible shader interface", mID.c_str()))
			return false;

		mBlendUniform = uni_struct->getOrCreateUniform<UniformFloatInstance>("blend");
		if (!errorState.check(mBlendUniform != nullptr, "%s: Incompatible shader interface", mID.c_str()))
			return false;

		return true;
	}


	void ControlBloomComponentInstance::update(double deltaTime)
	{
		float intensity = (mResource->mIntensityParam != nullptr) ? mResource->mIntensityParam->mValue : 0.0f;
		float movement = (mResource->mMovementParam != nullptr) ? mResource->mMovementParam->mValue : 1.0f;
		mBlendUniform->setValue(movement * intensity);
	}
}
