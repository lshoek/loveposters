/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "renderwarplinecomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>
#include <meshutils.h>
#include <renderglobals.h>

RTTI_BEGIN_CLASS(nap::RenderWarpLineComponent)
        RTTI_PROPERTY("Warp Line Mesh", &nap::RenderWarpLineComponent::mLine, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("Perp Warp Line Component", &nap::RenderWarpLineComponent::mPerpWarpLineComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderWarpLineComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    bool RenderWarpLineComponentInstance::init(utility::ErrorState& errorState)
    {
        mLine = getComponent<RenderWarpLineComponent>()->mLine.get();

        if(mLine->getMeshInstance().getAttribute<glm::vec3>(vertexid::position).getData().size() < 4)
        {
            errorState.fail("Line has less then 4 vertices!");
            return false;
        }

        auto top_left       = mPerpWarpLineComponent->getTopLeft();
        auto top_right      = mPerpWarpLineComponent->getTopRight();
        auto bottom_right   = mPerpWarpLineComponent->getBottomRight();
        auto bottom_left    = mPerpWarpLineComponent->getBottomLeft();

        mTopLeft        = top_left;
        mTopRight       = top_right;
        mBottomRight    = bottom_right;
        mBottomLeft     = bottom_left;

        updateMeshInstance();

        return true;
    }


    void RenderWarpLineComponentInstance::update(double deltaTime)
    {
        auto top_left       = mPerpWarpLineComponent->getTopLeft();
        auto top_right      = mPerpWarpLineComponent->getTopRight();
        auto bottom_right   = mPerpWarpLineComponent->getBottomRight();
        auto bottom_left    = mPerpWarpLineComponent->getBottomLeft();

        bool update_mesh = top_left != mTopLeft || top_right != mTopRight || bottom_right != mBottomRight || bottom_left != mBottomLeft;

        if(update_mesh)
        {
			mTopLeft = top_left;
			mTopRight = top_right;
			mBottomRight = bottom_right;
			mBottomLeft = bottom_left;

            updateMeshInstance();
        }
    }


    void RenderWarpLineComponentInstance::onDestroy()
    {
    }


    void RenderWarpLineComponentInstance::updateMeshInstance()
    {
        auto& pos_data = mLine->getMeshInstance().getAttribute<glm::vec3>(vertexid::position).getData();

        assert(pos_data.size() >= 4);

        pos_data[0] = glm::vec3(mTopLeft.x,     mTopLeft.y, 0);
        pos_data[1] = glm::vec3(mTopRight.x,    mTopRight.y, 0);
        pos_data[2] = glm::vec3(mBottomRight.x, mBottomRight.y, 0);
        pos_data[3] = glm::vec3(mBottomLeft.x,  mBottomLeft.y, 0);

        utility::ErrorState error_state;
        if(!mLine->getMeshInstance().update(error_state))
        {
            nap::Logger::error(*this, error_state.toString());
        }
    }
}
