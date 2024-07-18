/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>
#include <renderablemesh.h>
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <uniforminstance.h>
#include <planemesh.h>

#include "homographycomponent.h"

namespace nap
{
    // Forward Declares
    class RenderHomographyComponentInstance;
	class HomographyComponentInstance;
	class RenderService;
	class Core;

	/**
	 * RenderHomographyComponent
	 */
    class NAPAPI RenderHomographyComponent : public RenderableMeshComponent
    {
        RTTI_ENABLE(RenderableMeshComponent)
        DECLARE_COMPONENT(RenderHomographyComponent, RenderHomographyComponentInstance)
	public:
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};

	/**
	 * RenderHomographyComponentInstance
	 */
    class NAPAPI RenderHomographyComponentInstance : public RenderableMeshComponentInstance
    {
		RTTI_ENABLE(RenderableComponentInstance)
    public:
		RenderHomographyComponentInstance(EntityInstance& entity, Component& resource);

        virtual bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Draws the current camera frustrum
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

    private:
		RenderHomographyComponent* mResource = nullptr;
		HomographyComponentInstance* mHomographyComponent = nullptr;

		// Uniforms
		UniformStructInstance* mUBOStruct = nullptr;
		UniformMat4Instance* mHomographyMatrixUniform = nullptr;
    };
}
