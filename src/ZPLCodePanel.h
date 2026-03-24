#pragma once
#include <wx/wx.h>

class ZPLCodePanel : public wxPanel
{
public:
    explicit ZPLCodePanel(wxWindow* parent);
    void SetZPL(const wxString& zpl);

private:
    wxTextCtrl* m_text = nullptr;
};
