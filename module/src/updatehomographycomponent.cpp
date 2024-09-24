/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "updatehomographycomponent.h"

// External Includes
#include <entity.h>
#include <rtti/typeinfo.h>
#include <meshutils.h>
#include <renderglobals.h>
#include <opencv2/opencv.hpp>
#include <glm/gtc/type_ptr.hpp>

RTTI_BEGIN_CLASS(nap::UpdateHomographyComponent)
	RTTI_PROPERTY("TopLeftX",		&nap::UpdateHomographyComponent::mTopLeftX,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TopLeftY",		&nap::UpdateHomographyComponent::mTopLeftY,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TopRightX",		&nap::UpdateHomographyComponent::mTopRightX,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TopRightY",		&nap::UpdateHomographyComponent::mTopRightY,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BottomLeftX",	&nap::UpdateHomographyComponent::mBottomLeftX,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BottomLeftY",	&nap::UpdateHomographyComponent::mBottomLeftY,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BottomRightX",	&nap::UpdateHomographyComponent::mBottomRightX, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BottomRightY",	&nap::UpdateHomographyComponent::mBottomRightY, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateHomographyComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UpdateHomographyComponent
	//////////////////////////////////////////////////////////////////////////

	void UpdateHomographyComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(HomographyComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// UpdateHomographyComponentInstance
	//////////////////////////////////////////////////////////////////////////

    bool UpdateHomographyComponentInstance::init(utility::ErrorState& errorState)
    {
		mResource = getComponent<UpdateHomographyComponent>();

		// Ensure there is a transform component
		mHomographyComponent = getEntityInstance()->findComponent<HomographyComponentInstance>();
		if (!errorState.check(mHomographyComponent != nullptr, "%s: missing homography component", mID.c_str()))
			return false;

        return true;
    }


    void UpdateHomographyComponentInstance::update(double deltaTime)
    {
		mHomographyComponent->setTopLeft({ mResource->mTopLeftX->mValue, mResource->mTopLeftY->mValue });
		mHomographyComponent->setTopRight({ mResource->mTopRightX->mValue, mResource->mTopRightY->mValue });
		mHomographyComponent->setBottomLeft({ mResource->mBottomLeftX->mValue, mResource->mBottomLeftY->mValue });
		mHomographyComponent->setBottomRight({ mResource->mBottomRightX->mValue, mResource->mBottomRightY->mValue });
    }
}
