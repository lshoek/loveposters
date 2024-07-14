/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "legacyfluxmeasurementcomponent.h"
#include "fftaudionodecomponent.h"
#include "fftutils.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::LegacyFluxMeasurementComponent::FilterParameterItem)
	RTTI_PROPERTY("Parameter", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mParameter, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Multiplier", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mMultiplier, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Offset", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mOffset, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TargetOnset", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mTargetOnset, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Decay", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mDecay, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Stretch", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mStretch, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OnsetImpact", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mOnsetImpact, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinHertz", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mMinHz, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxHertz", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mMaxHz, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EvaluationSampleCount", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mEvaluationSampleCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SmoothTime", &nap::LegacyFluxMeasurementComponent::FilterParameterItem::mSmoothTime, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LegacyFluxMeasurementComponent)
	RTTI_PROPERTY("Parameters", &nap::LegacyFluxMeasurementComponent::mParameters, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Enable", &nap::LegacyFluxMeasurementComponent::mEnable, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LegacyFluxMeasurementComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	// Always ensure a decreasing gradient
	static const float sFluxEpsilon = 0.0001f;


	//////////////////////////////////////////////////////////////////////////
	// LegacyFluxMeasurementComponent
	//////////////////////////////////////////////////////////////////////////

	void LegacyFluxMeasurementComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(FFTAudioNodeComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// LegacyFluxMeasurementComponentInstance
	//////////////////////////////////////////////////////////////////////////

	bool LegacyFluxMeasurementComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch resource
		mResource = getComponent<LegacyFluxMeasurementComponent>();

		// Ensure FFTAudioComponentInstance is available
		mFFTAudioComponent = &getEntityInstance()->getComponent<FFTAudioNodeComponentInstance>();
		if (!errorState.check(mFFTAudioComponent != nullptr, "Missing nap::FFTAudioComponentInstance under entity"))
			return false;

		const uint bin_count = mFFTAudioComponent->getFFTBuffer().getBinCount();
		mOnsetList.reserve(mResource->mParameters.size());
		for (auto& entry : mResource->mParameters)
		{
			if (!errorState.check(entry->mMinHz < entry->mMaxHz, "%s: Invalid filter parameter item. Minimum hertz higher than maximum hertz.", mResource->mID.c_str()))
				return false;

			mOnsetList.emplace_back(*entry);
		}

		mPreviousBuffer.resize(bin_count);
		return true;
	}


	void LegacyFluxMeasurementComponentInstance::update(double deltaTime)
	{
		if (!mResource->mEnable)
			return;

		const float delta_time = static_cast<float>(deltaTime);
		mElapsedTime += delta_time;

		// Fetch amplitudes
		const auto& amps = mFFTAudioComponent->getFFTBuffer().getAmplitudeSpectrum();

		for (auto& entry : mOnsetList)
		{
			const float interval = utility::interval(mFFTAudioComponent->getFFTBuffer().getBinCount()-1, mFFTAudioComponent->getSampleRate());
			const uint min_bin = static_cast<uint>(entry.mMinHz / interval);
			const uint max_bin = static_cast<uint>(entry.mMaxHz / interval);

			float flux = utility::flux(amps, mPreviousBuffer, min_bin, max_bin);
			float mult = (entry.mMultiplier != nullptr) ? entry.mMultiplier->mValue : 1.0f;
			float raw_onset = flux * mult;
			float previous_onset = entry.mOnsetValue;
			
			// Acceleration
			if (raw_onset > previous_onset)
			{
				// Compute upwards force on acceleration proportionate to the difference in onset
				float diff = std::abs(raw_onset - previous_onset);
				entry.mAcceleration = (1.0f - std::pow(diff - 1.0f, 2.0f)) * entry.mOnsetImpact;
				entry.mVelocity = 0.0f;
			}
			else
			{
				float decay = (entry.mDecay != nullptr) ? entry.mDecay->mValue : 0.1f;
				entry.mAcceleration -= decay * delta_time * 1000.0f;
			}
			entry.mVelocity = std::max(entry.mVelocity + entry.mAcceleration * delta_time, -1000.0f);
			float max_onset = std::max(raw_onset, previous_onset);
			float onset = std::max(max_onset + entry.mVelocity * delta_time, 0.0f);

			// Compute stretch factor to normalize output to target average over a time period
			float stretch = 1.0f;
			if (entry.mStretch != nullptr)
			{
				float average_onset = std::max(entry.computeMovingAverage(onset, entry.mSampleAverage), glm::epsilon<float>()*2.0f);
				float target_onset = (entry.mTargetOnset != nullptr) ? entry.mTargetOnset->mValue : 0.25f;
				float factor = target_onset / average_onset;
				entry.mStretch->setValue(entry.mStretchSmoother.update(factor, delta_time));
				stretch = entry.mStretch->mValue;
			}

			entry.mOnsetValue = onset;
			float smooth_onset = entry.mOnsetSmoother.update(entry.mOnsetValue, delta_time);
			float stretch_onset = smooth_onset * stretch;
			float offset = (entry.mOffset != nullptr) ? entry.mOffset->mValue : 0.0f;
			entry.mParameter.setValue(stretch_onset + offset);
		}

		// Copy
		mPreviousBuffer = amps;
	}


	//////////////////////////////////////////////////////////////////////////
	// LegacyFluxMeasurementComponentInstance::OnsetData
	//////////////////////////////////////////////////////////////////////////

	float LegacyFluxMeasurementComponentInstance::OnsetData::computeMovingAverage(float value, float& outAverage)
	{
		if (mSamplesEvaluated < mEvaluationSampleCount)
		{
			float result = mSamplesEvaluated * outAverage + value;
			outAverage = result / (mSamplesEvaluated + 1.0f);
			++mSamplesEvaluated;
		}
		else
		{
			float mult = 2.0f / (mEvaluationSampleCount + 1.0f);
			outAverage = (value - outAverage) * mult + outAverage;
		}
		return outAverage;
	}
}
