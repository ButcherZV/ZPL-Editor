#pragma once
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include "zpl/ZPLTypes.h"
#include "I18n.h"

class NewLabelDialog : public wxDialog
{
public:
    explicit NewLabelDialog(wxWindow* parent);

    LabelConfig GetConfig() const { return m_config; }

private:
    void PopulatePrinters();
    void UpdateDPIFromPrinter(const wxString& printerName);
    void UpdatePreview();
    double GetFieldValueInDots(wxTextCtrl* field, wxChoice* unitChoice) const;

    int  GetSelectedDPI() const;
    void DrawLabelPreview(wxPaintEvent&);

    void OnPrinterSelected(wxCommandEvent&);
    void OnValueChanged(wxCommandEvent&);
    void OnDPIChanged(wxCommandEvent&);
    void OnOrientationChanged(wxCommandEvent&);
    void OnCreate(wxCommandEvent&);

    // Controls
    wxChoice*     m_printerChoice    = nullptr;
    wxChoice*     m_dpiChoice        = nullptr;
    wxTextCtrl*   m_widthField       = nullptr;
    wxTextCtrl*   m_heightField      = nullptr;
    wxChoice*     m_sizeUnitChoice   = nullptr;
    wxTextCtrl*   m_marginTop        = nullptr;
    wxTextCtrl*   m_marginBottom     = nullptr;
    wxTextCtrl*   m_marginLeft       = nullptr;
    wxTextCtrl*   m_marginRight      = nullptr;
    wxChoice*     m_marginUnitChoice = nullptr;
    wxSpinCtrl*   m_labelsPerRow     = nullptr;
    wxRadioBox*   m_orientationBox   = nullptr;
    wxStaticText* m_previewLabel     = nullptr;
    wxPanel*      m_previewPanel     = nullptr;

    LabelConfig m_config;

    wxDECLARE_EVENT_TABLE();
};
