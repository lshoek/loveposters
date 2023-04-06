/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "parameterwindow.h"

// nap includes
#include <imgui/imgui.h>
#include <nap/core.h>
#include <imguiservice.h>
#include <imguiutils.h>
#include <appguiservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterWindow)
    RTTI_CONSTRUCTOR(nap::AppGUIService&)
	RTTI_PROPERTY("ParameterGUIs", &nap::ParameterWindow::mParameterGUIs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	ParameterWindow::ParameterWindow(AppGUIService& service) :
		AppGUIWindow(service), mGuiService(service.getCore().getService<IMGuiService>()) {}


	void ParameterWindow::drawContent(double deltaTime)
	{
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar(this->mID.c_str(), tab_bar_flags))
		{
			for (auto& gui : mParameterGUIs)
			{
				ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
				ImGui::PushID(gui->mParameterGroup.get());
				if (ImGui::BeginTabItem(gui->mParameterGroup->mID.c_str()))
				{
					gui->show(false);
					ImGui::EndTabItem();
				}
				ImGui::PopID();
			}
			ImGui::EndTabBar();
		}
    }
}
