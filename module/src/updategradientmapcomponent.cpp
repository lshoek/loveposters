/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "updategradientmapcomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <blinnphongcolorshader.h>

// nap::UpdateGradientMapComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateGradientMapComponent)
	RTTI_PROPERTY("Renderer",	&nap::UpdateGradientMapComponent::mRenderer,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClockSpeed",	&nap::UpdateGradientMapComponent::mClockSpeed,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color0",		&nap::UpdateGradientMapComponent::mColor0,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color1",		&nap::UpdateGradientMapComponent::mColor1,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color2",		&nap::UpdateGradientMapComponent::mColor2,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color3",		&nap::UpdateGradientMapComponent::mColor3,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color4",		&nap::UpdateGradientMapComponent::mColor4,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::UpdateGradientMapComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateGradientMapComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool UpdateGradientMapComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<UpdateGradientMapComponent>();

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


	void UpdateGradientMapComponentInstance::update(double deltaTime)
	{
		mElapsedTime += deltaTime * mResource->mClockSpeed->mValue;
		mUniformStruct->getOrCreateUniform<UniformFloatInstance>("elapsedTime")->setValue(static_cast<float>(mElapsedTime));

		auto* colors = mUniformStruct->getOrCreateUniform<UniformVec3ArrayInstance>("colors");
		assert(colors != nullptr);

		colors->setValue(mResource->mColor0->mValue.toVec3(), 0);
		colors->setValue(mResource->mColor1->mValue.toVec3(), 1);
		colors->setValue(mResource->mColor2->mValue.toVec3(), 2);
		colors->setValue(mResource->mColor3->mValue.toVec3(), 3);
		colors->setValue(mResource->mColor4->mValue.toVec3(), 4);
	}
}
