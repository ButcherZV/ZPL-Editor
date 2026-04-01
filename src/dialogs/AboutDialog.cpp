#include "AboutDialog.h"
#include "../I18n.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/button.h>
#include <wx/statline.h>
#include <wx/font.h>

AboutDialog::AboutDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, TR(ABOUT_TITLE),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // App name (large)
    auto* appName = new wxStaticText(this, wxID_ANY, wxT("ZPL Editor"));
    {
        wxFont f = appName->GetFont();
        f.SetPointSize(f.GetPointSize() + 6);
        f.SetWeight(wxFONTWEIGHT_BOLD);
        appName->SetFont(f);
    }

    // Version
    auto* version = new wxStaticText(
        this, wxID_ANY,
        TR(ABOUT_VERSION) + wxT(": 0.1.0"));

    // Author
    auto* author = new wxStaticText(
        this, wxID_ANY,
        TR(ABOUT_AUTHOR) + wxT(": ButcherZV"));

    // Description
    auto* desc = new wxStaticText(this, wxID_ANY, TR(ABOUT_DESC));

    // GitHub link
    auto* linkLabel = new wxStaticText(this, wxID_ANY, TR(ABOUT_GITHUB) + wxT(":"));
    auto* link = new wxHyperlinkCtrl(
        this, wxID_ANY,
        wxT("https://github.com/ButcherZV/ZPL-Editor"),
        wxT("https://github.com/ButcherZV/ZPL-Editor"));

    auto* sep = new wxStaticLine(this);
    auto* close = new wxButton(this, wxID_CLOSE, _("Close"));
    close->SetDefault();
    close->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxID_CLOSE); });

    const int pad = 10;
    sizer->AddSpacer(pad);
    sizer->Add(appName,   0, wxALIGN_CENTER | wxLEFT | wxRIGHT, pad * 3);
    sizer->AddSpacer(6);
    sizer->Add(version,   0, wxALIGN_CENTER | wxLEFT | wxRIGHT, pad * 3);
    sizer->AddSpacer(2);
    sizer->Add(author,    0, wxALIGN_CENTER | wxLEFT | wxRIGHT, pad * 3);
    sizer->AddSpacer(10);
    sizer->Add(desc,      0, wxALIGN_CENTER | wxLEFT | wxRIGHT, pad * 3);
    sizer->AddSpacer(10);
    sizer->Add(linkLabel, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, pad * 3);
    sizer->AddSpacer(2);
    sizer->Add(link,      0, wxALIGN_CENTER | wxLEFT | wxRIGHT, pad * 3);
    sizer->AddSpacer(pad);
    sizer->Add(sep,       0, wxEXPAND | wxLEFT | wxRIGHT, pad);
    sizer->AddSpacer(6);
    sizer->Add(close,     0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, pad);

    SetSizerAndFit(sizer);
    SetMinSize(GetSize());
    CentreOnParent();
}
