// Local Includes
#include "graphictoolapp.h"

// External Includes
#include <utility/fileutils.h>
#include <imguiutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <rendertotexturecomponent.h>
#include <renderbloomcomponent.h>
#include <renderdofcomponent.h>
#include <funtransformcomponent.h>
#include <audio/component/playbackcomponent.h>

namespace nap 
{    
    bool GraphicToolApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService			= getCore().getService<nap::RenderService>();
		mRenderAdvancedService	= getCore().getService<nap::RenderAdvancedService>();
		mSceneService			= getCore().getService<nap::SceneService>();
		mInputService			= getCore().getService<nap::InputService>();
		mGuiService				= getCore().getService<nap::IMGuiService>();

		// Fetch the resource manager
        mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		mRenderTarget = mResourceManager->findObject<RenderTarget>("ColorTarget");

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Get the camera and origin Gnomon entity
		mCameraEntity = mScene->findEntity("CameraEntity");
		mWorldEntity = mScene->findEntity("WorldEntity");
		mAudioEntity = mScene->findEntity("AudioEntity");
		mRenderEntity = mScene->findEntity("RenderEntity");
		mRenderCameraEntity = mScene->findEntity("RenderCameraEntity");

		mAppGUIs = mResourceManager->getObjects<AppGUI>();

		// Debug
		//setFramerate(60.0f);
		//capFramerate(true);

		// All done!
        return true;
    }


    // Called when the window is going to render
    void GraphicToolApp::render()
    {
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the offscreen render target. Rendering always happens after compute.
		// This prepares a command buffer and starts a render pass.
		if (mRenderService->beginHeadlessRecording())
		{
			// The world entity holds all visible renderable components in the scene.
			std::vector<RenderableComponentInstance*> render_comps;
			mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

			// Get Perspective camera to render with
			auto& cam = mCameraEntity->getComponent<CameraComponentInstance>();

			// Offscreen color pass -> Render all available geometry to the color texture bound to the render target.
			mRenderTarget->beginRendering();
			mRenderService->renderObjects(*mRenderTarget, cam, render_comps);
			mRenderTarget->endRendering();

			// DOF
			mRenderEntity->getComponent<RenderDOFComponentInstance>().draw();

			// Offscreen contrast pass -> Use previous `ColorTexture` as input, `ColorTextureFX` as output.
			// Input and output resources of these operations are described in JSON in their appropriate components.
			mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("ChangeColor")->draw();

			// Offscreen bloom pass -> Use `ColorTextureFX` as input and output.
			// This is fine as the bloom component blits the input to internally managed render targets on which the effect is applied.
			// the effect result is blitted to the output texture. The effect therefore does not write to itself.
			mRenderEntity->getComponent<RenderBloomComponentInstance>().draw();

			// End headless recording
			mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get Perspective camera to render with
			auto& cam = mRenderCameraEntity->getComponent<CameraComponentInstance>();

			// Get composite component responsible for rendering final texture
			auto* composite_comp = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("BlendTogether");

			// Render composite component
			// The nap::RenderToTextureComponentInstance transforms a plane to match the window dimensions and applies the texture to it.
			mRenderService->renderObjects(*mRenderWindow, cam, { composite_comp });

			// Draw GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
    }


    void GraphicToolApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
		mRenderService->addEvent(std::move(windowEvent));
    }


    void GraphicToolApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(KeyPressEvent)))
		{
			KeyPressEvent* press_event = static_cast<KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();

			if (press_event->mKey == nap::EKeyCode::KEY_h)
				mHideGUI = !mHideGUI;

			// For testing purposes only
			if (press_event->mKey == nap::EKeyCode::KEY_p)
			{
				auto* playback = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();
				if (playback != nullptr)
				{
					if (!playback->isPlaying())
						playback->start();
					else
						playback->stop();
				}
			}
		}
		mInputService->addEvent(std::move(inputEvent));
    }


    void GraphicToolApp::update(double deltaTime)
    {
		// Use a default input router to forward input events (recursively) to all input components in the scene
		// This is explicit because we don't know what entity should handle the events from a specific window.
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		if (!mHideGUI)
		{
			for (auto& gui : mAppGUIs)
				gui->draw(deltaTime);

			ImGui::Begin("DOF");
			ImGui::Image(*mResourceManager->findObject<RenderTexture2D>("DOFTexture"), { 1920.0f, 1080.0f });
			ImGui::End();
		}
    }
}
