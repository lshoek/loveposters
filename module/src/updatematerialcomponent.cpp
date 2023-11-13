#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <blinnphongcolorshader.h>
#include <uniformupdate.h>

// nap::UpdateMaterialComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
	RTTI_PROPERTY("Ambient",			&nap::UpdateMaterialComponent::mAmbient,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Diffuse",			&nap::UpdateMaterialComponent::mDiffuse,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Specular",			&nap::UpdateMaterialComponent::mSpecular,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Fresnel",			&nap::UpdateMaterialComponent::mFresnel,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Shininess",			&nap::UpdateMaterialComponent::mShininess,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Alpha",				&nap::UpdateMaterialComponent::mAlpha,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Environment",		&nap::UpdateMaterialComponent::mEnvironment,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpdateBase",			&nap::UpdateMaterialComponent::mUpdateBase,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::UpdateMaterialComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateMaterialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void UpdateMaterialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(RenderableMeshComponent));
	}


	bool UpdateMaterialComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<UpdateMaterialComponent>();
		mRenderableMeshComponent = &getEntityInstance()->getComponent<RenderableMeshComponentInstance>();

		/**
		 * Assume BlinnPhongShader material interface
		 */
		auto& mat_instance = mRenderableMeshComponent->getMaterialInstance();
		mUniformStruct = mResource->mUpdateBase ? mat_instance.getMaterial().findUniform("UBO") : mat_instance.getOrCreateUniform("UBO");

		auto* uni = mUniformStruct;
		if (!errorState.check(uni != nullptr, "Missing uniform struct with name `UBO`"))
			return false;

		auto* ambient = uni->getOrCreateUniform<UniformVec4Instance>("ambient");
		if (ambient != nullptr && mResource->mAmbient != nullptr)
			registerUniformUpdate(*ambient, *mResource->mAmbient);

		auto* diffuse = uni->getOrCreateUniform<UniformVec3Instance>("diffuse");
		if (diffuse != nullptr && mResource->mDiffuse != nullptr)
			registerUniformUpdate(*diffuse, *mResource->mDiffuse);

		auto* specular = uni->getOrCreateUniform<UniformVec3Instance>("specular");
		if (specular != nullptr && mResource->mSpecular != nullptr)
			registerUniformUpdate(*specular, *mResource->mSpecular);

		auto* fresnel = uni->getOrCreateUniform<UniformVec2Instance>("fresnel");
		if (fresnel != nullptr && mResource->mFresnel != nullptr)
			registerUniformUpdate(*fresnel, *mResource->mFresnel);

		auto* shininess = uni->getOrCreateUniform<UniformFloatInstance>("shininess");
		if (shininess != nullptr && mResource->mShininess != nullptr)
			registerUniformUpdate(*shininess, *mResource->mShininess);

		auto* alpha = uni->getOrCreateUniform<UniformFloatInstance>("alpha");
		if (alpha != nullptr && mResource->mAlpha != nullptr)
			registerUniformUpdate(*alpha, *mResource->mAlpha);

		auto* environment = uni->getOrCreateUniform<UniformUIntInstance>("environment");
		if (environment != nullptr && mResource->mEnvironment != nullptr)
			registerUniformUpdate(*environment, *mResource->mEnvironment);

		return true;
	}


	void UpdateMaterialComponentInstance::update(double deltaTime)
	{
		mElapsedTime += static_cast<float>(deltaTime);
		auto* elapsed_time_uni = mUniformStruct->getOrCreateUniform<UniformFloatInstance>("elapsedTime");
		if (elapsed_time_uni != nullptr)
			elapsed_time_uni->setValue(mElapsedTime);
	}
}
