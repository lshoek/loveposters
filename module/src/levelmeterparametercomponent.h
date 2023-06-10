#pragma once

#include <component.h>
#include <parameternumeric.h>
#include <nap/resourceptr.h>
#include <smoothdamp.h>

#include <audio/component/levelmetercomponent.h>

namespace nap
{
	class LevelMeterParameterComponentInstance;

	/**
	 * LevelMeterParameterComponent
	 */
	class NAPAPI LevelMeterParameterComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LevelMeterParameterComponent, LevelMeterParameterComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<audio::LevelMeterComponent> mLevelMeter;
		ResourcePtr<ParameterFloat> mLevelMeterParam;
		ResourcePtr<ParameterFloat> mMultiplyParam;
		float mSmoothtime = 0.01f;
	};


	/**
	 * LevelMeterParameterComponentInstance	
	 */
	class NAPAPI LevelMeterParameterComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LevelMeterParameterComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize LevelMeterParameterComponentInstance based on the LevelMeterParameterComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LevelMeterParameterComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update LevelMeterParameterComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		ComponentInstancePtr<audio::LevelMeterComponent> mLevelMeter = { this, &nap::LevelMeterParameterComponent::mLevelMeter };

		LevelMeterParameterComponent* mResource = nullptr;

	private:
		math::SmoothOperator<float> mLevelSmoother{ 0.0f, 0.0f };
	};
}
