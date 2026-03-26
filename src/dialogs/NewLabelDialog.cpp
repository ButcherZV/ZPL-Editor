#include "NewLabelDialog.h"
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/dcbuffer.h>
#include <algorithm>
#include <climits>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <winspool.h>
#include <vector>
#endif

enum
{
    ID_PRINTER_CHOICE = wxID_HIGHEST + 100,
    ID_DPI_CHOICE,
    ID_ORIENTATION,
    ID_SIZE_FIELD,
    ID_MARGIN_FIELD,
    ID_CREATE_BTN,
};

wxBEGIN_EVENT_TABLE(NewLabelDialog, wxDialog)
    EVT_CHOICE  (ID_PRINTER_CHOICE, NewLabelDialog::OnPrinterSelected)
    EVT_CHOICE  (ID_DPI_CHOICE,     NewLabelDialog::OnDPIChanged)
    EVT_TEXT    (ID_SIZE_FIELD,     NewLabelDialog::OnValueChanged)
    EVT_TEXT    (ID_MARGIN_FIELD,   NewLabelDialog::OnValueChanged)
    EVT_RADIOBOX(ID_ORIENTATION,    NewLabelDialog::OnOrientationChanged)
    EVT_BUTTON  (ID_CREATE_BTN,     NewLabelDialog::OnCreate)
wxEND_EVENT_TABLE()

NewLabelDialog::NewLabelDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, TR(NEWLABEL_TITLE),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // ── Printer selection ─────────────────────────────────────────────────────
    auto* printerBox = new wxStaticBoxSizer(wxVERTICAL,
                           this, TR(NEWLABEL_PRINTER_RES));
    {
        auto* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, TR(NEWLABEL_PRINTER)),
                 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        m_printerChoice = new wxChoice(this, ID_PRINTER_CHOICE,
                                       wxDefaultPosition, wxSize(220, -1));
        row->Add(m_printerChoice, 1, wxALIGN_CENTER_VERTICAL);
        printerBox->Add(row, 0, wxEXPAND | wxALL, 4);
    }
    {
        auto* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, TR(NEWLABEL_DPI)),
                 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        wxArrayString dpiItems;
        dpiItems.Add("152 DPI"); dpiItems.Add("203 DPI");
        dpiItems.Add("300 DPI"); dpiItems.Add("600 DPI");
        m_dpiChoice = new wxChoice(this, ID_DPI_CHOICE,
                          wxDefaultPosition, wxDefaultSize, dpiItems);
        m_dpiChoice->SetSelection(1); // 203 DPI default
        row->Add(m_dpiChoice, 0, wxALIGN_CENTER_VERTICAL);
        printerBox->Add(row, 0, wxEXPAND | wxALL, 4);
    }
    mainSizer->Add(printerBox, 0, wxEXPAND | wxALL, 8);

    // ── Label size ────────────────────────────────────────────────────────────
    auto* sizeBox = new wxStaticBoxSizer(wxVERTICAL, this, TR(NEWLABEL_LABEL_SIZE));
    {
        auto* units = new wxBoxSizer(wxHORIZONTAL);
        units->Add(new wxStaticText(this, wxID_ANY, TR(NEWLABEL_UNITS)),
                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        wxArrayString unitChoices;
        unitChoices.Add(TR(NEWLABEL_MM));
        unitChoices.Add(TR(NEWLABEL_INCHES));
        unitChoices.Add(TR(NEWLABEL_DOTS));
        m_sizeUnitChoice = new wxChoice(this, wxID_ANY,
                               wxDefaultPosition, wxDefaultSize, unitChoices);
        m_sizeUnitChoice->SetSelection(0);
        m_sizeUnitChoice->Bind(wxEVT_CHOICE,
            [this](wxCommandEvent&){ UpdatePreview(); });
        units->Add(m_sizeUnitChoice, 0, wxALIGN_CENTER_VERTICAL);
        sizeBox->Add(units, 0, wxEXPAND | wxALL, 4);
    }
    {
        auto* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, TR(NEWLABEL_WIDTH)),
                 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        m_widthField = new wxTextCtrl(this, ID_SIZE_FIELD, "101.6",
                           wxDefaultPosition, wxSize(80, -1));
        row->Add(m_widthField, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
        row->Add(new wxStaticText(this, wxID_ANY, TR(NEWLABEL_HEIGHT)),
                 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        m_heightField = new wxTextCtrl(this, ID_SIZE_FIELD, "152.4",
                            wxDefaultPosition, wxSize(80, -1));
        row->Add(m_heightField, 0, wxALIGN_CENTER_VERTICAL);
        sizeBox->Add(row, 0, wxEXPAND | wxALL, 4);
    }
    mainSizer->Add(sizeBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // ── Margins ───────────────────────────────────────────────────────────────
    auto* marginBox = new wxStaticBoxSizer(wxVERTICAL, this, TR(NEWLABEL_MARGINS));
    {
        auto* units = new wxBoxSizer(wxHORIZONTAL);
        units->Add(new wxStaticText(this, wxID_ANY, TR(NEWLABEL_UNITS)),
                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        wxArrayString unitChoices;
        unitChoices.Add(TR(NEWLABEL_MM));
        unitChoices.Add(TR(NEWLABEL_INCHES));
        unitChoices.Add(TR(NEWLABEL_DOTS));
        m_marginUnitChoice = new wxChoice(this, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize, unitChoices);
        m_marginUnitChoice->SetSelection(0);
        m_marginUnitChoice->Bind(wxEVT_CHOICE,
            [this](wxCommandEvent&){ UpdatePreview(); });
        units->Add(m_marginUnitChoice, 0, wxALIGN_CENTER_VERTICAL);
        marginBox->Add(units, 0, wxEXPAND | wxALL, 4);
    }
    {
        auto* grid = new wxFlexGridSizer(2, 4, 6, 12);
        auto addMarginField = [&](const wxString& label, wxTextCtrl*& field,
                                  const wxString& def)
        {
            grid->Add(new wxStaticText(this, wxID_ANY, label),
                      0, wxALIGN_CENTER_VERTICAL);
            field = new wxTextCtrl(this, ID_MARGIN_FIELD, def,
                        wxDefaultPosition, wxSize(60, -1));
            grid->Add(field, 0, wxALIGN_CENTER_VERTICAL);
        };
        addMarginField(TR(NEWLABEL_TOP),    m_marginTop,    "0");
        addMarginField(TR(NEWLABEL_BOTTOM), m_marginBottom, "0");
        addMarginField(TR(NEWLABEL_LEFT),   m_marginLeft,   "0");
        addMarginField(TR(NEWLABEL_RIGHT),  m_marginRight,  "0");
        marginBox->Add(grid, 0, wxEXPAND | wxALL, 4);
    }
    mainSizer->Add(marginBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // ── Orientation ───────────────────────────────────────────────────────────
    {
        wxArrayString orientChoices;
        orientChoices.Add(TR(NEWLABEL_PORTRAIT));
        orientChoices.Add(TR(NEWLABEL_LANDSCAPE));
        m_orientationBox = new wxRadioBox(this, ID_ORIENTATION, TR(NEWLABEL_ORIENTATION),
                               wxDefaultPosition, wxDefaultSize,
                               orientChoices, 2, wxRA_SPECIFY_COLS);
        mainSizer->Add(m_orientationBox, 0,
                       wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    }

    // ── Labels per row ────────────────────────────────────────────────────────
    {
        auto* layoutBox = new wxStaticBoxSizer(wxHORIZONTAL, this, TR(NEWLABEL_LAYOUT));
        layoutBox->Add(new wxStaticText(this, wxID_ANY, TR(NEWLABEL_LABELS_PER_ROW)),
                       0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
        m_labelsPerRow = new wxSpinCtrl(this, wxID_ANY, "1",
                             wxDefaultPosition, wxSize(60, -1),
                             wxSP_ARROW_KEYS, 1, 20, 1);
        m_labelsPerRow->SetToolTip(TR(NEWLABEL_LPR_TOOLTIP));
        m_labelsPerRow->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&){ UpdatePreview(); });
        layoutBox->Add(m_labelsPerRow, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
        mainSizer->Add(layoutBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    }

    // ── Preview ───────────────────────────────────────────────────────────────
    m_previewLabel = new wxStaticText(this, wxID_ANY, "---",
                         wxDefaultPosition, wxDefaultSize,
                         wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    m_previewLabel->SetForegroundColour(wxColour(0, 80, 160));
    mainSizer->Add(m_previewLabel, 0,
                   wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // ── Buttons ───────────────────────────────────────────────────────────────
    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 8);
    {
        auto* btnSizer = new wxBoxSizer(wxHORIZONTAL);
        btnSizer->AddStretchSpacer();
        auto* createBtn = new wxButton(this, ID_CREATE_BTN, TR(BTN_CREATE));
        createBtn->SetDefault();
        btnSizer->Add(createBtn, 0, wxALL, 6);
        btnSizer->Add(new wxButton(this, wxID_CANCEL, TR(BTN_CANCEL)), 0, wxALL, 6);
        mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 4);
    }

    // Wrap in horizontal layout so the visual preview sits to the right
    auto* outerSizer = new wxBoxSizer(wxHORIZONTAL);
    outerSizer->Add(mainSizer, 0, wxEXPAND);
    {
        auto* previewBox = new wxStaticBoxSizer(wxVERTICAL, this, TR(NEWLABEL_PREVIEW));
        m_previewPanel = new wxPanel(this, wxID_ANY,
                             wxDefaultPosition, wxSize(220, 260));
        m_previewPanel->SetMinSize(wxSize(200, 240));
        m_previewPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
        m_previewPanel->Bind(wxEVT_PAINT,
            [this](wxPaintEvent& e){ DrawLabelPreview(e); });
        previewBox->Add(m_previewPanel, 1, wxEXPAND | wxALL, 4);
        outerSizer->Add(previewBox, 1, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 8);
    }

    SetSizerAndFit(outerSizer);
    SetMinSize(GetSize());

    PopulatePrinters();
    UpdatePreview();
}

void NewLabelDialog::PopulatePrinters()
{
    m_printerChoice->Append(TR(NEWLABEL_MANUAL));

#ifdef _WIN32
    // Use level 4: only pPrinterName + Attributes, no DEVMODE, no security —
    // safe and never crashes even for virtual/remote printer entries.
    DWORD needed = 0, returned = 0;
    EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                  nullptr, 4, nullptr, 0, &needed, &returned);
    if (needed > 0)
    {
        std::vector<BYTE> buf(needed, 0);
        if (EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                          nullptr, 4, buf.data(), needed, &needed, &returned))
        {
            auto* info = reinterpret_cast<PRINTER_INFO_4W*>(buf.data());
            for (DWORD i = 0; i < returned; ++i)
                if (info[i].pPrinterName)
                    m_printerChoice->Append(wxString(info[i].pPrinterName));
        }
    }
#endif
    m_printerChoice->SetSelection(0);
}

void NewLabelDialog::UpdateDPIFromPrinter(const wxString& printerName)
{
#ifdef _WIN32
    if (printerName.empty() || printerName.StartsWith("("))
        return;

    HANDLE hPrinter = nullptr;
    if (!OpenPrinterW(const_cast<LPWSTR>(printerName.wc_str()), &hPrinter, nullptr))
        return;

    DWORD needed = 0;
    GetPrinterW(hPrinter, 2, nullptr, 0, &needed);
    if (needed > 0)
    {
        std::vector<BYTE> buf(needed, 0);
        if (GetPrinterW(hPrinter, 2, buf.data(), needed, &needed))
        {
            auto* info = reinterpret_cast<PRINTER_INFO_2W*>(buf.data());
            if (info && info->pDevMode)
            {
                // dmPrintQuality can be negative DMRES_* for non-ZPL printers
                int dpi = static_cast<int>(info->pDevMode->dmPrintQuality);
                if (dpi <= 0)
                    dpi = static_cast<int>(info->pDevMode->dmYResolution);
                // Select the closest common DPI
                static const int kDPIs[] = { 152, 203, 300, 600 };
                int best = 1, bestDiff = INT_MAX;
                for (int i = 0; i < 4; ++i)
                {
                    int diff = std::abs(dpi - kDPIs[i]);
                    if (diff < bestDiff) { bestDiff = diff; best = i; }
                }
                if (dpi > 50 && dpi <= 1200)
                    m_dpiChoice->SetSelection(best);
            }
        }
    }
    ClosePrinter(hPrinter);
#endif
}

double NewLabelDialog::GetFieldValueInDots(wxTextCtrl* field,
                                            wxChoice* unitChoice) const
{
    if (!field || !unitChoice || !m_dpiChoice)
        return 0.0;
    double val = 0.0;
    if (!field->GetValue().ToDouble(&val) || val <= 0.0)
        return 0.0;

    PrinterDPI dpi = static_cast<PrinterDPI>(GetSelectedDPI());

    int sel = unitChoice->GetSelection();
    if (sel == 0) return static_cast<double>(MMToDots(val, dpi));      // mm
    if (sel == 1) return static_cast<double>(InchesToDots(val, dpi));  // inches
    return val;                                                         // dots
}

void NewLabelDialog::UpdatePreview()
{
    // Guard: EVT_TEXT fires during wxTextCtrl construction before all controls exist
    if (!m_widthField || !m_heightField || !m_dpiChoice ||
        !m_sizeUnitChoice || !m_marginUnitChoice || !m_previewLabel)
        return;

    int wDots = static_cast<int>(GetFieldValueInDots(m_widthField,  m_sizeUnitChoice));
    int hDots = static_cast<int>(GetFieldValueInDots(m_heightField, m_sizeUnitChoice));

    if (wDots <= 0 || hDots <= 0)
    {
        m_previewLabel->SetLabel("---");
        return;
    }

    PrinterDPI dpi = static_cast<PrinterDPI>(GetSelectedDPI());

    double wMM = DotsToMM(wDots, dpi);
    double hMM = DotsToMM(hDots, dpi);
    double wIn = DotsToInches(wDots, dpi);
    double hIn = DotsToInches(hDots, dpi);
    int    lpr = m_labelsPerRow ? m_labelsPerRow->GetValue() : 1;

    wxString preview = wxString::Format(
        "%.1f x %.1f mm  (%.2f x %.2f in)  |  %d x %d dots",
        wMM, hMM, wIn, hIn, wDots, hDots);
    if (lpr > 1)
        preview += wxString::Format("  x%d labels across", lpr);
    m_previewLabel->SetLabel(preview);

    if (m_previewPanel)
        m_previewPanel->Refresh();
}

void NewLabelDialog::OnPrinterSelected(wxCommandEvent&)
{
    UpdateDPIFromPrinter(m_printerChoice->GetStringSelection());
    UpdatePreview();
}

void NewLabelDialog::OnDPIChanged(wxCommandEvent&)  { UpdatePreview(); }
void NewLabelDialog::OnValueChanged(wxCommandEvent&) { UpdatePreview(); }

void NewLabelDialog::OnOrientationChanged(wxCommandEvent&)
{
    // Swap width/height when switching orientation
    bool landscape = (m_orientationBox->GetSelection() == 1);
    double w = 0.0, h = 0.0;
    m_widthField->GetValue().ToDouble(&w);
    m_heightField->GetValue().ToDouble(&h);

    bool isCurrentlyLandscape = (w > h);
    if (landscape && !isCurrentlyLandscape)
    {
        m_widthField->SetValue(wxString::Format("%.4g", h));
        m_heightField->SetValue(wxString::Format("%.4g", w));
    }
    else if (!landscape && isCurrentlyLandscape)
    {
        m_widthField->SetValue(wxString::Format("%.4g", h));
        m_heightField->SetValue(wxString::Format("%.4g", w));
    }
    UpdatePreview();
}

void NewLabelDialog::OnCreate(wxCommandEvent&)
{
    int wDots = static_cast<int>(GetFieldValueInDots(m_widthField,  m_sizeUnitChoice));
    int hDots = static_cast<int>(GetFieldValueInDots(m_heightField, m_sizeUnitChoice));

    if (wDots <= 0 || hDots <= 0)
    {
        wxMessageBox("Width and height must be greater than zero.",
                     "Invalid Size", wxOK | wxICON_WARNING, this);
        return;
    }

    int raw        = GetSelectedDPI();
    PrinterDPI dpi = static_cast<PrinterDPI>(raw);

    m_config.dpi           = dpi;
    m_config.widthDots     = wDots;
    m_config.heightDots    = hDots;
    m_config.marginTop     = static_cast<int>(GetFieldValueInDots(m_marginTop,    m_marginUnitChoice));
    m_config.marginBottom  = static_cast<int>(GetFieldValueInDots(m_marginBottom, m_marginUnitChoice));
    m_config.marginLeft    = static_cast<int>(GetFieldValueInDots(m_marginLeft,   m_marginUnitChoice));
    m_config.marginRight   = static_cast<int>(GetFieldValueInDots(m_marginRight,  m_marginUnitChoice));
    m_config.orientation   = (m_orientationBox->GetSelection() == 1)
                             ? LabelOrientation::Landscape
                             : LabelOrientation::Portrait;
    m_config.labelsPerRow  = m_labelsPerRow->GetValue();

    EndModal(wxID_OK);
}

// ── GetSelectedDPI ────────────────────────────────────────────────────────────

int NewLabelDialog::GetSelectedDPI() const
{
    static const int kDPIs[] = { 152, 203, 300, 600 };
    int sel = m_dpiChoice ? m_dpiChoice->GetSelection() : 1;
    if (sel < 0 || sel > 3) sel = 1;
    return kDPIs[sel];
}

// ── DrawLabelPreview ──────────────────────────────────────────────────────────

void NewLabelDialog::DrawLabelPreview(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(m_previewPanel);
    wxSize sz = m_previewPanel->GetClientSize();

    dc.SetBackground(wxBrush(wxColour(55, 55, 55)));
    dc.Clear();

    int wDots = static_cast<int>(GetFieldValueInDots(m_widthField,  m_sizeUnitChoice));
    int hDots = static_cast<int>(GetFieldValueInDots(m_heightField, m_sizeUnitChoice));
    if (wDots <= 0 || hDots <= 0) return;

    int mTop    = static_cast<int>(GetFieldValueInDots(m_marginTop,    m_marginUnitChoice));
    int mBottom = static_cast<int>(GetFieldValueInDots(m_marginBottom, m_marginUnitChoice));
    int mLeft   = static_cast<int>(GetFieldValueInDots(m_marginLeft,   m_marginUnitChoice));
    int mRight  = static_cast<int>(GetFieldValueInDots(m_marginRight,  m_marginUnitChoice));
    int lpr     = m_labelsPerRow ? m_labelsPerRow->GetValue() : 1;

    const int pad  = 12;
    int availW = sz.x - 2 * pad;
    int availH = sz.y - 2 * pad;
    if (availW <= 0 || availH <= 0) return;

    double scale = std::min(static_cast<double>(availW) / wDots,
                            static_cast<double>(availH) / hDots);

    int lpx = static_cast<int>(wDots * scale);
    int hpx = static_cast<int>(hDots * scale);
    int ox  = pad + (availW - lpx) / 2;
    int oy  = pad + (availH - hpx) / 2;

    // Drop shadow
    dc.SetBrush(wxBrush(wxColour(25, 25, 25)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(ox + 4, oy + 4, lpx, hpx);

    // Label paper (white)
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(wxPen(wxColour(80, 80, 80), 1));
    dc.DrawRectangle(ox, oy, lpx, hpx);

    // Printable area (light blue) inside margins
    int mlpx = static_cast<int>(mLeft   * scale);
    int mrpx = static_cast<int>(mRight  * scale);
    int mtpx = static_cast<int>(mTop    * scale);
    int mbpx = static_cast<int>(mBottom * scale);
    int px = ox + mlpx;
    int py = oy + mtpx;
    int pw = lpx - mlpx - mrpx;
    int ph = hpx - mtpx - mbpx;

    if (pw > 2 && ph > 2)
    {
        dc.SetBrush(wxBrush(wxColour(215, 230, 255)));
        dc.SetPen(wxPen(wxColour(120, 155, 210), 1, wxPENSTYLE_DOT));
        dc.DrawRectangle(px, py, pw, ph);

        // Multi-label column dividers
        if (lpr > 1)
        {
            dc.SetPen(wxPen(wxColour(190, 80, 80), 1, wxPENSTYLE_SHORT_DASH));
            int colW = pw / lpr;
            for (int i = 1; i < lpr; ++i)
            {
                int x = px + i * colW;
                dc.DrawLine(x, py, x, py + ph);
            }
        }
    }

    // Dimension annotation at bottom of label
    PrinterDPI pdpi = static_cast<PrinterDPI>(GetSelectedDPI());
    double wmm = DotsToMM(wDots, pdpi);
    double hmm = DotsToMM(hDots, pdpi);
    wxString dims = wxString::Format("%.1fx%.1fmm", wmm, hmm);
    dc.SetFont(wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetTextForeground(wxColour(50, 50, 50));
    int tw, th;
    dc.GetTextExtent(dims, &tw, &th);
    if (tw < lpx - 4)
        dc.DrawText(dims, ox + (lpx - tw) / 2, oy + hpx - th - 3);
}
