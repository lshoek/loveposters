#pragma once
#include <transformcomponent.h>

namespace nap
{
	struct AffineTransform
	{
		AffineTransform(const TransformComponentInstance& transform) :
			mTranslate(transform.getTranslate()), mRotate(transform.getRotate()), mScale(transform.getScale()), mUniformScale(transform.getUniformScale()) {}
		glm::vec3 mTranslate;
		glm::quat mRotate;
		glm::vec3 mScale;
		float mUniformScale;
	};
}
