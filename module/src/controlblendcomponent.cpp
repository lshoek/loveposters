#include "controlblendcomponent.h"

// External Includes
#include <entity.h>
#include <materialinstance.h>

// nap::ControlBlendComponent run time class definition
RTTI_BEGIN_CLASS(nap::ControlBlendComponent)
	RTTI_PROPERTY("Blend",      &nap::ControlBlendComponent::mBlend,    	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Brightness", &nap::ControlBlendComponent::mBrightness,   nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Renderer",   &nap::ControlBlendComponent::mRenderer,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::ControlBlendComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControlBlendComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool ControlBlendComponentInstance::init(utility::ErrorState& errorState)
    {
        auto* resource = getComponent<ControlBlendComponent>();

        // Find get or create material instance method
        auto mtl_method = nap::rtti::findMethodRecursive((*mRenderer).get_type(), nap::material::instance::getOrCreateMaterial);
        if (!errorState.check(mtl_method.is_valid(), "Missing method `getOrCreateMaterial()`"))
            return false;

        // Make sure return type is material instance
        auto mtl_return_type = mtl_method.get_return_type();
        if (!errorState.check(mtl_return_type.is_pointer() && mtl_return_type.is_derived_from(RTTI_OF(nap::MaterialInstance)), "Method `getOrCreateMaterial()` has incorrect return type"))
            return false;

        // Get material instance
        auto mtl_result = mtl_method.invoke(*mRenderer);
        assert(mtl_result.is_valid() && mtl_result.get_type().is_pointer() && mtl_result.get_type().is_derived_from(RTTI_OF(nap::MaterialInstance)));

        auto* mtl = mtl_result.get_value<nap::MaterialInstance*>();
        assert(mtl != nullptr);

        auto* ubo = mtl->getOrCreateUniform("UBO");
        if (!errorState.check(ubo != nullptr, "Missing uniform struct with name `UBO`"))
            return false;

        mBlendUniform = ubo->getOrCreateUniform<UniformFloatInstance>("blend");
        if (!errorState.check(mBlendUniform != nullptr, "Missing uniform struct member with name `blend`"))
            return false;

		mBrightnessUniform = ubo->getOrCreateUniform<UniformFloatInstance>("brightness");
		if (!errorState.check(mBrightnessUniform != nullptr, "Missing uniform struct member with name `brightness`"))
			return false;

        resource->mBlend->valueChanged.connect(mBlendChangedSlot);
        resource->mBrightness->valueChanged.connect(mBrightnessChangedSlot);

		return true;
	}


    void ControlBlendComponentInstance::onBlendChanged(float blend)
    {
        mBlendUniform->setValue(blend);
    }


	void ControlBlendComponentInstance::onBrightnessChanged(float brightness)
	{
		mBrightnessUniform->setValue(brightness);
	}
}
