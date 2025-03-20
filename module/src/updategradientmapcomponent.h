/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

#include <component.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametersimple.h>
#include <parametercolor.h>
#include <uniforminstance.h>
#include <rendercomponent.h>
#include <componentptr.h>

namespace nap
{
	class UpdateGradientMapComponentInstance;
	class RenderableMeshComponentInstance;
	class UniformStructInstance;

	/**
	 *	UpdateGradientMapComponent
	 */
	class NAPAPI UpdateGradientMapComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateGradientMapComponent, UpdateGradientMapComponentInstance)
	public:
		ComponentPtr<RenderableComponent> mRenderer;

		ResourcePtr<ParameterFloat> mClockSpeed;
		ResourcePtr<ParameterRGBColorFloat> mColor0;
		ResourcePtr<ParameterRGBColorFloat> mColor1;
		ResourcePtr<ParameterRGBColorFloat> mColor2;
		ResourcePtr<ParameterRGBColorFloat> mColor3;
		ResourcePtr<ParameterRGBColorFloat> mColor4;
	};


	/**
	 * UpdateGradientMapComponentInstance	
	 */
	class NAPAPI UpdateGradientMapComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateGradientMapComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize UpdateGradientMapComponentInstance based on the UpdateGradientMapComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateGradientMapComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update UpdateGradientMapComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		ComponentInstancePtr<RenderableComponent> mRenderer = initComponentInstancePtr(this, &UpdateGradientMapComponent::mRenderer);

		UpdateGradientMapComponent* mResource = nullptr;

		UniformStructInstance* mUniformStruct = nullptr;

		double mElapsedTime = 0.0;
	};
}
