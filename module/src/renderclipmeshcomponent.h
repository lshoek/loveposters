/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>


namespace nap
{
	class RenderClipMeshComponentInstance;
	
	/**
	 * Resource part of the renderable mesh component. Renders a mesh to screen or any other render target.
	 * The link to the mesh and clipping rectangle (property) are optional. You can set the mesh at runtime if necessary.
	 * The material is required. 
	 *
	 * A mesh becomes 'renderable' when it is used in combination with a material. Such a mesh-material combination forms a 'nap::RenderableMesh'. 
	 * Vertex attributes in both the shader and mesh should match. Otherwise, the RenderableMesh is invalid. 
	 * The nap::RenderableMesh is created based on the given mesh and the material properties.
	 *
	 * It is, however, possible to switch the mesh and / or material from the RenderClipMeshComponent to some other mesh and / or material. 
	 * To do so, other components should create their own nap::RenderableMesh by calling nap::RenderService::createRenderableMesh, 
	 * and pass the returned object to setMesh(). The object calling nap::RenderService::createRenderableMesh should own the custom mesh and / or material 
	 * and should validate the result on init().
	 *
	 * The model view and projection matrices are automatically set when the vertex shader exposes a struct with the 'uniform::mvpStruct' name.
	 * If exposed by the shader it is required that the model, view and projection matrix names match those as declared in 'renderglobals.h'.
	 *
	 * A Transform component is required to position the mesh.
	 */
	class NAPAPI RenderClipMeshComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(RenderClipMeshComponent, RenderClipMeshComponentInstance)
	public:
		MaterialInstanceResource			mShadowMaterialInstanceResource;			///< Property: 'ShadowMaterialInstance' instance of the shadow material, used to override uniforms for this instance
	};


	/**
	 * Instance part of the renderable mesh component. Renders a mesh to screen or any other render target.
	 * You can set the mesh at runtime if necessary.
	 *
	 * A mesh becomes 'renderable' when it is used in combination with a material. Such a mesh-material combination forms a 'nap::RenderableMesh'.
	 * Vertex attributes in both the shader and mesh should match. Otherwise, the RenderableMesh is invalid.
	 * The nap::RenderableMesh is created based on the given mesh and the material properties.
	 *
	 * It is, however, possible to switch the mesh and / or material from the RenderClipMeshComponent to some other mesh and / or material.
	 * To do so, other components should create their own nap::RenderableMesh by calling nap::RenderService::createRenderableMesh,
	 * and pass the returned object to setMesh(). The object calling nap::RenderService::createRenderableMesh should own the custom mesh and / or material
	 * and should validate the result on init().
	 *
	 * The model view and projection matrices are automatically updated when the vertex shader exposes a struct with the 'uniform::mvpStruct' name.
	 *
	 * ~~~~~{.cpp}
	 * uniform nap
	 * {
	 *		mat4 projectionMatrix;		///< Optional
	 *		mat4 viewMatrix;			///< Optional
	 *		mat4 modelMatrix;			///< Optional
	 *	} mvp;							///< Optional
	 * ~~~~~
	 *
	 * A Transform component is required to position the mesh.
	 */
	class NAPAPI RenderClipMeshComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)

	public:
		RenderClipMeshComponentInstance(EntityInstance& entity, Component& component);

		virtual bool init(utility::ErrorState& errorState) override;

		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		MaterialInstance						mShadowMaterialInstance;					///< 

		UniformMat4Instance*					mShadowModelMatUniform = nullptr;			///< Pointer to the model matrix uniform
		UniformMat4Instance*					mShadowViewMatUniform = nullptr;			///< Pointer to the view matrix uniform
		UniformMat4Instance*					mShadowProjectMatUniform = nullptr;			///< Pointer to the projection matrix uniform
	};
}
