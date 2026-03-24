#pragma once
#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

class LabelCanvas;
class LabelElement;

class PropertiesPanel : public wxPanel
{
public:
    PropertiesPanel(wxWindow* parent, LabelCanvas* canvas);

    void ShowElement(LabelElement* el);
    void RefreshUnits();

private:
    void PopulateCommon (LabelElement* el);
    void PopulateText   (LabelElement* el);
    void PopulateBarcode(LabelElement* el);
    void PopulateBox    (LabelElement* el);
    void PopulateImage  (LabelElement* el);

    void OnPropertyChanged(wxPropertyGridEvent&);

    wxPropertyGrid* m_grid    = nullptr;
    LabelCanvas*    m_canvas  = nullptr;
    LabelElement*   m_element = nullptr;

    wxDECLARE_EVENT_TABLE();
};

