#include "MainFrame.h"
#include "LabelCanvas.h"
#include "ToolboxPanel.h"
#include "PropertiesPanel.h"
#include "ZPLCodePanel.h"
#include <wx/config.h>
#include "AppConfig.h"
#include "I18n.h"
#include "dialogs/NewLabelDialog.h"
#include "dialogs/AppOptionsDialog.h"
#include "zpl/ZPLSerializer.h"
#include "zpl/ZPLParser.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/choicdlg.h>
#ifdef __WINDOWS__
#include <windows.h>
#include <winspool.h>
#endif

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_NEW,         MainFrame::OnNew)
    EVT_MENU(ID_OPEN,        MainFrame::OnOpen)
    EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, MainFrame::OnRecentFile)
    EVT_MENU(ID_SAVE,        MainFrame::OnSave)
    EVT_MENU(ID_SAVEAS,      MainFrame::OnSaveAs)
    EVT_MENU(ID_PRINT,       MainFrame::OnPrint)
    EVT_MENU(ID_EXIT,        MainFrame::OnExit)
    EVT_MENU(ID_UNDO,        MainFrame::OnUndo)
    EVT_MENU(ID_REDO,        MainFrame::OnRedo)
    EVT_MENU(ID_DUPLICATE,   MainFrame::OnDuplicate)
    EVT_MENU_RANGE(ID_ALIGN_LEFT, ID_ALIGN_CENTERV, MainFrame::OnAlignElement)
    EVT_MENU(ID_OPTIONS,     MainFrame::OnOptions)
    EVT_MENU(ID_SNAP_GRID,   MainFrame::OnSnapGrid)
    EVT_UPDATE_UI(ID_UNDO,   MainFrame::OnUpdateUndo)
    EVT_UPDATE_UI(ID_REDO,   MainFrame::OnUpdateRedo)
    EVT_UPDATE_UI_RANGE(ID_ALIGN_LEFT, ID_ALIGN_CENTERV, MainFrame::OnUpdateAlignElement)
    EVT_UPDATE_UI(ID_SNAP_GRID, MainFrame::OnUpdateSnapGrid)
    EVT_MENU(ID_TOGGLE_ZPL,  MainFrame::OnToggleZPL)
    EVT_MENU(ID_ZOOM_IN,     MainFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT,    MainFrame::OnZoomOut)
    EVT_MENU(ID_ZOOM_FIT,    MainFrame::OnZoomFit)
    EVT_MENU_RANGE(ID_LANG_ENGLISH, ID_LANG_SERBIAN, MainFrame::OnLanguage)
    EVT_CLOSE(               MainFrame::OnClose)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, TR(APP_TITLE),
              wxDefaultPosition, wxSize(1280, 800))
{
    m_auiMgr.SetManagedWindow(this);

    m_fileHistory.Load(*wxConfig::Get());

    BuildMenuBar();
    BuildToolBar();
    BuildPanels();

    // Bind canvas-changed event (fires when elements are added/moved/deleted)
    Bind(wxEVT_CANVAS_CHANGED, &MainFrame::OnCanvasChanged, this);
    // Track mouse position for cursor status bar display
    m_canvas->Bind(wxEVT_MOTION, &MainFrame::OnCanvasMouseMove, this);

    m_auiMgr.Update();
    Centre();
}

MainFrame::~MainFrame()
{
    m_auiMgr.UnInit();
}

void MainFrame::BuildMenuBar()
{
    auto* mb = new wxMenuBar();

    // File
    auto* file     = new wxMenu();
    auto* recentSub = new wxMenu();
    if (m_recentMenu)
        m_fileHistory.RemoveMenu(m_recentMenu);
    m_recentMenu = recentSub;
    m_fileHistory.UseMenu(recentSub);
    m_fileHistory.AddFilesToMenu();

    file->Append(ID_NEW,    TR(MENU_NEW));
    file->Append(ID_OPEN,   TR(MENU_OPEN));
    file->AppendSubMenu(recentSub, TR(MENU_RECENT_FILES));
    file->AppendSeparator();
    file->Append(ID_SAVE,   TR(MENU_SAVE));
    file->Append(ID_SAVEAS, TR(MENU_SAVE_AS));
    file->AppendSeparator();
    file->Append(ID_PRINT,  TR(MENU_PRINT));
    file->AppendSeparator();
    file->Append(ID_EXIT,   TR(MENU_EXIT));
    mb->Append(file, TR(MENU_FILE));

    // Edit
    auto* edit  = new wxMenu();
    auto* align = new wxMenu();
    align->Append(ID_ALIGN_LEFT,    TR(MENU_ALIGN_LEFT));
    align->Append(ID_ALIGN_RIGHT,   TR(MENU_ALIGN_RIGHT));
    align->Append(ID_ALIGN_TOP,     TR(MENU_ALIGN_TOP));
    align->Append(ID_ALIGN_BOTTOM,  TR(MENU_ALIGN_BOTTOM));
    align->AppendSeparator();
    align->Append(ID_ALIGN_CENTERH, TR(MENU_ALIGN_CENTERH));
    align->Append(ID_ALIGN_CENTERV, TR(MENU_ALIGN_CENTERV));

    edit->Append(ID_UNDO,      TR(MENU_UNDO));
    edit->Append(ID_REDO,      TR(MENU_REDO));
    edit->AppendSeparator();
    edit->Append(ID_DUPLICATE, TR(MENU_DUPLICATE));
    edit->AppendSubMenu(align, TR(MENU_ALIGN));
    edit->AppendSeparator();
    edit->Append(ID_OPTIONS,   TR(MENU_OPTIONS));
    mb->Append(edit, TR(MENU_EDIT));

    // View
    auto* view = new wxMenu();
    view->Append(ID_ZOOM_IN,     TR(MENU_ZOOM_IN));
    view->Append(ID_ZOOM_OUT,    TR(MENU_ZOOM_OUT));
    view->Append(ID_ZOOM_FIT,    TR(MENU_ZOOM_FIT));
    view->AppendSeparator();
    view->AppendCheckItem(ID_SNAP_GRID,   TR(MENU_SNAP_GRID));
    view->AppendSeparator();
    view->AppendCheckItem(ID_TOGGLE_ZPL, TR(MENU_SHOW_ZPL));

    auto* langMenu = new wxMenu();
    langMenu->Append(ID_LANG_ENGLISH, TR(STR_LANG_ENGLISH));
    langMenu->Append(ID_LANG_SERBIAN, TR(STR_LANG_SERBIAN));
    view->AppendSeparator();
    view->AppendSubMenu(langMenu, TR(MENU_LANGUAGE));

    mb->Append(view, TR(MENU_VIEW));

    SetMenuBar(mb);
}

void MainFrame::BuildToolBar()
{
    CreateStatusBar(3);
    SetStatusText(TR(STATUS_READY));
    SetStatusText(TR(STATUS_NO_LABEL), 1);
}

void MainFrame::BuildPanels()
{
    // Centre: canvas
    m_canvas = new LabelCanvas(this);
    m_auiMgr.AddPane(m_canvas,
        wxAuiPaneInfo()
            .Name("canvas").Caption(TR(PANEL_CANVAS))
            .CenterPane().PaneBorder(false));

    // Left: toolbox
    m_toolbox = new ToolboxPanel(this);
    m_toolbox->SetToolChangedCallback([this](ActiveTool tool) {
        m_canvas->SetActiveTool(tool);
    });
    m_auiMgr.AddPane(m_toolbox,
        wxAuiPaneInfo()
            .Name("toolbox").Caption(TR(PANEL_TOOLBOX))
            .Left().Layer(1).BestSize(56, -1)
            .CloseButton(false).Gripper(false));

    // Right: properties — receives pointer to canvas for undo/redo
    m_properties = new PropertiesPanel(this, m_canvas);
    m_auiMgr.AddPane(m_properties,
        wxAuiPaneInfo()
            .Name("properties").Caption(TR(PANEL_PROPERTIES))
            .Right().Layer(1).BestSize(260, -1)
            .CloseButton(false).Gripper(false));

    // Bottom: ZPL code panel (hidden by default)
    m_codePanel = new ZPLCodePanel(this);
    m_auiMgr.AddPane(m_codePanel,
        wxAuiPaneInfo()
            .Name("zplcode").Caption(TR(PANEL_ZPL_CODE))
            .Bottom().Layer(0).BestSize(-1, 180)
            .Hide());
}

void MainFrame::LoadLabel(const LabelConfig& config)
{
    m_canvas->SetLabelConfig(config);

    wxString info = wxString::Format(TR(STATUS_LABEL_INFO),
        config.totalWidth(), config.totalHeight(),
        static_cast<int>(config.dpi));
    SetStatusText(info, 1);
}

// ── Menu handlers ─────────────────────────────────────────────────────────────

void MainFrame::LoadFile(const wxString& path)
{
    wxFile f;
    if (!f.Open(path, wxFile::read))
    {
        wxMessageBox(TR(ERR_OPEN_FILE) + path, TR(ERR_TITLE), wxOK | wxICON_ERROR, this);
        return;
    }
    wxString zplText;
    f.ReadAll(&zplText);

    // Ask the user which DPI this label was designed for.
    // ZPL files store all coordinates in dots and do NOT embed DPI.
    wxArrayString dpiChoices;
    dpiChoices.Add("152 DPI  (6 dots/mm)");
    dpiChoices.Add("203 DPI  (8 dots/mm)  \u2013 most common");
    dpiChoices.Add("300 DPI  (12 dots/mm)");
    dpiChoices.Add("600 DPI  (24 dots/mm)");
    wxSingleChoiceDialog dpiDlg(this,
        "ZPL files do not store DPI.\n"
        "Select the resolution the label was designed for:",
        "Printer DPI", dpiChoices);
    dpiDlg.SetSelection(1);  // default: 203
    if (dpiDlg.ShowModal() != wxID_OK)
        return;
    static const PrinterDPI kDpiMap[] = {
        PrinterDPI::DPI_152, PrinterDPI::DPI_203,
        PrinterDPI::DPI_300, PrinterDPI::DPI_600 };
    PrinterDPI dpi = kDpiMap[dpiDlg.GetSelection()];

    ZPLParser parser;
    auto result = parser.Parse(zplText.ToStdString(), dpi);
    if (!result.ok)
    {
        wxMessageBox(TR(ERR_PARSE) + result.errorMsg,
                     TR(ERR_PARSE_TITLE), wxOK | wxICON_ERROR, this);
        return;
    }

    m_currentFile = path;
    SetTitle(TR(APP_TITLE) + " \u2014 " + wxFileName(path).GetFullName());
    LoadLabel(result.config);
    m_canvas->SetElements(std::move(result.elements));
    m_codePanel->SetZPL(zplText);

    m_fileHistory.AddFileToHistory(path);
    m_fileHistory.Save(*wxConfig::Get());
}

void MainFrame::OnNew(wxCommandEvent&)
{
    NewLabelDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_currentFile.clear();
        SetTitle(TR(TITLE_NEW_LABEL));
        LoadLabel(dlg.GetConfig());
    }
}

void MainFrame::OnOpen(wxCommandEvent&)
{
    wxFileDialog dlg(this, TR(DLG_OPEN_TITLE), "", "",
        TR(FILE_FILTER_ZPL),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() != wxID_OK)
        return;

    LoadFile(dlg.GetPath());
}

void MainFrame::OnRecentFile(wxCommandEvent& evt)
{
    wxString path = m_fileHistory.GetHistoryFile(evt.GetId() - wxID_FILE1);
    if (path.empty() || !wxFileExists(path))
    {
        m_fileHistory.RemoveFileFromHistory(evt.GetId() - wxID_FILE1);
        m_fileHistory.Save(*wxConfig::Get());
        wxMessageBox(TR(ERR_FILE_NOT_FOUND) + path, TR(ERR_RECENT_FILES),
                     wxOK | wxICON_ERROR, this);
        return;
    }
    LoadFile(path);
}

void MainFrame::OnSave(wxCommandEvent& evt)
{
    if (m_currentFile.empty())
    {
        OnSaveAs(evt);
        return;
    }
    ZPLSerializer ser;
    std::string zpl = ser.Serialize(m_canvas->GetConfig(),
                                    m_canvas->GetElements());
    wxFile f;
    if (!f.Open(m_currentFile, wxFile::write))
    {
        wxMessageBox(TR(ERR_WRITE) + m_currentFile, TR(ERR_TITLE),
                     wxOK | wxICON_ERROR, this);
        return;
    }
    f.Write(zpl.c_str(), zpl.size());
    m_codePanel->SetZPL(wxString::FromUTF8(zpl));
}

void MainFrame::OnSaveAs(wxCommandEvent&)
{
    wxFileDialog dlg(this, TR(DLG_SAVE_TITLE), "", "",
        TR(FILE_FILTER_ZPL),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    m_currentFile = dlg.GetPath();
    SetTitle(TR(APP_TITLE) + " \u2014 " +
             wxFileName(m_currentFile).GetFullName());

    wxCommandEvent dummy;
    OnSave(dummy);

    m_fileHistory.AddFileToHistory(m_currentFile);
    m_fileHistory.Save(*wxConfig::Get());
}

void MainFrame::OnExit(wxCommandEvent&)
{
    Close(true);
}

void MainFrame::OnUndo(wxCommandEvent&) { m_canvas->Undo(); }
void MainFrame::OnRedo(wxCommandEvent&) { m_canvas->Redo(); }

void MainFrame::OnUpdateUndo(wxUpdateUIEvent& evt)
{
    evt.Enable(m_canvas && m_canvas->CanUndo());
}

void MainFrame::OnUpdateRedo(wxUpdateUIEvent& evt)
{
    evt.Enable(m_canvas && m_canvas->CanRedo());
}

void MainFrame::OnDuplicate(wxCommandEvent&)
{
    if (m_canvas) m_canvas->DuplicateSelected();
}

void MainFrame::OnAlignElement(wxCommandEvent& evt)
{
    if (!m_canvas) return;
    int type = evt.GetId() - ID_ALIGN_LEFT;   // 0..5
    m_canvas->AlignSelected(type);
}

void MainFrame::OnUpdateAlignElement(wxUpdateUIEvent& evt)
{
    evt.Enable(m_canvas && m_canvas->GetSelected() != nullptr &&
               m_canvas->GetConfig().totalWidth() > 0);
}

void MainFrame::OnOptions(wxCommandEvent&)
{
    AppOptionsDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        AppConfig::Get().Save();
        m_canvas->Refresh();
        m_properties->RefreshUnits();
    }
}

void MainFrame::OnSnapGrid(wxCommandEvent&)
{
    if (m_canvas) m_canvas->SetSnapToGrid(!m_canvas->GetSnapToGrid());
}

void MainFrame::OnUpdateSnapGrid(wxUpdateUIEvent& evt)
{
    evt.Check(m_canvas && m_canvas->GetSnapToGrid());
}

void MainFrame::OnToggleZPL(wxCommandEvent& evt)
{
    wxAuiPaneInfo& pane = m_auiMgr.GetPane("zplcode");
    pane.Show(evt.IsChecked());
    m_auiMgr.Update();

    // Populate ZPL text immediately when the panel is shown
    if (evt.IsChecked() && m_canvas->GetConfig().totalWidth() > 0)
    {
        ZPLSerializer ser;
        std::string zpl = ser.Serialize(m_canvas->GetConfig(),
                                        m_canvas->GetElements());
        m_codePanel->SetZPL(wxString::FromUTF8(zpl));
    }
}

void MainFrame::OnZoomIn(wxCommandEvent&)
{
    m_canvas->ZoomIn();
}

void MainFrame::OnZoomOut(wxCommandEvent&)
{
    m_canvas->ZoomOut();
}

void MainFrame::OnZoomFit(wxCommandEvent&)
{
    m_canvas->ZoomFit();
}

void MainFrame::OnClose(wxCloseEvent& evt)
{
    AppConfig::Get().Save();
    evt.Skip();
}

// ── Canvas changed (live ZPL + properties sync) ───────────────────────────────

void MainFrame::OnCanvasChanged(wxCommandEvent&)
{
    // Sync toolbox button state in case canvas reset the tool
    m_toolbox->SetActiveTool(m_canvas->GetActiveTool());

    // Update properties panel for currently selected element
    m_properties->ShowElement(m_canvas->GetSelected());

    // Zoom % + snap state in status bar field 0
    wxString snapStr = m_canvas->GetSnapToGrid()
        ? wxString::Format(TR(STATUS_SNAP_ON), m_canvas->GetSnapSize())
        : wxString(TR(STATUS_SNAP_OFF));
    SetStatusText(wxString::Format(TR(STATUS_ZOOM),
                  static_cast<int>(m_canvas->GetZoom() * 100.0), snapStr), 0);

    // Update the live ZPL code panel if it is visible
    wxAuiPaneInfo& pane = m_auiMgr.GetPane("zplcode");
    if (pane.IsShown() && m_canvas->GetConfig().totalWidth() > 0)
    {
        ZPLSerializer ser;
        std::string zpl = ser.Serialize(m_canvas->GetConfig(),
                                        m_canvas->GetElements());
        m_codePanel->SetZPL(wxString::FromUTF8(zpl));
    }
}

// ── Print (raw ZPL) ───────────────────────────────────────────────────────────

void MainFrame::OnPrint(wxCommandEvent&)
{
    if (m_canvas->GetElements().empty())
    {
        wxMessageBox(TR(PRINT_NO_ELEMENTS),
                     TR(PRINT_TITLE), wxOK | wxICON_INFORMATION, this);
        return;
    }

    // Build ZPL
    ZPLSerializer ser;
    std::string zpl = ser.Serialize(m_canvas->GetConfig(),
                                    m_canvas->GetElements());

#ifdef __WINDOWS__
    // Enumerate installed printers
    DWORD needed = 0, count = 0;
    EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                  nullptr, 4, nullptr, 0, &needed, &count);
    std::vector<BYTE> buf(needed);
    if (!EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                       nullptr, 4, buf.data(), needed, &needed, &count))
    {
        wxMessageBox(TR(PRINT_CANT_ENUM), TR(PRINT_ERR_TITLE),
                     wxOK | wxICON_ERROR, this);
        return;
    }

    wxArrayString printerNames;
    auto* info = reinterpret_cast<PRINTER_INFO_4W*>(buf.data());
    for (DWORD i = 0; i < count; ++i)
        printerNames.Add(wxString(info[i].pPrinterName));

    if (printerNames.empty())
    {
        wxMessageBox(TR(PRINT_NO_PRINTERS), TR(PRINT_TITLE), wxOK | wxICON_WARNING, this);
        return;
    }

    wxSingleChoiceDialog dlg(this, TR(PRINT_SELECT),
                             TR(PRINT_DLG_TITLE), printerNames);
    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString printerName = dlg.GetStringSelection();

    HANDLE hPrinter = nullptr;
    if (!OpenPrinterW(const_cast<LPWSTR>(printerName.wc_str()), &hPrinter, nullptr))
    {
        wxMessageBox(TR(PRINT_CANT_OPEN) + printerName,
                     TR(PRINT_ERR_TITLE), wxOK | wxICON_ERROR, this);
        return;
    }

    DOC_INFO_1A di{};
    di.pDocName  = const_cast<CHAR*>("ZPL Label");
    di.pDatatype = const_cast<CHAR*>("RAW");

    if (!StartDocPrinterA(hPrinter, 1, reinterpret_cast<LPBYTE>(&di)))
    {
        ClosePrinter(hPrinter);
        wxMessageBox(TR(PRINT_ERR_START), TR(PRINT_ERR_TITLE),
                     wxOK | wxICON_ERROR, this);
        return;
    }

    StartPagePrinter(hPrinter);
    DWORD written = 0;
    WritePrinter(hPrinter,
                 const_cast<CHAR*>(zpl.c_str()),
                 static_cast<DWORD>(zpl.size()),
                 &written);
    EndPagePrinter(hPrinter);
    EndDocPrinter(hPrinter);
    ClosePrinter(hPrinter);

    SetStatusText(wxString::Format(TR(STATUS_PRINT_SENT),
                                   zpl.size(), printerName), 1);
#else
    wxMessageBox(TR(PRINT_WIN_ONLY),
                 TR(PRINT_TITLE), wxOK | wxICON_INFORMATION, this);
#endif
}

void MainFrame::OnCanvasMouseMove(wxMouseEvent& evt)
{
    evt.Skip();
    if (!m_canvas || m_canvas->GetConfig().totalWidth() == 0)
    {
        SetStatusText("", 2);
        return;
    }
    wxPoint dot = m_canvas->PixelToDot(evt.GetPosition());
    int tw = m_canvas->GetConfig().totalWidth();
    int th = m_canvas->GetConfig().totalHeight();
    if (dot.x < 0 || dot.x > tw || dot.y < 0 || dot.y > th)
    {
        SetStatusText("", 2);
        return;
    }
    PrinterDPI dpi = m_canvas->GetConfig().dpi;
    wxString pos;
    if (AppConfig::Get().units == MeasureUnit::Metric)
        pos = wxString::Format("X: %.1f mm  Y: %.1f mm",
                               DotsToMM(dot.x, dpi), DotsToMM(dot.y, dpi));
    else
        pos = wxString::Format("X: %.3f\"  Y: %.3f\"",
                               DotsToInches(dot.x, dpi), DotsToInches(dot.y, dpi));
    SetStatusText(pos, 2);
}

void MainFrame::OnLanguage(wxCommandEvent& evt)
{
    AppLang lang = (evt.GetId() == ID_LANG_ENGLISH) ? AppLang::English : AppLang::Serbian;
    I18n::SetLanguage(lang);
    wxConfig::Get()->Write("/Language", lang == AppLang::Serbian ? "sr" : "en");

    // Rebuild menu bar with new strings
    wxMenuBar* old = GetMenuBar();
    SetMenuBar(nullptr);
    delete old;
    BuildMenuBar();

    // Refresh toolbox tooltips
    m_toolbox->RefreshLanguage();

    // Refresh AUI pane captions
    m_auiMgr.GetPane("canvas")    .Caption(TR(PANEL_CANVAS));
    m_auiMgr.GetPane("toolbox")   .Caption(TR(PANEL_TOOLBOX));
    m_auiMgr.GetPane("properties").Caption(TR(PANEL_PROPERTIES));
    m_auiMgr.GetPane("zplcode")   .Caption(TR(PANEL_ZPL_CODE));
    m_auiMgr.Update();

    // Status bar — retranslate field 1
    if (m_canvas->GetConfig().totalWidth() > 0)
    {
        const LabelConfig& cfg = m_canvas->GetConfig();
        SetStatusText(wxString::Format(TR(STATUS_LABEL_INFO),
            cfg.totalWidth(), cfg.totalHeight(),
            static_cast<int>(cfg.dpi)), 1);
    }
    else
    {
        SetStatusText(TR(STATUS_NO_LABEL), 1);
    }
    // Retranslate field 0 (zoom / snap)
    {
        wxString snapStr = m_canvas->GetSnapToGrid()
            ? wxString::Format(TR(STATUS_SNAP_ON), m_canvas->GetSnapSize())
            : wxString(TR(STATUS_SNAP_OFF));
        SetStatusText(wxString::Format(TR(STATUS_ZOOM),
            static_cast<int>(m_canvas->GetZoom() * 100.0), snapStr), 0);
    }

    // Refresh properties panel labels
    m_properties->ShowElement(m_canvas->GetSelected());
}
