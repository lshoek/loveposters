/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <uniforminstance.h>
#include <lineutils.h>
#include <renderablemeshcomponent.h>
#include <polyline.h>

namespace nap
{
    // Forward Declares
    class NAPAPI PerpWarpLineComponentInstance;

    class PerpWarpLineComponent : public Component
    {
    RTTI_ENABLE(Component)
    DECLARE_COMPONENT(PerpWarpLineComponent, PerpWarpLineComponentInstance)
    public:
        nap::ResourcePtr<Line> mTargetLine = nullptr;
        nap::ResourcePtr<Line> mSourceLine = nullptr;

        glm::vec2 mTopLeft = glm::vec2(-0.5f, 0.5f);
        glm::vec2 mTopRight = glm::vec2(0.5f, 0.5f);
        glm::vec2 mBottomRight = glm::vec2(0.5f, -0.5f);
        glm::vec2 mBottomLeft = glm::vec2(-0.5f, -0.5f);

        bool mUpdateMeshInstance = false;
    };

    class NAPAPI PerpWarpLineComponentInstance : public ComponentInstance
    {
    RTTI_ENABLE(ComponentInstance)
    public:
        PerpWarpLineComponentInstance(EntityInstance& entity, Component& resource) :
                ComponentInstance(entity, resource) { }

        virtual bool init(utility::ErrorState& errorState) override;

        virtual void update(double deltaTime) override;

        virtual void onDestroy() override;

        void setTopLeft(const glm::vec2& topLeft){ mTopLeft = topLeft; }
        glm::vec2 getTopLeft() const{ return mTopLeft; }

        void setTopRight(const glm::vec2& topRight){ mTopRight = topRight; }
        glm::vec2 getTopRight() const{ return mTopRight; }

        void setBottomRight(const glm::vec2& bottomRight){ mBottomRight = bottomRight; }
        glm::vec2 getBottomRight() const{ return mBottomRight; }

        void setBottomLeft(const glm::vec2& bottomLeft){ mBottomLeft = bottomLeft; }
        glm::vec2 getBottomLeft() const{ return mBottomLeft; }
    private:
        void updateNormals(std::vector<glm::vec3>& normals, const std::vector<glm::vec3>& vertices);

        Line* mTargetLine;
        Line* mSourceLine;

        glm::vec2 mTopLeft = glm::vec2(-0.5f, 0.5f);
        glm::vec2 mTopRight = glm::vec2(0.5f, 0.5f);
        glm::vec2 mBottomRight = glm::vec2(0.5f, -0.5f);
        glm::vec2 mBottomLeft = glm::vec2(-0.5f, -0.5f);

        bool mUpdateMeshInstance = false;
    };
}
