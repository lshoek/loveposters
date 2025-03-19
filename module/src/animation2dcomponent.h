#pragma once

#include <componentptr.h>
#include <rendercomponent.h>
#include <nap/resourceptr.h>
#include <imagefromfilegroup.h>

namespace nap
{
	class Animation2DComponentInstance;

	/**
	 *	Animation2DComponent
	 */
	class NAPAPI Animation2DComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(Animation2DComponent, Animation2DComponentInstance)
	public:
		ResourcePtr<ImageFromFileGroup> mFrames;    ///< Property: 'Frames' Animation frames
    	double mFramesPerSecond = 1.0;				///< Property: 'FramesPerSecond' Frames per second
	};


	/**
	 * Animation2DComponentInstance
	 */
	class NAPAPI Animation2DComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		Animation2DComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

		/**
		 * Initialize Animation2DComponentInstance based on the Animation2DComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the Animation2DComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *
		 * @param deltaTime
		 */
		void update(double deltaTime) override;

		/**
		 *
		 * @return
		 */
		Texture2D& getFrame() const { return *mFrames[mFrameIndex]; }

	private:

		std::vector<ImageFromFile*> mFrames;

		double mElapsedTime = 0.0;
		double mFrameInterval = 0.0;
		uint mFrameIndex = 0;
    };
}
