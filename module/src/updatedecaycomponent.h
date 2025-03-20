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
	class UpdateDecayComponentInstance;
	class RenderableMeshComponentInstance;
	class UniformStructInstance;

	/**
	 *	UpdateDecayComponent
	 */
	class NAPAPI UpdateDecayComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateDecayComponent, UpdateDecayComponentInstance)
	public:
		ComponentPtr<RenderableComponent> mRenderer;

		ResourcePtr<ParameterFloat> mClockSpeed;
		ResourcePtr<ParameterFloat> mDecay;
		ResourcePtr<ParameterFloat> mVertical;
		ResourcePtr<ParameterFloat> mTangent;
		ResourcePtr<ParameterFloat> mExpand;
		ResourcePtr<ParameterFloat> mExpandBlend;
		ResourcePtr<ParameterFloat> mNoiseScale;
	};


	/**
	 * UpdateDecayComponentInstance	
	 */
	class NAPAPI UpdateDecayComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateDecayComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize UpdateDecayComponentInstance based on the UpdateDecayComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateDecayComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update UpdateDecayComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		ComponentInstancePtr<RenderableComponent> mRenderer = initComponentInstancePtr(this, &UpdateDecayComponent::mRenderer);

		UpdateDecayComponent* mResource = nullptr;

		UniformStructInstance* mUniformStruct = nullptr;

		double mElapsedTime = 0.0;
	};
}
