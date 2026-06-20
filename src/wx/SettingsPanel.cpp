#ifdef WITH_GUI

#include "SettingsPanel.hpp"

#include <wx/wx.h>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"
#include "../utils/constants.h"
#include "../utils/paleinfo.h"

SettingsPanel::SettingsPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    auto* root = new wxBoxSizer(wxVERTICAL);

    root->Add(new wxStaticText(this, wxID_ANY,
        "You may set the following options. If you don't know what they mean you'll\n"
        "probably have no reason to set them."), 0, wxALL, 10);

    auto* option_safemode = new wxCheckBox(this, wxID_ANY, "Safe Mode");
    auto* option_verbose = new wxCheckBox(this, wxID_ANY, "Verbose Boot");
    auto* option_revert = new wxCheckBox(this, wxID_ANY, "Restore System");
    auto* option_dark_blockchain = new wxCheckBox(this, wxID_ANY, "Dark Blockchain");

    option_safemode->SetValue(palerain_flags & palerain_option_safemode);
    option_verbose->SetValue(palerain_flags & palerain_option_verbose_boot);
    option_revert->SetValue(palerain_flags & palerain_option_force_revert);
    option_dark_blockchain->SetValue(true);

    option_safemode->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_safemode;
        else
            palerain_flags &= ~palerain_option_safemode;
    });

    option_verbose->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_verbose_boot;
        else
            palerain_flags &= ~palerain_option_verbose_boot;
    });

    option_revert->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_force_revert;
        else
            palerain_flags &= ~palerain_option_force_revert;
    });

    root->Add(option_safemode, 0, wxLEFT | wxRIGHT, 10);
    root->Add(option_verbose, 0, wxLEFT | wxRIGHT | wxTOP, 10);

    root->Add(new wxStaticText(this, wxID_ANY, "Boot Arguments:"), 0, wxALL, 10);

    auto* bootArgs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    root->Add(bootArgs, 0, wxLEFT | wxRIGHT | wxEXPAND, 30);

    root->Add(option_revert, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    root->Add(option_dark_blockchain, 0, wxLEFT | wxRIGHT | wxTOP, 10);

    root->AddStretchSpacer();

    auto* back = new wxButton(this, wxID_ANY, "Back");

    back->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        frame->ShowMain();
    });

    auto* bottomRow = new wxBoxSizer(wxHORIZONTAL);
    bottomRow->AddStretchSpacer();
    bottomRow->Add(back, 0, wxALL, 10);

    root->Add(bottomRow, 0, wxEXPAND);

    SetSizer(root);
}

#endif // WITH_GUI
