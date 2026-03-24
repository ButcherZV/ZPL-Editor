#include "ZPLCodePanel.h"

ZPLCodePanel::ZPLCodePanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    m_text = new wxTextCtrl(this, wxID_ANY, "",
                wxDefaultPosition, wxDefaultSize,
                wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxHSCROLL);
    m_text->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE,
                           wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    sizer->Add(m_text, 1, wxEXPAND);
    SetSizer(sizer);
}

void ZPLCodePanel::SetZPL(const wxString& zpl)
{
    m_text->SetValue(zpl);
}
