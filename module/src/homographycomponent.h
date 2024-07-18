/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <glm/glm.hpp>
#include <texture.h>

namespace nap
{
    // Forward Declares
    class HomographyComponentInstance;

	/**
	 * HomographyComponent
	 */
    class NAPAPI HomographyComponent : public Component
    {
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(HomographyComponent, HomographyComponentInstance)
    public:
		glm::vec2 mTopLeft		= { 0.0f, 1080.0f };
		glm::vec2 mTopRight		= { 1920.0f, 1080.0f };
		glm::vec2 mBottomRight	= { 1920.0f, 0.0f };
		glm::vec2 mBottomLeft	= { 0.0f, 0.0f };

		float mSourceWidth		= 1920.0f;
		float mSourceHeight		= 1080.0f;
		ResourcePtr<Texture2D> mReferenceTexture;
    };

	/**
	 * HomographyComponentInstance
	 */
    class NAPAPI HomographyComponentInstance : public ComponentInstance
    {
		RTTI_ENABLE(ComponentInstance)
    public:
        HomographyComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

        virtual bool init(utility::ErrorState& errorState) override;

        virtual void update(double deltaTime) override;

        void setTopLeft(const glm::vec2& topLeft)				{ mTopLeft = topLeft; }
        glm::vec2 getTopLeft() const							{ return mTopLeft; }

        void setTopRight(const glm::vec2& topRight)				{ mTopRight = topRight; }
        glm::vec2 getTopRight() const							{ return mTopRight; }

        void setBottomRight(const glm::vec2& bottomRight)		{ mBottomRight = bottomRight; }
        glm::vec2 getBottomRight() const						{ return mBottomRight; }

        void setBottomLeft(const glm::vec2& bottomLeft)			{ mBottomLeft = bottomLeft; }
        glm::vec2 getBottomLeft() const							{ return mBottomLeft; }

		const glm::mat3& getHomographyMatrix() const			{ return mHomographyMatrix; }
		const glm::mat3& getInverseHomographyMatrix() const		{ return mInverseHomographyMatrix; }

    private:
		glm::vec2 mTopLeft;
		glm::vec2 mTopRight;
		glm::vec2 mBottomRight;
		glm::vec2 mBottomLeft;

		glm::vec2 mSourceSize;

		glm::mat3 mHomographyMatrix;
		glm::mat3 mInverseHomographyMatrix;
	};
}
