#pragma once
#include <wx/wx.h>
#include <wx/dialog.h>

class AppOptionsDialog : public wxDialog
{
public:
    explicit AppOptionsDialog(wxWindow* parent);

private:
    void OnOK(wxCommandEvent&);

    wxRadioButton* m_metricBtn   = nullptr;
    wxRadioButton* m_imperialBtn = nullptr;

    wxDECLARE_EVENT_TABLE();
};
