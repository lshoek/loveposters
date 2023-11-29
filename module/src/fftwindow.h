#pragma once

// Local includes
#include <fftaudionodecomponent.h>

// External Includes
#include <appguiwidget.h>
#include <nap/resourceptr.h>

namespace nap
{
	class IMGuiService;
	class ParameterService;

    /**
     * FFTWindow
     */
	class NAPAPI FFTWindow : public AppGUIWindow
	{
		RTTI_ENABLE(AppGUIWindow)

	public:
		FFTWindow(AppGUIService& service);

		std::string mEntityId = "AudioEntity";
		float mMaximum = 1.0f;

	protected:
		/**
		 * Draw window content
		 */
		virtual void drawContent(double deltaTime) override;

		/**
		 *
		 */
		bool fetchFFTAudioComponent(FFTAudioNodeComponentInstance*& outComponent, utility::ErrorState& errorState);

		IMGuiService* mGuiService = nullptr;

		std::unordered_map<std::string, std::vector<float>> mOnsetMap;
		std::vector<float> mOnsetPlot;
		const uint mOnsetBufferSize = 512;
		uint mUpdateCount = 0;
	};

    using FFTWindowObjectCreator = rtti::ObjectCreator<FFTWindow, AppGUIService>;
}
