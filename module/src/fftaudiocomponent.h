/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "fftnode.h"

// Nap includes
#include <component.h>
#include <parameternumeric.h>
#include <audio/utility/safeptr.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>
#include <audio/node/filternode.h>

namespace nap
{
	class FFTAudioComponentInstance;
			
	/**
	 * Component to measure the fft of the audio signal from an @AudioComponentBase.
	 */
	class NAPAPI FFTAudioComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FFTAudioComponent, FFTAudioComponentInstance)	
	public:

		class NAPAPI FilterParameterItem : public Resource
		{
			RTTI_ENABLE(Resource)
		public:
			bool init(utility::ErrorState& errorState) override { return true; }

			rtti::ObjectPtr<ParameterFloat> mParameter;
			uint mMinBin = 0;
			uint mMaxBin = 1;
		};

		FFTAudioComponent() :
			Component() {}
			
		nap::ComponentPtr<audio::AudioComponentBase> mInput;		///< property: 'Input' The component whose audio output will be measured.
		int mChannel = 0;											///< property: 'Channel' Channel of the input that will be analyzed.

		std::vector<rtti::ObjectPtr<FilterParameterItem>> mParameters;
	};
		
		
	/**
	 * Instance of component to measure the amplitude level of the audio signal from an @AudioComponentBase.
	 * A specific frequency band to be measured can be specified.
	 */
	class NAPAPI FFTAudioComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FFTAudioComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}
			
		// Initialize the component
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Update this component
		 * @param deltaTime the time in between cooks in seconds
		 */
		virtual void update(double deltaTime) override;
			
		/**
		 * Connects a different audio component as input to be analyzed.
		 */
		void setInput(audio::AudioComponentBaseInstance& input);
		
	private:
		ComponentInstancePtr<audio::AudioComponentBase> mInput = { this, &FFTAudioComponent::mInput };		// Pointer to component that outputs this components audio input
		audio::SafeOwner<FFTNode> mFFTNode = nullptr;														// Node doing the actual analysis
			
		FFTAudioComponent* mResource = nullptr;
		audio::AudioService* mAudioService = nullptr;
	};

}
