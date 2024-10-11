// Local Includes
#include "pointspritevolume.h"

// External Includes
#include <entity.h>
#include <renderservice.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <mesh.h>
#include <mathutils.h>
#include <renderglobals.h>
#include <transformcomponent.h>

RTTI_BEGIN_CLASS(nap::PointSpriteVolume)
	RTTI_PROPERTY("PointSize",				&nap::PointSpriteVolume::mPointSize,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointScale",				&nap::PointSpriteVolume::mPointScale,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointScaleIntensity",	&nap::PointSpriteVolume::mPointScaleIntensity,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TimeScale",				&nap::PointSpriteVolume::mTimeScale,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointSpriteVolumeInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	PointSpriteVolumeInstance::PointSpriteVolumeInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<RenderService>())
	{ }


	bool PointSpriteVolumeInstance::init(nap::utility::ErrorState& errorState)
	{
		// Fetch resource
		mResource = getComponent<PointSpriteVolume>();

		if (!errorState.check(mResource->mMesh.get()->get_type() == RTTI_OF(ParticleMesh), "Mesh must be of type nap::ParticleMesh"))
			return false;

		// Init base class
		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		// Check transform
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransform != nullptr, "%s: unable to find transform component", mResource->mID.c_str()))
			return false;

		// Cache uniforms
		mPointSizeUniform		= mMaterialInstance.getOrCreateUniform("UBO")->getOrCreateUniform<UniformFloatInstance>("pointSize");
		mPointScaleUniform		= mMaterialInstance.getOrCreateUniform("UBO")->getOrCreateUniform<UniformFloatInstance>("pointScale");
		mElapsedTimeUniform		= mMaterialInstance.getOrCreateUniform("UBO")->getOrCreateUniform<UniformFloatInstance>("elapsedTime");

		return true;
	}


	void PointSpriteVolumeInstance::update(double deltaTime)
	{
		const auto delta_clock = static_cast<float>(deltaTime) * mResource->mTimeScale->mValue;
		mElapsedClockTime += delta_clock;

		const auto point_scale = mResource->mPointScale->mValue * mResource->mPointScaleIntensity->mValue;
		mPointScaleUniform->setValue(point_scale);

		mPointSizeUniform->setValue(mResource->mPointSize->mValue);
		mElapsedTimeUniform->setValue(mElapsedClockTime);
	}


	void PointSpriteVolumeInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Set mvp matrices if present in material
		if (mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		if (mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if (mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		if (mNormalMatrixUniform != nullptr)
			mNormalMatrixUniform->setValue(glm::transpose(glm::inverse(mTransformComponent->getGlobalTransform())));

		if (mCameraWorldPosUniform != nullptr)
			mCameraWorldPosUniform->setValue(math::extractPosition(glm::inverse(viewMatrix)));

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = getMaterialInstance();
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// TODO: move to push/pop cliprect on RenderTarget once it has been ported
		bool has_clip_rect = mClipRect.hasWidth() && mClipRect.hasHeight();
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = mClipRect.getMin().x;
			rect.offset.y = mClipRect.getMin().y;
			rect.extent.width = mClipRect.getWidth();
			rect.extent.height = mClipRect.getHeight();
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mLineWidth);

		// Draw
		vkCmdDraw(commandBuffer, mResource->mMesh->getMeshInstance().getNumVertices(), 1, 0, 0);

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);

		// Restore clipping
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = 0;
			rect.offset.y = 0;
			rect.extent.width = renderTarget.getBufferSize().x;
			rect.extent.height = renderTarget.getBufferSize().y;
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}
	}
}
