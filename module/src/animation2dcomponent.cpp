#include "animation2dcomponent.h"

// External Includes
#include <entity.h>
#include <materialinstance.h>

// nap::Animation2DComponent run time class definition
RTTI_BEGIN_CLASS(nap::Animation2DComponent)
	RTTI_PROPERTY("Frames",   			&nap::Animation2DComponent::mFrames,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FramesPerSecond",	&nap::Animation2DComponent::mFramesPerSecond,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::Animation2DComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Animation2DComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool Animation2DComponentInstance::init(utility::ErrorState& errorState)
    {
        auto* resource = getComponent<Animation2DComponent>();

		if (!errorState.check(!resource->mFrames->mMembers.empty(), "Missing frames"))
            return false;

		mFrames.reserve(resource->mFrames->mMembers.size());
		for (auto& frame : resource->mFrames->mMembers)
			mFrames.emplace_back(frame.get());

		mFrameInterval = 1.0 / math::max(resource->mFramesPerSecond, math::epsilon<double>());

		return true;
	}


	void Animation2DComponentInstance::update(double deltaTime)
	{
		mElapsedTime += deltaTime;
		mFrameIndex = static_cast<uint>(math::floor(mElapsedTime/mFrameInterval))%mFrames.size();
	}
}
