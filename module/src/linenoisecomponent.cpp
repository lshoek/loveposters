/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "linenoisecomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/noise.hpp>
#include <nap/logger.h>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>

RTTI_BEGIN_STRUCT(nap::NoiseProperties)
	RTTI_PROPERTY("Frequency",			&nap::NoiseProperties::mFrequency,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LinePosFrequency",	&nap::NoiseProperties::mLinePosFrequency,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Speed",				&nap::NoiseProperties::mSpeed,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Offset",				&nap::NoiseProperties::mOffset,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Amplitude",			&nap::NoiseProperties::mAmplitude,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Shift",				&nap::NoiseProperties::mShift,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SmoothTime",			&nap::NoiseProperties::mSmoothTime,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::LineNoiseComponent)
	RTTI_PROPERTY("Properties",			&nap::LineNoiseComponent::mProperties,		nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("LineBlender",		&nap::LineNoiseComponent::mBlendComponent,	nap::rtti::EPropertyMetaData::Required)

	RTTI_PROPERTY("LowInput",			&nap::LineNoiseComponent::mLowLevel,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MidInput",			&nap::LineNoiseComponent::mMidLevel,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("HighInput",			&nap::LineNoiseComponent::mHighLevel,		nap::rtti::EPropertyMetaData::Required)

	RTTI_PROPERTY("LowInputIntensity", &nap::LineNoiseComponent::mLowLevelIntensity, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MidInputIntensity", &nap::LineNoiseComponent::mMidLevelIntensity, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("HighInputIntensity", &nap::LineNoiseComponent::mHighLevelIntensity, nap::rtti::EPropertyMetaData::Required)

	RTTI_PROPERTY("LowInputAccumulation",	&nap::LineNoiseComponent::mLowLevelAccumulation,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MidInputAccumulation",	&nap::LineNoiseComponent::mMidLevelAccumulation,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("HighInputAccumulation",	&nap::LineNoiseComponent::mHighLevelAccumulation,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineNoiseComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineNoiseComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over properties and link to blend component
		mResource = getComponent<LineNoiseComponent>();
		mProperties = mResource->mProperties;

		// Set smooth timing values
		mAmpSmoother.mSmoothTime = mProperties.mSmoothTime;
		mAmpSmoother.setValue(mProperties.mAmplitude->mValue);

		mFreqSmoother.mSmoothTime = mProperties.mSmoothTime;
		mFreqSmoother.setValue(mProperties.mFrequency->mValue);

		mLinePosFreqSmoother.mSmoothTime = mProperties.mSmoothTime;
		mLinePosFreqSmoother.setValue(mProperties.mLinePosFrequency->mValue);

		mOffsetSmoother.mSmoothTime = mProperties.mSmoothTime;
		mOffsetSmoother.setValue(mProperties.mOffset->mValue);

		mSpeedSmoother.mSmoothTime = mProperties.mSmoothTime;
		mSpeedSmoother.setValue(mProperties.mSpeed->mValue);

		mShiftSmoother.mSmoothTime = mProperties.mSmoothTime;
		mShiftSmoother.setValue(mProperties.mShift->mValue);

		mRandomSeed = { glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f), glm::linearRand<float>(0.0f, 1000.0f) };

		return true;
	}


	void LineNoiseComponentInstance::update(double deltaTime)
	{
		// Update accumulators
		float delta_time = static_cast<float>(deltaTime);

		float low = mResource->mLowLevel->mValue * mResource->mLowLevelIntensity->mValue;
		float mid = mResource->mMidLevel->mValue * mResource->mMidLevelIntensity->mValue;
		float high = mResource->mHighLevel->mValue * mResource->mHighLevelIntensity->mValue;

		mLowLevelAccumulator += mResource->mLowLevel->mValue * mResource->mLowLevelAccumulation->mValue * delta_time;
		mMidLevelAccumulator += mResource->mMidLevel->mValue * mResource->mMidLevelAccumulation->mValue * delta_time;
		mHighLevelAccumulator += mResource->mHighLevel->mValue * mResource->mHighLevelAccumulation->mValue * delta_time;

		low += glm::simplex(glm::vec2(mLowLevelAccumulator, mRandomSeed.x)) * mResource->mLowLevelAccumulation->mValue;
		mid += glm::simplex(glm::vec2(mMidLevelAccumulator, mRandomSeed.y)) * mResource->mMidLevelAccumulation->mValue;
		high += glm::simplex(glm::vec2(mHighLevelAccumulator, mRandomSeed.z)) * mResource->mHighLevelAccumulation->mValue;

		// Update smoothers
		mSpeedSmoother.update(mProperties.mSpeed->mValue, deltaTime);
		mFreqSmoother.update(mProperties.mFrequency->mValue, deltaTime);
		mLinePosFreqSmoother.update(mProperties.mLinePosFrequency->mValue, deltaTime);
		mAmpSmoother.update(mProperties.mAmplitude->mValue, deltaTime);
		mOffsetSmoother.update(mProperties.mOffset->mValue, deltaTime);
		mShiftSmoother.update(mProperties.mShift->mValue, deltaTime);

		// Update current time
		mCurrentTime += (deltaTime * mSpeedSmoother.getValue());

		float offset = mCurrentTime + mOffsetSmoother.getValue() + mid;

		glm::vec2 shift = glm::normalize(glm::mix(glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f), mLinePosFreqSmoother.getValue())) * mShiftSmoother.getValue();
		shift.x += high;

		// Apply noise based on normal
		nap::PolyLine& line = mBlendComponent->getLine();
		int vert_count = line.getMeshInstance().getNumVertices();

		// Get the normals and vertices to manipulate
		std::vector<glm::vec3>& normals = line.getNormalAttr().getData();
		std::vector<glm::vec3>& vertices = line.getPositionAttr().getData();
		std::vector<glm::vec3>& uvs = line.getUvAttr().getData();

		for (int i = 0; i < vert_count; i++)
		{
			float pos = i / static_cast<float>(vert_count) - 0.5f;
			glm::vec2 uv_sample = glm::mix(glm::vec2(uvs[i].x, uvs[i].y), glm::vec2(pos, 0.0f), mLinePosFreqSmoother.getValue());

			float freq = mFreqSmoother.getValue() + low;
			glm::vec2 freq_shift = uv_sample * freq;
			
			auto uv = freq_shift + shift + offset;
			float v = glm::simplex(uv) * mAmpSmoother.getValue();
			vertices[i] += (normals[i] * v);
		}

		// Update normal based on displaced vertices
		updateNormals(normals, vertices);

		// Push changes to the gpu
		utility::ErrorState error;
		if (!line.getMeshInstance().update(error))
		{
			nap::Logger::warn(error.toString().c_str());
		}
	}


	void LineNoiseComponentInstance::updateNormals(std::vector<glm::vec3>& normals, const std::vector<glm::vec3>& vertices)
	{
		glm::vec3 crossn(0.0f, 0.0f, -1.0f);
		for (int i = 1; i < vertices.size() - 1; i++)
		{
			// Get vector pointing to next and previous vertex
			glm::vec3 dnormal_one = glm::normalize(vertices[i + 1] - vertices[i]);
			glm::vec3 dnormal_two = glm::normalize(vertices[i] - vertices[i - 1]);

			// Rotate around z using cross product
			normals[i] = glm::cross(glm::normalize(math::lerp<glm::vec3>(dnormal_one, dnormal_two, 0.5f)), crossn);
		}
	
		// Fix beginning and end
		normals[0] = glm::cross(glm::normalize(vertices[1] - vertices.front()), crossn);
		normals.back() = normals[normals.size() - 2];
	}
}
