#ifdef WITH_GUI

#include "app.hpp"
#include <wx/wx.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statline.h>
#include <string>
#include "../utils/constants.h"
#include "../utils/paleinfo.h"
#include "../utils/log.h"
#include "../exploit/logo.h"

class MainFrame;

class MainPanel : public wxPanel
{
public:
    explicit MainPanel(MainFrame* frame, wxWindow* parent);
};

class SettingsPanel : public wxPanel
{
public:
    explicit SettingsPanel(MainFrame* frame, wxWindow* parent);
};

class MainFrame : public wxFrame
{
public:
    MainFrame()
        : wxFrame(
            nullptr,
            wxID_ANY,
            "palera1n – Version beta " + wxString(PALERAIN_VERSION),
            wxDefaultPosition,
            wxSize(480, 360),
            wxDEFAULT_FRAME_STYLE & ~(wxMAXIMIZE_BOX | wxRESIZE_BORDER))
    {
        auto* root = new wxBoxSizer(wxVERTICAL);

        m_main = new MainPanel(this, this);
        m_settings = new SettingsPanel(this, this);

        root->Add(m_main, 1, wxEXPAND);
        root->Add(m_settings, 1, wxEXPAND);

        SetSizer(root);

        ShowMain();
    }

    void ShowMain()
    {
        m_main->Show();
        m_settings->Hide();
        Layout();
    }

    void ShowSettings()
    {
        m_main->Hide();
        m_settings->Show();
        Layout();
    }

private:
    MainPanel* m_main;
    SettingsPanel* m_settings;
};

MainPanel::MainPanel(MainFrame* frame, wxWindow* parent)
    : wxPanel(parent)
{
    auto* root = new wxBoxSizer(wxVERTICAL);

    wxInitAllImageHandlers();

    wxMemoryInputStream stream(resources_logo_png, resources_logo_png_len);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    wxBitmap bmp(img);
    auto* logo = new wxStaticBitmap(this, wxID_ANY, bmp);

    auto* titletext = new wxStaticText(this, wxID_ANY, "Welcome to palera1n!");
    wxFont titlefont = titletext->GetFont();
    titlefont.SetWeight(wxFONTWEIGHT_BOLD);
    titletext->SetFont(titlefont);

    auto* left = new wxBoxSizer(wxVERTICAL);
    left->Add(titletext, 0, wxTOP | wxLEFT, 10);

    auto* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(left, 0, wxALIGN_TOP);
    row->AddStretchSpacer();
    row->Add(logo, 0, wxTOP | wxRIGHT, 10);
    root->Add(row, 0, wxEXPAND);

    root->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    auto* credtext = new wxStaticText(this, wxID_ANY, "Made by: asdfugil, kok3shidoll, claration, mineek, staturnz\n\nThanks to: itsnebulalol, llsc12, lrdsnow, nikias (libimobiledevice), Checkra1n\n(Siguza, axi0mx, littlelailo et al.), Procursus (Hayden Seay, Cameron Katri,\nKeto et al.)\n\nWith 💖 from C (claration)");
    root->Add(credtext, 0, wxLEFT | wxRIGHT, 10);

    root->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    auto* notetext = new wxStaticText(this, wxID_ANY, "NOTE: Please ensure you have a backup of your device before applying the\njailbreak. While data loss is unlikely, we won't be responsible if something goes\nwrong. Use at your own risk.");
    wxFont notefont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    notefont.SetPointSize(12);
    notefont.SetStyle(wxFONTSTYLE_ITALIC);
    notetext->SetFont(notefont);
    root->Add(notetext, 0, wxLEFT | wxRIGHT, 10);

    root->AddStretchSpacer();

    auto* btn = new wxButton(this, wxID_ANY, "Options");
    auto* sbtn = new wxButton(this, wxID_ANY, "Start");
    sbtn->Disable();
    btn->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        frame->ShowSettings();
    });
    auto* bottomRow = new wxBoxSizer(wxHORIZONTAL);
    bottomRow->AddStretchSpacer();
    bottomRow->Add(btn, 0, wxALL, 10);
    bottomRow->Add(sbtn, 0, wxALL, 10);
    root->Add(bottomRow, 0, wxEXPAND);

    SetSizer(root);
}

SettingsPanel::SettingsPanel(MainFrame* frame, wxWindow* parent)
    : wxPanel(parent)
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

bool PalerainApp::OnInit()
{
    auto* frame = new MainFrame();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP_NO_MAIN(PalerainApp);

#endif // WITH_GUI
