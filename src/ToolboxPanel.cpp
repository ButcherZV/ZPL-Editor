#include "ToolboxPanel.h"
#include "ToolIcons.h"
#include "I18n.h"

static const ActiveTool kTools[] = {
    ActiveTool::Select, ActiveTool::Text,
    ActiveTool::Barcode, ActiveTool::Box, ActiveTool::Image
};

enum
{
    ID_BTN_SELECT  = wxID_HIGHEST + 200,
    ID_BTN_TEXT,
    ID_BTN_BARCODE,
    ID_BTN_BOX,
    ID_BTN_IMAGE,
};

wxBEGIN_EVENT_TABLE(ToolboxPanel, wxPanel)
    EVT_TOGGLEBUTTON(ID_BTN_SELECT,  ToolboxPanel::OnToggle)
    EVT_TOGGLEBUTTON(ID_BTN_TEXT,    ToolboxPanel::OnToggle)
    EVT_TOGGLEBUTTON(ID_BTN_BARCODE, ToolboxPanel::OnToggle)
    EVT_TOGGLEBUTTON(ID_BTN_BOX,     ToolboxPanel::OnToggle)
    EVT_TOGGLEBUTTON(ID_BTN_IMAGE,   ToolboxPanel::OnToggle)
wxEND_EVENT_TABLE()

ToolboxPanel::ToolboxPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddSpacer(6);

    const wxString kTooltips[] = {
        TR(TOOLTIP_SELECT), TR(TOOLTIP_TEXT), TR(TOOLTIP_BARCODE),
        TR(TOOLTIP_BOX),    TR(TOOLTIP_IMAGE)
    };

    const int ids[]  = { ID_BTN_SELECT, ID_BTN_TEXT, ID_BTN_BARCODE,
                         ID_BTN_BOX,    ID_BTN_IMAGE };
    const int iconSz = 24;
    const int btnSz  = 42;

    for (int i = 0; i < 5; ++i)
    {
        wxBitmap icon = CreateToolIcon(kTools[i], iconSz);
        m_buttons[i]  = new wxToggleButton(this, ids[i], wxEmptyString,
                                           wxDefaultPosition, wxSize(btnSz, btnSz));
        m_buttons[i]->SetBitmap(icon);
        m_buttons[i]->SetToolTip(kTooltips[i]);
        sizer->Add(m_buttons[i], 0, wxALL | wxALIGN_CENTER, 3);
    }
    m_buttons[0]->SetValue(true);  // Select is default
    SetSizer(sizer);
}

void ToolboxPanel::SetActiveTool(ActiveTool tool)
{
    m_current = tool;
    for (int i = 0; i < 5; ++i)
        m_buttons[i]->SetValue(kTools[i] == tool);
}

void ToolboxPanel::SetToolChangedCallback(std::function<void(ActiveTool)> cb)
{
    m_callback = std::move(cb);
}

void ToolboxPanel::RefreshLanguage()
{
    const wxString tips[] = {
        TR(TOOLTIP_SELECT), TR(TOOLTIP_TEXT), TR(TOOLTIP_BARCODE),
        TR(TOOLTIP_BOX),    TR(TOOLTIP_IMAGE)
    };
    for (int i = 0; i < 5; ++i)
        m_buttons[i]->SetToolTip(tips[i]);
}

void ToolboxPanel::OnToggle(wxCommandEvent& evt)
{
    int id = evt.GetId();
    for (int i = 0; i < 5; ++i)
    {
        if (m_buttons[i]->GetId() == id)
        {
            m_current = kTools[i];
            m_buttons[i]->SetValue(true);  // keep depressed
        }
        else
        {
            m_buttons[i]->SetValue(false);
        }
    }
    if (m_callback)
        m_callback(m_current);
}

