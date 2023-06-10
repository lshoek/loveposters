/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <uniforminstance.h>
#include <lineutils.h>
#include <polyline.h>

#include "perpwarplinecomponent.h"

namespace nap
{
    // Forward Declares
    class NAPAPI RenderWarpLineComponentInstance;

    class RenderWarpLineComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(RenderWarpLineComponent, RenderWarpLineComponentInstance)
    public:
        nap::ResourcePtr<Line> mLine = nullptr;
        nap::ComponentPtr<PerpWarpLineComponent> mPerpWarpLineComponent;
    };

    class NAPAPI RenderWarpLineComponentInstance : public ComponentInstance
    {
    RTTI_ENABLE(ComponentInstance)
    public:
        RenderWarpLineComponentInstance(EntityInstance& entity, Component& resource) :
                ComponentInstance(entity, resource) { }

        virtual bool init(utility::ErrorState& errorState) override;

        virtual void update(double deltaTime) override;

        virtual void onDestroy() override;

        nap::ComponentInstancePtr<PerpWarpLineComponent> mPerpWarpLineComponent = { this, &RenderWarpLineComponent::mPerpWarpLineComponent };
    private:
        void updateMeshInstance();

        Line* mLine;
        bool mUpdateMesh = true;
        glm::vec2 mTopLeft;
        glm::vec2 mTopRight;
        glm::vec2 mBottomRight;
        glm::vec2 mBottomLeft;
    };
}
