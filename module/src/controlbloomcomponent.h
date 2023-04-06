#pragma once

#include <component.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <rendertotexturecomponent.h>

namespace nap
{
	class ControlBloomComponentInstance;

	/**
	 *	ControlBloomComponent
	 */
	class NAPAPI ControlBloomComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ControlBloomComponent, ControlBloomComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterFloat> mMovementParam;
		ResourcePtr<ParameterFloat> mIntensityParam;

		ComponentPtr<RenderToTextureComponent> mRenderToTextureComponent;
	};


	/**
	 * ControlBloomComponentInstance	
	 */
	class NAPAPI ControlBloomComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ControlBloomComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize ControlBloomComponentInstance based on the ControlBloomComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the ControlBloomComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update ControlBloomComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		ControlBloomComponent* mResource = nullptr;
		ComponentInstancePtr<RenderToTextureComponent> mRenderToTextureComponent = { this, &nap::ControlBloomComponent::mRenderToTextureComponent };

		UniformFloatInstance* mBlendUniform = nullptr;
	};
}
