// Local Includes
#include "lovepostersapp.h"

// External Includes
#include <utility/fileutils.h>
#include <imguiutils.h>
#include <inputrouter.h>
#include <perspcameracomponent.h>
#include <rendertotexturecomponent.h>
#include <renderbloomcomponent.h>
#include <renderdofcomponent.h>
#include <rendermultivideocomponent.h>
#include <funtransformcomponent.h>
#include <orthocameracomponent.h>
#include <audio/component/playbackcomponent.h>
#include <depthsorter.h>

namespace nap 
{    
    bool LovePostersApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService			= getCore().getService<nap::RenderService>();
		mRenderAdvancedService	= getCore().getService<nap::RenderAdvancedService>();
		mSceneService			= getCore().getService<nap::SceneService>();
		mInputService			= getCore().getService<nap::InputService>();
		mGuiService				= getCore().getService<nap::IMGuiService>();

		// Fetch the resource manager
        mResourceManager 		= getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find nap::RenderWindow with name: %s", "Window"))
			return false;

		mColorTarget = mResourceManager->findObject<RenderTarget>("ColorTarget");
		if (!error.check(mColorTarget != nullptr, "unable to find nap::RenderTarget with name: %s", "ColorTarget"))
			return false;

		// Stencil target (not required)
		mStencilTarget = mResourceManager->findObject<RenderTarget>("StencilTarget");

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Get the camera and origin Gnomon entity
		mCameraEntity 			= mScene->findEntity("CameraEntity");
		mWorldEntity 			= mScene->findEntity("WorldEntity");
		mAudioEntity 			= mScene->findEntity("AudioEntity");
		mVideoEntity 			= mScene->findEntity("VideoEntity");
		mRenderEntity 			= mScene->findEntity("RenderEntity");
		mRenderCameraEntity 	= mScene->findEntity("RenderCameraEntity");
		mWarpEntity 			= mScene->findEntity("WarpEntity");

		// Start video players
		auto video_players = mResourceManager->getObjects<VideoPlayer>();
		for (auto& player : video_players)
			player->play();

		mAppGUIs = mResourceManager->getObjects<AppGUI>();

        return true;
    }


    // Called when the window is going to render
    void LovePostersApp::render()
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

			// Render shadows
			const auto shadow_mask = mRenderService->getRenderMask("Shadow");
			mRenderAdvancedService->renderShadows(render_comps, true, shadow_mask);

			// Video
			auto* multi_video = mRenderEntity->findComponent<RenderMultiVideoComponentInstance>();
			if (multi_video != nullptr)
				multi_video->draw();

			// Render stencil geometry to stencil target
			if (mStencilTarget != nullptr)
			{
				auto stencil_mask = mRenderService->getRenderMask("Stencil");
				mStencilTarget->beginRendering();
				mRenderService->renderObjects(*mStencilTarget, cam, render_comps, stencil_mask);
				mStencilTarget->endRendering();
			}

			// Offscreen color pass -> Render all available geometry to the color texture bound to the render target.
			mColorTarget->beginRendering();
			{
				auto* composite_video = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("CompositeVideo");
				if (composite_video != nullptr)
				{
					const auto size = static_cast<glm::vec2>(mColorTarget->getBufferSize());
					const auto proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, size.x, 0.0f, size.y);
					mRenderService->renderObjects(*mColorTarget, proj_matrix, glm::identity<glm::mat4>(), { composite_video }, std::bind(&sorter::sortObjectsByDepth, std::placeholders::_1, std::placeholders::_2));
				}

				auto mask = mRenderService->getRenderMask("Default");
				mRenderService->renderObjects(*mColorTarget, cam, render_comps, std::bind(&sorter::sortObjectsByZ, std::placeholders::_1), (mask != 0) ? mask : mask::all);

				if (mShowLocators)
					mRenderAdvancedService->renderLocators(*mColorTarget, cam, true);
			}
			mColorTarget->endRendering();

			// DOF
			auto* dof = mRenderEntity->findComponent<RenderDOFComponentInstance>();
			if (dof != nullptr)
				dof->draw();

			// Offscreen contrast pass -> Use previous `ColorTexture` as input, `ColorTextureFX` as output.
			// Input and output resources of these operations are described in JSON in their appropriate components.
			auto* change_color = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("ChangeColor"); assert(change_color != nullptr);
			change_color->draw();

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
			if (composite_comp != nullptr)
			{
				// Render composite component
				// The nap::RenderToTextureComponentInstance transforms a plane to match the window dimensions and applies the texture to it.
				mRenderService->renderObjects(*mRenderWindow, cam, { composite_comp });
			}
			else if (mWarpEntity != nullptr)
			{
				// Get composite component responsible for rendering final texture
				std::vector<RenderableComponentInstance*> render_comps;
				mWarpEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

				// Render warp components
				mRenderService->renderObjects(*mRenderWindow, cam, { render_comps });	
			}

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


    void LovePostersApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
		mRenderService->addEvent(std::move(windowEvent));
    }


    void LovePostersApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(KeyPressEvent)))
		{
			auto* press_event = static_cast<KeyPressEvent*>(inputEvent.get());

			// Evaluate key
			switch (press_event->mKey)
			{
				case EKeyCode::KEY_ESCAPE:
				{
					quit();
					break;
				}

				case nap::EKeyCode::KEY_f:
				{
					mRenderWindow->toggleFullscreen();
					break;
				}

				case nap::EKeyCode::KEY_g:
				{
					mShowGUI = !mShowGUI;
					break;
				}

				case nap::EKeyCode::KEY_l:
				{
					mShowLocators = !mShowLocators;
					break;
				}

				case nap::EKeyCode::KEY_r:
				{
					mRandomizeOffset = !mRandomizeOffset;
					std::vector<FunTransformComponentInstance*> move_comps;
					mWorldEntity->getComponentsOfTypeRecursive<FunTransformComponentInstance>(move_comps);
					for (auto& comp : move_comps)
						comp->randomize(mRandomizeOffset);

					break;
				}

				case nap::EKeyCode::KEY_p:
				{
					// For testing purposes only
					auto* playback = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();
					if (playback != nullptr)
					{
						if (!playback->isPlaying())
							playback->start();
						else
							playback->stop();
					}
					break;
				}
			}

		}
		mInputService->addEvent(std::move(inputEvent));
    }


    void LovePostersApp::update(double deltaTime)
    {
		// Use a default input router to forward input events (recursively) to all input components in the scene
		// This is explicit because we don't know what entity should handle the events from a specific window.
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		if (mShowGUI)
		{
			for (auto& gui : mAppGUIs)
				gui->draw(deltaTime);
		}
    }
}
