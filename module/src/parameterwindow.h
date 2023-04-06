#pragma once

// External Includes
#include <appguiwidget.h>
#include <parametergui.h>

namespace nap
{
	class IMGuiService;
	class ParameterService;

    /**
     * PerformanceWindow
     */
    class NAPAPI ParameterWindow : public AppGUIWindow
    {
        RTTI_ENABLE(AppGUIWindow)

    public:
		ParameterWindow(AppGUIService& service);

		std::vector<ResourcePtr<ParameterGUI>> mParameterGUIs;			///< Property: 'ParameterGUIs'

    protected:
		/**
		 * Draw window content
		 */
		virtual void drawContent(double deltaTime) override;

		IMGuiService* mGuiService = nullptr;
    };

    using ParameterWindowObjectCreator = rtti::ObjectCreator<ParameterWindow, AppGUIService>;
}
