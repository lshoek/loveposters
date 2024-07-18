/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <parameternumeric.h>

#include "homographycomponent.h"

namespace nap
{
    // Forward Declares
    class UpdateHomographyComponentInstance;
	class HomographyComponentInstance;

	/**
	 * UpdateHomographyComponent
	 */
    class NAPAPI UpdateHomographyComponent : public Component
    {
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateHomographyComponent, UpdateHomographyComponentInstance)
    public:
		ResourcePtr<ParameterFloat> mTopLeftX;
		ResourcePtr<ParameterFloat> mTopLeftY;

		ResourcePtr<ParameterFloat> mTopRightX;
		ResourcePtr<ParameterFloat> mTopRightY;

		ResourcePtr<ParameterFloat> mBottomRightX;
		ResourcePtr<ParameterFloat> mBottomRightY;

		ResourcePtr<ParameterFloat> mBottomLeftX;
		ResourcePtr<ParameterFloat> mBottomLeftY;

		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};

	/**
	 * UpdateHomographyComponentInstance
	 */
    class NAPAPI UpdateHomographyComponentInstance : public ComponentInstance
    {
		RTTI_ENABLE(ComponentInstance)
    public:
        UpdateHomographyComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

        virtual bool init(utility::ErrorState& errorState) override;

        virtual void update(double deltaTime) override;

		UpdateHomographyComponent* mResource = nullptr;
		HomographyComponentInstance* mHomographyComponent = nullptr;
	};
}
