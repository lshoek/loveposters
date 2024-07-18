#pragma once

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>

// Module includes
#include <renderservice.h>
#include <renderadvancedservice.h>
#include <imguiservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <scene.h>
#include <renderwindow.h>
#include <rendertarget.h>
#include <entity.h>
#include <app.h>
#include <appgui.h>

#include "audiodevicesettingsgui.h"

namespace nap 
{
	using namespace rtti;

    /**
     * LovePostersApp
     */
    class LovePostersApp : public App 
	{
    public:
		/**
		 * Constructor
		 */
        LovePostersApp(nap::Core& core) : App(core) {}

        /**
         * Initialize all the services and app specific data structures
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
         */
        bool init(utility::ErrorState& error) override;

		/**
		 * Update is called every frame, before render.
		 * @param deltaTime the time in seconds between calls
		 */
		void update(double deltaTime) override;

        /**
         * Render is called after update. Use this call to render objects to a specific target
         */
        void render() override;

        /**
         * Called when the app receives a window message.
		 * @param windowEvent the window message that occurred
         */
        void windowMessageReceived(WindowEventPtr windowEvent) override;

        /**
         * Called when the app receives an input message (from a mouse, keyboard etc.)
		 * @param inputEvent the input event that occurred
         */
        void inputMessageReceived(InputEventPtr inputEvent) override;

        /**
		 * Called when the app is shutting down after quit() has been invoked
		 * @return the application exit code, this is returned when the main loop is exited
         */
		int shutdown() override { return 0; }

    private:
        ResourceManager*			mResourceManager = nullptr;			///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;			///< Render Service that handles render calls
		RenderAdvancedService*		mRenderAdvancedService = nullptr;	///< Render Service that handles render calls
		SceneService*				mSceneService = nullptr;			///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;			///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;				///< Manages GUI related update / draw calls

		ObjectPtr<RenderWindow>		mRenderWindow;						///< Pointer to the render window	
		ObjectPtr<RenderTarget>		mRenderTarget;						///< Pointer to the render window	
		ObjectPtr<Scene>			mScene;								///< Pointer to the main scene
		ObjectPtr<EntityInstance>	mCameraEntity;						///< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance>	mWorldEntity;						///< Pointer to the world entity
		ObjectPtr<EntityInstance>	mAudioEntity;						///< Pointer to the audio entity
		ObjectPtr<EntityInstance>	mRenderEntity;						///< Pointer to the render entity
		ObjectPtr<EntityInstance>	mWarpEntity;						///< Pointer to the warp entity
		ObjectPtr<EntityInstance>	mRenderCameraEntity;				///< Pointer to the render camera entity

		std::vector<ObjectPtr<AppGUI>> mAppGUIs;						///< AppGUIs

		bool mHideGUI = true;
		bool mRandomizeOffset = false;
	};
}
