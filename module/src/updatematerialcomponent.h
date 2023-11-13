#pragma once

#include <component.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametersimple.h>
#include <parametercolor.h>

namespace nap
{
	class UpdateMaterialComponentInstance;
	class RenderableMeshComponentInstance;
	class UniformStructInstance;

	/**
	 *	UpdateMaterialComponent
	 */
	class NAPAPI UpdateMaterialComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateMaterialComponent, UpdateMaterialComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterRGBAColorFloat>	mAmbient;
		ResourcePtr<ParameterRGBColorFloat>		mDiffuse;
		ResourcePtr<ParameterRGBColorFloat>		mSpecular;
		ResourcePtr<ParameterVec2>				mFresnel;
		ResourcePtr<ParameterFloat>				mShininess;
		ResourcePtr<ParameterFloat>				mAlpha;
		ResourcePtr<ParameterBool>				mEnvironment;
		bool									mUpdateBase = false;
	};


	/**
	 * UpdateMaterialComponentInstance	
	 */
	class NAPAPI UpdateMaterialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateMaterialComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize UpdateMaterialComponentInstance based on the UpdateMaterialComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateMaterialComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update UpdateMaterialComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		UpdateMaterialComponent* mResource = nullptr;
		RenderableMeshComponentInstance* mRenderableMeshComponent = nullptr;

		UniformStructInstance* mUniformStruct = nullptr;
		float mElapsedTime = 0.0f;
	};
}
