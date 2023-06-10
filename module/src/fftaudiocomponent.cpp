/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fftaudiocomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS(nap::FFTAudioComponent::FilterParameterItem)
	RTTI_PROPERTY("Parameter",		&nap::FFTAudioComponent::FilterParameterItem::mParameter,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MinimumBin",		&nap::FFTAudioComponent::FilterParameterItem::mMinBin,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaximumBin",		&nap::FFTAudioComponent::FilterParameterItem::mMaxBin,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FFTAudioComponent)
	RTTI_PROPERTY("Input",			&nap::FFTAudioComponent::mInput,							nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Channel",		&nap::FFTAudioComponent::mChannel,							nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Parameters",		&nap::FFTAudioComponent::mParameters,						nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FFTAudioComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance &, nap::Component &)
RTTI_END_CLASS

namespace nap
{		
	bool FFTAudioComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<FFTAudioComponent>();
		mAudioService = getEntityInstance()->getCore()->getService<audio::AudioService>();
		auto& nodeManager = mAudioService->getNodeManager();
			
		if (!errorState.check(mResource->mChannel < mInput->getChannelCount(), "%s: Channel exceeds number of input channels", mResource->mID.c_str()))
			return false;
			
		mFFTNode = nodeManager.makeSafe<FFTNode>(nodeManager);		
		mFFTNode->mInput.connect(*mInput->getOutputForChannel(mResource->mChannel));
			
		return true;
	}


	void FFTAudioComponentInstance::update(double deltaTime)
	{
		if (mFFTNode->isUpdated())
		{
			const auto& fft = mFFTNode->getFFT();
			assert(!fft.empty());

			for (auto& entry : mResource->mParameters)
			{
				float sum = 0.0f;
				for (uint i = entry->mMinBin; i < entry->mMaxBin; i++)
				{
					glm::vec2 bin = { fft[i].real(), fft[i].imag() };
					sum += glm::length(bin);
				}
				float avg = sum / entry->mMaxBin - entry->mMinBin;
				entry->mParameter->setValue(avg);
			}
		}
	}
		
		
	void FFTAudioComponentInstance::setInput(audio::AudioComponentBaseInstance& input)
	{
		auto inputPtr = &input;
		mAudioService->enqueueTask([&, inputPtr]() {
			mFFTNode->mInput.connect(*inputPtr->getOutputForChannel(mResource->mChannel));
		});
	}
}
