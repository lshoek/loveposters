/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "oschandlercomponent.h"

// External includes
#include <entity.h>
#include <oscinputcomponent.h>

#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::OscHandlerComponent)
	RTTI_PROPERTY("ParameterGroup",	&nap::OscHandlerComponent::mParameterGroup,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",		&nap::OscHandlerComponent::mVerbose,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OscHandlerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    void OscHandlerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.emplace_back(RTTI_OF(nap::OSCInputComponent));
    }

    
    bool OscHandlerComponentInstance::init(utility::ErrorState& errorState)
    {
		OSCInputComponentInstance* osc_input = getEntityInstance()->findComponent<OSCInputComponentInstance>();
		if (!errorState.check(osc_input != nullptr, "%s: missing OSCInputComponent", mID.c_str()))
			return false;

        osc_input->messageReceived.connect(eventReceivedSlot);
		const std::string filter = !osc_input->mAddressFilter.empty() ? osc_input->mAddressFilter[0] : "default";

		// Get the resource part of the component
		mResource = getComponent<OscHandlerComponent>();

		// Register parametersq
		for (auto& param : mResource->mParameterGroup->mMembers)
		{
			// Filter float parameters for simplicity
			if (!errorState.check(param.get()->get_type().is_derived_from(RTTI_OF(ParameterFloat)), "%s: Unsupported parameter type found in group", mID.c_str()))
				return false;

			// Cast to float param
			auto* fparam = static_cast<ParameterFloat*>(param.get());

			// Construct an osc address for this parameter
			std::vector<std::string> elements = { filter, param->getDisplayName() };
			const auto address = utility::joinString(elements, "/");
			addParameter(address, *fparam);
			mCachedAddresses.emplace_back(address);		
		}
        return true;
    }


	void OscHandlerComponentInstance::addParameter(std::string oscAddress, ParameterFloat& parameter)
	{
		auto it = mOscEventFunctions.emplace(oscAddress, OSCFunctionMapping(&OscHandlerComponentInstance::updateParameter, parameter));
		bool success = it.second;

		if (!success)
		{
			nap::Logger::warn("%s: Duplicate parameter with name: %s", mResource->mID.c_str(), parameter.getDisplayName().c_str());
		}
		else if (mResource->mVerbose)
		{
			nap::Logger::info("%s: Parameter registered with OSC address '%s'", mResource->mID.c_str(), oscAddress.c_str());
		}
	}


	void OscHandlerComponentInstance::updateParameter(const OSCEvent& oscEvent, ParameterFloat& parameter)
	{
		assert(oscEvent.getCount() >= 1);
		float lvalue = math::lerp<float>(parameter.mMinimum, parameter.mMaximum, oscEvent[0].asFloat());
		parameter.setValue(lvalue);

		if (mResource->mVerbose)
			nap::Logger::info("%s: %s = %.02f", mResource->mID.c_str(), oscEvent.getAddress().c_str(), lvalue);
	}


	// Generic parameter update function
	void OscHandlerComponentInstance::updateMasterParameter(const OSCEvent& oscEvent, ParameterFloat& parameter)
	{
		//assert(oscEvent.getCount() >= 1);

		//// Update master parameter
		//float lvalue = math::lerp<float>(parameter.getMin(), parameter.getMax(), oscEvent[0].asFloat());
		//parameter.setValue(lvalue);

		//// Update controller masters
		//for (auto& ctrl : mResource->mRingComposer->mControllers)
		//{
		//	if (ctrl->getMasterEffect() != nullptr)
		//		ctrl->getMasterEffect()->mIntensity->setValue(lvalue);
		//}

		//if (mResource->mVerbose)
		//	nap::Logger::info("%s: %s = %.02f", mResource->mID.c_str(), oscEvent.getAddress().c_str(), lvalue);
	}

    
    void OscHandlerComponentInstance::onEventReceived(const OSCEvent& event)
    {
		// Find matching osc function
		auto osc_func = mOscEventFunctions.find(event.getAddress());
		if (osc_func != mOscEventFunctions.end())
			(this->*(osc_func->second.mFunction))(event, *(osc_func->second.mParameter));
    }


	bool OscHandlerComponentInstance::getParameterAddress(ParameterFloat* parameter, std::string& address) const
	{
		const auto& it = std::find_if(mOscEventFunctions.begin(), mOscEventFunctions.end(), [&](const auto& it) {
			return it.second.mParameter == parameter;
		});
		if (it != mOscEventFunctions.end())
		{
			address = it->first;
			return true;
		}
		return false;
	}


	const std::vector<std::string>& OscHandlerComponentInstance::getAddresses() const
	{
		return mCachedAddresses;
	}
}
