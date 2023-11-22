/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mathutils.h"

// External Includes
#include <parameternumeric.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * ParameterMultiplyFloat
	 */
	class ParameterMultiplyFloat : public ParameterFloat
	{
		RTTI_ENABLE(ParameterFloat)
	public:
		ResourcePtr<ParameterFloat> mInputParameter;
		float mMultiply = 1.0f;

		/**
		 * 
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * 
		 */
		virtual void setValue(const Parameter& value) override;

		/**
		 * 
		 */
		virtual void setValue(float value) override;

	private:
		void onInputParameterChanged(float value);

		nap::Slot<float> mInputParameterChangedSlot = { this, &ParameterMultiplyFloat::onInputParameterChanged };
	};
}
