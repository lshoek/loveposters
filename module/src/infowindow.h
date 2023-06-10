#pragma once

// External Includes
#include <appguiwidget.h>
#include <parametergui.h>

namespace nap
{
	class IMGuiService;
	class ParameterService;

    /**
     * InfoWindow
     */
    class NAPAPI InfoWindow : public AppGUIWindow
    {
        RTTI_ENABLE(AppGUIWindow)

    public:
		InfoWindow(AppGUIService& service);
		std::string mNotes;

    protected:
		/**
		 * Draw window content
		 */
		virtual void drawContent(double deltaTime) override;

		IMGuiService* mGuiService = nullptr;
    };

    using InfoWindowObjectCreator = rtti::ObjectCreator<InfoWindow, AppGUIService>;
}
