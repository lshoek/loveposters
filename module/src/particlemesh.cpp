/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "particlemesh.h"
#include "renderservice.h"

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/numeric.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParticleMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::ParticleMesh::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::ParticleMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::ParticleMesh::mPolygonMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Count",			&nap::ParticleMesh::mCount,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	ParticleMesh::ParticleMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool ParticleMesh::init(utility::ErrorState& errorState)
	{
		assert(mRenderService != nullptr);
		if (!errorState.check(mCount > 0, "Invalid Count"))
			return false;

		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);
		mMeshInstance->setNumVertices(mCount);
		mMeshInstance->setUsage(EMemoryUsage::Static);
		mMeshInstance->setDrawMode(EDrawMode::Points);
		mMeshInstance->setCullMode(ECullMode::Back);

		return mMeshInstance->init(errorState);
	}
}
