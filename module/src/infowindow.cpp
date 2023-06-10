// local includes
#include "infowindow.h"

// nap includes
#include <imgui/imgui.h>
#include <nap/core.h>
#include <imguiservice.h>
#include <imguiutils.h>
#include <appguiservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::InfoWindow)
    RTTI_CONSTRUCTOR(nap::AppGUIService&)
	RTTI_PROPERTY("Notes", &nap::InfoWindow::mNotes, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	InfoWindow::InfoWindow(AppGUIService& service) :
		AppGUIWindow(service), mGuiService(service.getCore().getService<IMGuiService>())
	{ }


	void InfoWindow::drawContent(double deltaTime)
	{
		ImGui::Text("%.02ffps | %.02fms", mGuiService->getCore().getFramerate(), deltaTime * 1000.0);
		ImGui::Dummy({ 0.0f, 2.0f * mGuiService->getScale() });

		ImGui::TextWrapped(mNotes.c_str());
		ImGui::Dummy({ 0.0f, 2.0f * mGuiService->getScale() });
    }
}
