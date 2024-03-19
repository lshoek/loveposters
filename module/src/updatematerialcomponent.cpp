#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <blinnphongcolorshader.h>

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

		auto* ambient = uni->getOrCreateUniform<UniformVec3Instance>("ambient");
		if (ambient != nullptr && mResource->mAmbient != nullptr)
		{
			ambient->setValue(mResource->mAmbient->mValue);
			mAmbientChangedSlot.setFunction(std::bind(&UpdateMaterialComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, ambient));
			mResource->mAmbient->valueChanged.connect(mAmbientChangedSlot);
		}
		
		auto* diffuse = uni->getOrCreateUniform<UniformVec3Instance>("diffuse");
		if (diffuse != nullptr && mResource->mDiffuse != nullptr)
		{
			diffuse->setValue(mResource->mDiffuse->mValue);
			mDiffuseChangedSlot.setFunction(std::bind(&UpdateMaterialComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, diffuse));
			mResource->mDiffuse->valueChanged.connect(mDiffuseChangedSlot);
		}

		auto* specular = uni->getOrCreateUniform<UniformVec3Instance>("specular");
		if (specular != nullptr && mResource->mSpecular != nullptr)
		{
			specular->setValue(mResource->mSpecular->mValue);
			mSpecularChangedSlot.setFunction(std::bind(&UpdateMaterialComponentInstance::onUniformRGBColorUpdate, this, std::placeholders::_1, specular));
			mResource->mSpecular->valueChanged.connect(mSpecularChangedSlot);
		}

		auto* fresnel = uni->getOrCreateUniform<UniformVec2Instance>("fresnel");
		if (fresnel != nullptr && mResource->mFresnel != nullptr)
		{
			fresnel->setValue(mResource->mFresnel->mValue);
			mFresnelChangedSlot.setFunction(std::bind(&UpdateMaterialComponentInstance::onUniformValueUpdate<glm::vec2>, this, std::placeholders::_1, fresnel));
			mResource->mFresnel->valueChanged.connect(mFresnelChangedSlot);
		}

		auto* shininess = uni->getOrCreateUniform<UniformFloatInstance>("shininess");
		if (shininess != nullptr && mResource->mShininess != nullptr)
		{
			shininess->setValue(mResource->mShininess->mValue);
			mShininessChangedSlot.setFunction(std::bind(&UpdateMaterialComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, shininess));
			mResource->mShininess->valueChanged.connect(mShininessChangedSlot);
		}

		auto* alpha = uni->getOrCreateUniform<UniformFloatInstance>("alpha");
		if (alpha != nullptr && mResource->mAlpha != nullptr)
		{
			alpha->setValue(mResource->mAlpha->mValue);
			mAlphaChangedSlot.setFunction(std::bind(&UpdateMaterialComponentInstance::onUniformValueUpdate<float>, this, std::placeholders::_1, alpha));
			mResource->mAlpha->valueChanged.connect(mAlphaChangedSlot);
		}

		auto* environment = uni->getOrCreateUniform<UniformUIntInstance>("environment");
		if (environment != nullptr && mResource->mEnvironment != nullptr)
		{
			environment->setValue(mResource->mEnvironment->mValue);
			mEnvironmentChangedSlot.setFunction(std::bind(&UpdateMaterialComponentInstance::onUniformBoolUpdate, this, std::placeholders::_1, environment));
			mResource->mEnvironment->valueChanged.connect(mEnvironmentChangedSlot);
		}

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
