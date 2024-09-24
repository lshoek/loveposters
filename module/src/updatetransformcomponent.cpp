#include "updatetransformcomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/noise.hpp>

// nap::UpdateTransformComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateTransformComponent)
	RTTI_PROPERTY("Position", &nap::UpdateTransformComponent::mPosition, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Scale", &nap::UpdateTransformComponent::mScale, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Angle", &nap::UpdateTransformComponent::mAngle, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Enable", &nap::UpdateTransformComponent::mEnable, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::UpdateTransformComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateTransformComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void UpdateTransformComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	bool UpdateTransformComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<UpdateTransformComponent>();
		mTransformComponent = &getEntityInstance()->getComponent<TransformComponentInstance>();
		return true;
	}


	void UpdateTransformComponentInstance::update(double deltaTime)
	{
		if (mResource->mEnable)
		{
			if (mResource->mAngle != nullptr)
			{
				auto orient = glm::angleAxis(glm::radians(mResource->mAngle->mValue), math::Z_AXIS);
				mTransformComponent->setRotate(orient);
			}

			if (mResource->mScale != nullptr)
			{
				mTransformComponent->setScale(mResource->mScale->mValue);
			}

			if (mResource->mPosition != nullptr)
			{
				mTransformComponent->setTranslate(mResource->mPosition->mValue);
			}
		}
	}
}
