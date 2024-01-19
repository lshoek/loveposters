// local includes
#include "fftwindow.h"
#include "fftutils.h"

// nap includes
#include <entity.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <nap/core.h>
#include <imguiservice.h>
#include <imguiutils.h>
#include <appguiservice.h>
#include <fluxmeasurementcomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FFTWindow)
    RTTI_CONSTRUCTOR(nap::AppGUIService&)
	RTTI_PROPERTY("EntityID", &nap::FFTWindow::mEntityId, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Maximum", &nap::FFTWindow::mMaximum, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool FFTWindow::fetchFFTAudioComponent(FFTAudioNodeComponentInstance*& outComponent, utility::ErrorState& errorState)
	{
		// Quite hacky but very useful
		const auto scene = mGuiService->getCore().getResourceManager()->findObject<Scene>("Scene");
		if (scene == nullptr)
		{
			errorState.fail("No nap::Scene with name 'Scene' found");
			return false;
		}

		auto entity = scene->findEntity(mEntityId);
		if (entity == nullptr)
		{
			errorState.fail(utility::stringFormat("No nap::Entity with name `%s` found", mEntityId.c_str()).c_str());
			return false;
		}

		std::vector<FFTAudioNodeComponentInstance*> comps;
		entity->getComponentsOfTypeRecursive<FFTAudioNodeComponentInstance>(comps);

		if (comps.empty())
		{
			errorState.fail("No nap::FFTAudioComponentInstance in entity");
			return false;
		}
		outComponent = comps.front();

		return true;
	}


	FFTWindow::FFTWindow(AppGUIService& service) :
		AppGUIWindow(service), mGuiService(service.getCore().getService<IMGuiService>())
	{
		mOnsetPlot.resize(mOnsetBufferSize);
	}


	void FFTWindow::drawContent(double deltaTime)
	{
		FFTAudioNodeComponentInstance* fft_comp = nullptr;
		utility::ErrorState error_state;
		if (!fetchFFTAudioComponent(fft_comp, error_state))
		{
			nap::Logger::error(error_state.toString().c_str());
			assert(false);
			return;
		}
		const ImVec2 graph_size = { 0.0f, 100.0f };
		const auto& amps = fft_comp->getFFTBuffer().getAmplitudeSpectrum();

		ImGui::PlotLines("FFT", amps.data(), amps.size()/2, 0, 0, 0.0f, mMaximum, graph_size);

		//float average = utility::average(amps_copy);
		//ImGui::SliderFloat("Average", &average, 0.0f, 1.0f);

		//float centroid = utility::centroid(amps_copy);
		//ImGui::SliderFloat("Centroid", &centroid, 0.0f, 1.0f);

		// Onset detection
		std::vector<FluxMeasurementComponentInstance*> onset_comps;
		fft_comp->getEntityInstance()->getComponentsOfTypeRecursive<FluxMeasurementComponentInstance>(onset_comps);

		if (!onset_comps.empty())
		{
			const auto& onset_comp = onset_comps.front();
			for (const auto& item : onset_comp->getParameterItems())
			{
				auto it = mOnsetMap.find(item->mID);
				if (it == mOnsetMap.end())
				{
					auto& onsets = mOnsetMap.insert({ item->mID, {} });
					onsets.first->second.resize(mOnsetBufferSize);
					it = onsets.first;
				}
				auto& onsets = it->second;
				onsets[mUpdateCount % onsets.size()] = item->mParameter->mValue;

				// Circular buffer using two copies
				uint start_index = (mUpdateCount + 1) % mOnsetBufferSize;
				std::copy(onsets.begin(), onsets.begin() + start_index, mOnsetPlot.begin() + (mOnsetBufferSize - start_index));
				std::copy(onsets.begin() + start_index, onsets.end(), mOnsetPlot.begin());

				ImGui::PlotLines(it->first.c_str(), mOnsetPlot.data(), mOnsetPlot.size(), 0, 0, 0.0f, mMaximum, graph_size);
			}
			++mUpdateCount;
		}
    }
}
