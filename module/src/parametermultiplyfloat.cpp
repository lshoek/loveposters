/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "parametermultiplyfloat.h"

RTTI_BEGIN_CLASS(nap::ParameterMultiplyFloat)
	RTTI_PROPERTY("InputParameter", &nap::ParameterMultiplyFloat::mInputParameter, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Multiply", &nap::ParameterMultiplyFloat::mMultiply, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool ParameterMultiplyFloat::init(utility::ErrorState& errorState)
	{
		mInputParameter->valueChanged.connect(mInputParameterChangedSlot);
		return true;
	}


	void ParameterMultiplyFloat::onInputParameterChanged(float value)
	{
		setValue(value * mMultiply);
	}


	void ParameterMultiplyFloat::setValue(const Parameter& value)
	{
		const ParameterMultiplyFloat* derived_type = rtti_cast<const ParameterMultiplyFloat>(&value);
		assert(derived_type != nullptr);
		setValue(derived_type->mValue);
	}


	void ParameterMultiplyFloat::setValue(float value)
	{
		mValue = value;
		valueChanged(mValue);
	}
}
