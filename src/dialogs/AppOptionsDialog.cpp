#include "AppOptionsDialog.h"
#include "AppConfig.h"
#include <wx/statline.h>

wxBEGIN_EVENT_TABLE(AppOptionsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, AppOptionsDialog::OnOK)
wxEND_EVENT_TABLE()

AppOptionsDialog::AppOptionsDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Options",
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE)
{
    const AppConfig& cfg = AppConfig::Get();

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    auto* unitBox = new wxStaticBoxSizer(wxVERTICAL, this, "Measurement Units");

    m_metricBtn   = new wxRadioButton(this, wxID_ANY, "Metric (millimetres)",
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_imperialBtn = new wxRadioButton(this, wxID_ANY, "Imperial (inches)");

    m_metricBtn->SetValue(cfg.units == MeasureUnit::Metric);
    m_imperialBtn->SetValue(cfg.units == MeasureUnit::Imperial);

    unitBox->Add(m_metricBtn,   0, wxALL, 4);
    unitBox->Add(m_imperialBtn, 0, wxALL, 4);

    mainSizer->Add(unitBox, 0, wxEXPAND | wxALL, 8);
    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 8);
    mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);

    SetSizerAndFit(mainSizer);
}

void AppOptionsDialog::OnOK(wxCommandEvent&)
{
    AppConfig::Get().units = m_metricBtn->GetValue()
                             ? MeasureUnit::Metric
                             : MeasureUnit::Imperial;
    EndModal(wxID_OK);
}
