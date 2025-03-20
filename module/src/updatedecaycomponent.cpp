/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "updatedecaycomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <blinnphongcolorshader.h>

// nap::UpdateDecayComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateDecayComponent)
	RTTI_PROPERTY("Renderer",		&nap::UpdateDecayComponent::mRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClockSpeed",		&nap::UpdateDecayComponent::mClockSpeed,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Decay",			&nap::UpdateDecayComponent::mDecay,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Vertical",		&nap::UpdateDecayComponent::mVertical,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Tangent",		&nap::UpdateDecayComponent::mTangent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Expand",			&nap::UpdateDecayComponent::mExpand,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ExpandBlend",	&nap::UpdateDecayComponent::mExpandBlend,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NoiseScale",		&nap::UpdateDecayComponent::mNoiseScale,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::UpdateDecayComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateDecayComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool UpdateDecayComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<UpdateDecayComponent>();

		// Find get or create material instance method
		auto mtl_method = rtti::findMethodRecursive((*mRenderer).get_type(), material::instance::getOrCreateMaterial);
		if (!errorState.check(mtl_method.is_valid(), "Missing method `getOrCreateMaterial()`"))
			return false;

		// Make sure return type is material instance
		auto mtl_return_type = mtl_method.get_return_type();
		if (!errorState.check(mtl_return_type.is_pointer() && mtl_return_type.is_derived_from(RTTI_OF(MaterialInstance)), "Method `getOrCreateMaterial()` has incorrect return type"))
			return false;

		// Get material instance
		auto mtl_result = mtl_method.invoke(*mRenderer);
		assert(mtl_result.is_valid() && mtl_result.get_type().is_pointer() && mtl_result.get_type().is_derived_from(RTTI_OF(MaterialInstance)));

		auto* mtl = mtl_result.get_value<MaterialInstance*>();
		assert(mtl != nullptr);

		mUniformStruct = mtl->getOrCreateUniform("UBO");
		if (!errorState.check(mUniformStruct != nullptr, "Missing uniform struct with name `UBO`"))
			return false;

		return true;
	}


	void UpdateDecayComponentInstance::update(double deltaTime)
	{
		mElapsedTime += deltaTime;
		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("elapsedTime")->setValue(static_cast<float>(mElapsedTime));

		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("decay")->setValue(mResource->mDecay->mValue);
		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("vertical")->setValue(mResource->mVertical->mValue);
		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("tangent")->setValue(mResource->mTangent->mValue);
		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("expand")->setValue(mResource->mExpand->mValue);
		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("expandBlend")->setValue(mResource->mExpandBlend->mValue);
		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("noiseScale")->setValue(mResource->mNoiseScale->mValue);
	}
}
