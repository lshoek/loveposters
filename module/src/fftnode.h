/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "fft.h"

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/dirtyflag.h>
#include <audio/utility/safeptr.h>
#include <audio/core/process.h>

namespace nap
{
	// Forward declarations
	class AudioService;
		
	/**
	 * Node to measure the amplitude level of an audio signal.
	 * Can be used for VU meters or envelope followers for example.
	 * Can switch between measuring peaks of the signal or the root mean square.
	 */
	class NAPAPI FFTNode : public audio::Node
	{
	public:
		/**
		 * @param audioService: the NAP audio service.
		 * @param analysisWindowSize: the time window in milliseconds that will be used to generate one single output value. Also the period that corresponds to the analysis frequency.
		 * @param rootProcess: indicates that the node is registered as root process with the @AudioNodeManager and is processed automatically.
		 */
		FFTNode(audio::NodeManager& nodeManager);

		virtual ~FFTNode();

		/**
		 * Inherited from Node
		 */
		void process() override;

		/**
		 * Return the cached FFT result
		 */
		const std::vector<std::complex<float>>& getFFT() const				{ return mBuffer->getFFT(); }

		/**
		 * Return whether the FFT was updated since the last fetch
		 */
		bool isUpdated() const												{ return mBuffer->isDirty(); }

		// The input for the audio signal that will be analyzed.
		audio::InputPin mInput = { this };

	private:
		std::unique_ptr<FFTBuffer>	mBuffer;
	};
}
