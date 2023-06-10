/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "perpwarplinecomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>
#include <meshutils.h>
#include <renderglobals.h>
#include <opencv2/opencv.hpp>

RTTI_BEGIN_CLASS(nap::PerpWarpLineComponent)
        RTTI_PROPERTY("Source Line", &nap::PerpWarpLineComponent::mSourceLine, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("Target Line", &nap::PerpWarpLineComponent::mTargetLine, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("Top Left", &nap::PerpWarpLineComponent::mTopLeft, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Top Right", &nap::PerpWarpLineComponent::mTopRight, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Bottom Right", &nap::PerpWarpLineComponent::mBottomRight, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Bottom Left", &nap::PerpWarpLineComponent::mBottomLeft, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Update Mesh Instance", &nap::PerpWarpLineComponent::mUpdateMeshInstance, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PerpWarpLineComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    bool PerpWarpLineComponentInstance::init(utility::ErrorState& errorState)
    {
        auto* resource = getComponent<PerpWarpLineComponent>();
        mTargetLine = resource->mTargetLine.get();
        mSourceLine = resource->mSourceLine.get();

        mTopLeft = resource->mTopLeft;
        mTopRight = resource->mTopRight;
        mBottomRight = resource->mBottomRight;
        mBottomLeft = resource->mBottomLeft;

        mUpdateMeshInstance = resource->mUpdateMeshInstance;

        return true;
    }


    void PerpWarpLineComponentInstance::update(double deltaTime)
    {
        const auto& src_pos_data = mSourceLine->getMeshInstance().getAttribute<glm::vec3>(vertexid::position).getData();
        const auto& src_col_data = mSourceLine->getMeshInstance().getAttribute<glm::vec4>(vertexid::color).getData();

        auto& target_pos_data = mTargetLine->getMeshInstance().getAttribute<glm::vec3>(vertexid::position).getData();
        auto& target_nor_data = mTargetLine->getMeshInstance().getAttribute<glm::vec3>(vertexid::normal).getData();
        auto& target_col_data = mTargetLine->getMeshInstance().getAttribute<glm::vec4>(vertexid::color).getData();

        assert(src_pos_data.size() == target_pos_data.size());

        std::vector<cv::Point2f> src_points;
        for(int i = 0 ; i < src_pos_data.size(); i++)
        {
            src_points.emplace_back(cv::Point2f(src_pos_data[i].x, src_pos_data[i].y));
        }

        // start with square/flat projection ( source )
        static cv::Point2f src_p[4] =
        {
            cv::Point2f(-0.5f, 0.5f),
            cv::Point2f(0.5f, 0.5f),
            cv::Point2f(0.5f, -0.5f),
            cv::Point2f(-0.5f, -0.5f)
        };

        // translated points of projection
        const cv::Point2f dst_p[4] =
        {
            cv::Point2f(mTopLeft.x, mTopLeft.y),
            cv::Point2f(mTopRight.x, mTopRight.y),
            cv::Point2f(mBottomRight.x, mBottomRight.y),
            cv::Point2f(mBottomLeft.x, mBottomLeft.y)
        };

        // create perspective transform matrix
        cv::Mat trans_mat33 = cv::getPerspectiveTransform(src_p, dst_p); //CV_64F->double

        // vector of translated points
        std::vector<cv::Point2f> dst_points;

        // perspective transform operation using transform matrix
        cv::perspectiveTransform(src_points, dst_points, trans_mat33);

        for(int i = 0 ; i < target_pos_data.size(); i++)
        {
            glm::vec3 warped_point = glm::vec3(dst_points[i].x, dst_points[i].y, 0.0f);
            warped_point.x = math::clamp(warped_point.x, -0.5f, 0.5f);
            warped_point.y = math::clamp(warped_point.y, -0.5f, 0.5f);
            target_pos_data[i] = warped_point;
            target_col_data[i] = src_col_data[i];
        }

        if(mUpdateMeshInstance)
        {
            utility::ErrorState error_state;
            if(!mTargetLine->getMeshInstance().update(error_state))
            {
                nap::Logger::warn(*this, error_state.toString());
            }
        }
    }


    void PerpWarpLineComponentInstance::onDestroy()
    {
    }


    void PerpWarpLineComponentInstance::updateNormals(std::vector<glm::vec3>& normals, const std::vector<glm::vec3>& vertices)
    {
        glm::vec3 crossn(0.0f, 0.0f, -1.0f);
        for (int i = 1; i < vertices.size() - 1; i++)
        {
            // Get vector pointing to next and previous vertex
            glm::vec3 dnormal_one = glm::normalize(vertices[i + 1] - vertices[i]);
            glm::vec3 dnormal_two = glm::normalize(vertices[i] - vertices[i - 1]);

            // Rotate around z using cross product
            normals[i] = glm::cross(glm::normalize(math::lerp<glm::vec3>(dnormal_one, dnormal_two, 0.5f)), crossn);
        }

        // Fix beginning and end
        normals[0] = glm::cross(glm::normalize(vertices[1] - vertices.front()), crossn);
        normals.back() = normals[normals.size() - 2];
    }
}
