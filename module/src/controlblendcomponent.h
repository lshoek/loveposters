#pragma once

#include <componentptr.h>
#include <rendercomponent.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

namespace nap
{
	class ControlBlendComponentInstance;

	/**
	 *	ControlBlendComponent
	 */
	class NAPAPI ControlBlendComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ControlBlendComponent, ControlBlendComponentInstance)
	public:
		ResourcePtr<ParameterFloat> mBlend;             ///< Property: 'Blend' Blend value [0, 1]
		ResourcePtr<ParameterFloat> mBrightness;        ///< Property: 'Brightness' Brightness value [-1, 1]
        ComponentPtr<RenderableComponent> mRenderer;    ///< Property: 'Renderer' Component that renders the blend effect
    };


	/**
	 * ControlBlendComponentInstance
	 */
	class NAPAPI ControlBlendComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ControlBlendComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

		/**
		 * Initialize ControlBlendComponentInstance based on the ControlBlendComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the ControlBlendComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
        void onBlendChanged(float);
        Slot<float> mBlendChangedSlot = { this, &ControlBlendComponentInstance::onBlendChanged };

		void onBrightnessChanged(float);
		Slot<float> mBrightnessChangedSlot = { this, &ControlBlendComponentInstance::onBrightnessChanged };

        nap::ComponentInstancePtr<RenderableComponent> mRenderer = initComponentInstancePtr(this, &ControlBlendComponent::mRenderer);

        UniformFloatInstance* mBlendUniform = nullptr;
        UniformFloatInstance* mBrightnessUniform = nullptr;
    };
}
