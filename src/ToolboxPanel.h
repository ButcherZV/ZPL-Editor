#pragma once
#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <functional>
#include "ActiveTool.h"

class ToolboxPanel : public wxPanel
{
public:
    explicit ToolboxPanel(wxWindow* parent);

    void SetActiveTool(ActiveTool tool);  // sync button state from outside
    void SetToolChangedCallback(std::function<void(ActiveTool)> cb);
    void RefreshLanguage();               // update tooltips after language change
    void SetToolsEnabled(bool enabled);   // enable/disable all tool buttons

private:
    void OnToggle(wxCommandEvent&);

    std::function<void(ActiveTool)> m_callback;
    wxToggleButton*                 m_buttons[5] = {};
    ActiveTool                      m_current    = ActiveTool::Select;

    wxDECLARE_EVENT_TABLE();
};

