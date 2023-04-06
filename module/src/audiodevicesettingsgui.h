/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include <appguiwidget.h>
#include <imguiservice.h>
#include <audio/service/audioservice.h>

namespace nap
{
    namespace audio
    {
        /**
         * Object that draws a gui to edit settings for the AudioService at runtime.
         * Enables selecting of audio devices for input and output and changing the buffer size and samplerate.
         */
        class AudioDeviceSettingsWindow : public AppGUIWindow
        {
			RTTI_ENABLE(AppGUIWindow)
        public:
            /**
             * Constructor
             * @param audioService The audio service of which the configuration will be edited by this gui
             */
			AudioDeviceSettingsWindow(AppGUIService& appGUIService);

			/**
			 * Init
			 */
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * Draw window content
			 */
			virtual void drawContent(double deltaTime) override;

        private:
            struct DeviceInfo
            {
                int mIndex = -1;
                std::string mName = "";
                int mChannelCount = 0;
            };

            struct DriverInfo
            {
            	int mIndex = -1;
                std::string mName = "";
                std::vector<DeviceInfo> mInputDevices;
                std::vector<DeviceInfo> mOutputDevices;
            };

        private:
	        AudioService& mAudioService;
			IMGuiService& mGuiService;

            int mDriverSelection = 0;
            int mInputDeviceSelection = 0;
            int mOutputDeviceSelection = 0;
            int mBufferSizeIndex = 0;
            int mSampleRateIndex = 0;
            std::vector<int> mBufferSizes = { 64, 128, 256, 512, 1024, 2048 };
            std::vector<int> mSampleRates = { 44100, 48000 };
            const char* const mBufferSizeNames[6] = { "64", "128", "256", "512", "1024", "2048" };
            const char* const mSampleRateNames[2] = { "44100", "48000" };

            std::vector<DriverInfo> mDrivers;
            
            bool mHasInputs = true;
        };

		using AudioDeviceSettingsWindowObjectCreator = rtti::ObjectCreator<AudioDeviceSettingsWindow, AppGUIService>;
    }
}
