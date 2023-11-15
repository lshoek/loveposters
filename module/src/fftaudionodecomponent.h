/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "fftaudiocomponent.h"
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
	class FFTAudioNodeComponentInstance;
			
	/**
	 * Component to measure the fft of the audio signal from an @AudioComponentBase.
	 */
	class NAPAPI FFTAudioNodeComponent : public FFTAudioComponent
	{
		RTTI_ENABLE(FFTAudioComponent)
		DECLARE_COMPONENT(FFTAudioNodeComponent, FFTAudioNodeComponentInstance)	
	public:
		FFTAudioNodeComponent() :
			FFTAudioComponent() {}
			
		nap::ComponentPtr<audio::AudioComponentBase> mInput;		///< Property: 'Input' The component whose audio output will be measured.
		int mChannel = 0;											///< Property: 'Channel' Channel of the input that will be analyzed.
	};
		
		
	/**
	 * Instance of component to measure the amplitude level of the audio signal from an @AudioComponentBase.
	 * A specific frequency band to be measured can be specified.
	 */
	class NAPAPI FFTAudioNodeComponentInstance : public FFTAudioComponentInstance
	{
		RTTI_ENABLE(FFTAudioComponentInstance)
	public:
		FFTAudioNodeComponentInstance(EntityInstance& entity, Component& resource) :
			FFTAudioComponentInstance(entity, resource) {}
			
		// Initialize the component
		virtual bool init(utility::ErrorState& errorState) override;
			
		/**
		 * Connects a different audio component as input to be analyzed.
		 */
		void setInput(audio::AudioComponentBaseInstance& input);

		/**
		 * 
		 */
		virtual const FFTBuffer& getFFTBuffer() const override		{ return *mFFTBuffer; }

		/**
		 *
		 */
		virtual FFTBuffer& getFFTBuffer() override					{ return *mFFTBuffer; }
	
	private:
		ComponentInstancePtr<audio::AudioComponentBase> mInput = { this, &FFTAudioNodeComponent::mInput };	// Pointer to component that outputs this components audio input

		FFTAudioNodeComponent* mResource = nullptr;
		audio::SafeOwner<FFTNode> mFFTNode = nullptr;														// Node doing the actual analysis
		
		audio::AudioService* mAudioService = nullptr;
		FFTBuffer* mFFTBuffer = nullptr;
	};
}