/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <nap/signalslot.h>
#include <oscevent.h>
#include <queue>
#include <parameternumeric.h>
#include <parametergroup.h>

namespace nap
{
    // Forward Declare
    class OscHandlerComponentInstance;
   
	/**
	 * Component that converts incoming osc messages into a string and stores them for display later on.
	 */
    class NAPAPI OscHandlerComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(OscHandlerComponent, OscHandlerComponentInstance)
        
    public:
        OscHandlerComponent() : Component() { }
        
        /**
         * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
         * @param components the components this object depends on
         */
        virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		std::vector<ResourcePtr<ParameterGroup>>	mParameterGroups;
		bool										mVerbose = false;

    private:
    };

    
	/**
	* Instance part of the osc handler component. This object registers itself to an osc input component and processes incoming messages.
	*/
    class NAPAPI OscHandlerComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        OscHandlerComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;

		// Return the given parameter's associated osc address
		bool getParameterAddress(ParameterFloat* parameter, std::string& outAddress) const;

		// Return a list of all osc addresses
		const std::vector<std::string>& getAddresses() const;

		std::unique_ptr<ParameterFloat> mOscMasterIntensityParameter;

    private:
		/**
		 * Called when the slot above is send a new message
		 * @param OSCEvent the new osc event
		 */
		void onEventReceived(const OSCEvent&);

		/**
		 * Slot that is connected to the osc input component that receives new messages
		 */
        Slot<const OSCEvent&> eventReceivedSlot = { this, &OscHandlerComponentInstance::onEventReceived };

		// This map holds all the various callbacks based on id
		typedef void (OscHandlerComponentInstance::*OscEventFunc)(const OSCEvent&, ParameterFloat&);

		// Simple struct that binds a function to a parameter
		struct OSCFunctionMapping
		{
			OSCFunctionMapping(OscEventFunc oscFunction, ParameterFloat& parameter) :
				mFunction(oscFunction), mParameter(&parameter)	{ }

			OscEventFunc mFunction = nullptr;
			ParameterFloat* mParameter  = nullptr;
		};
		std::unordered_map<std::string, OSCFunctionMapping> mOscEventFunctions;

		// Adds a parameter mapping
		void addParameter(std::string oscAddress, ParameterFloat& parameter);

		// Generic parameter update function
		void updateParameter(const OSCEvent& oscEvent, ParameterFloat& parameter);

		// Cached list of addresses for display in the OSC menu
		std::vector<std::string> mCachedAddresses;

		OscHandlerComponent* mResource = nullptr;
	};
}
