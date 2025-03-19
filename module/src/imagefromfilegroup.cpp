/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "imagefromfilegroup.h"

// For backwards compatibility reasons, override the the default 'Members' and 'Children' property names
// of the 'nap::ImageFromFileGroup' to the property names introduced before the arrival of the generic nap::Group<T>.
RTTI_BEGIN_CLASS(nap::ImageFromFileGroup, "Groups together a set of 'nap::ImageFromFile' objects")
	RTTI_PROPERTY(nap::group::parameter::members,	&nap::ImageFromFileGroup::mMembers,		nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)
	RTTI_PROPERTY(nap::group::parameter::children,	&nap::ImageFromFileGroup::mChildren,	nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)
RTTI_END_CLASS

namespace nap
{
	template<> 
	nap::Group<ImageFromFile>::Group() : IGroup(RTTI_OF(ImageFromFile),
		group::parameter::members, group::parameter::children) { }
}
