#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "DevicePanel.hpp"

class SettingsPanel;
class SettingsPanel : public DevicePanel
{
public:
    explicit SettingsPanel(MainFrame* frame, wxWindow* parent);
};

#endif // WITH_GUI

#endif // SETTINGSPANEL_H
