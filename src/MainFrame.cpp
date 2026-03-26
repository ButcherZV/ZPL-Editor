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
#include <wx/numdlg.h>
#include "ToolIcons.h"
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
    EVT_MENU_RANGE(ID_ALIGN_LEFT, ID_ALIGN_EL_CENTERV, MainFrame::OnAlignElement)
    EVT_MENU(ID_OPTIONS,     MainFrame::OnOptions)
    EVT_MENU(ID_SNAP_GRID,   MainFrame::OnSnapGrid)
    EVT_UPDATE_UI(ID_UNDO,   MainFrame::OnUpdateUndo)
    EVT_UPDATE_UI(ID_REDO,   MainFrame::OnUpdateRedo)
    EVT_UPDATE_UI_RANGE(ID_ALIGN_LEFT, ID_ALIGN_EL_CENTERV, MainFrame::OnUpdateAlignElement)
    EVT_UPDATE_UI(ID_SNAP_GRID, MainFrame::OnUpdateSnapGrid)
    EVT_UPDATE_UI(ID_ZOOM_IN,  MainFrame::OnUpdateZoom)
    EVT_UPDATE_UI(ID_ZOOM_OUT, MainFrame::OnUpdateZoom)
    EVT_UPDATE_UI(ID_ZOOM_FIT, MainFrame::OnUpdateZoom)
    EVT_MENU(ID_TOGGLE_ZPL,          MainFrame::OnToggleZPL)
    EVT_UPDATE_UI(ID_TOGGLE_ZPL,      MainFrame::OnUpdateToggleZPL)
    EVT_MENU(ID_ZOOM_IN,     MainFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT,    MainFrame::OnZoomOut)
    EVT_MENU(ID_ZOOM_FIT,    MainFrame::OnZoomFit)
    EVT_MENU_RANGE(ID_LANG_ENGLISH, ID_LANG_SERBIAN,   MainFrame::OnLanguage)
    EVT_MENU_RANGE(ID_UNITS_METRIC, ID_UNITS_IMPERIAL, MainFrame::OnUnits)
    EVT_MENU_RANGE(ID_GRID_2,       ID_GRID_CUSTOM,    MainFrame::OnGridSize)
    EVT_CLOSE(               MainFrame::OnClose)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, TR(APP_TITLE),
              wxDefaultPosition, wxSize(1280, 800))
{
    m_auiMgr.SetManagedWindow(this);

#ifdef __WXMSW__
    SetIcon(wxIcon("APP_ICON", wxBITMAP_TYPE_ICO_RESOURCE));
#endif

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
    align->Append(ID_ALIGN_EL_CENTERH, TR(MENU_ALIGN_EL_CENTERH));
    align->Append(ID_ALIGN_EL_CENTERV, TR(MENU_ALIGN_EL_CENTERV));
    align->AppendSeparator();
    align->Append(ID_ALIGN_CENTERH, TR(MENU_ALIGN_CENTERH));
    align->Append(ID_ALIGN_CENTERV, TR(MENU_ALIGN_CENTERV));

    edit->Append(ID_UNDO,      TR(MENU_UNDO));
    edit->Append(ID_REDO,      TR(MENU_REDO));
    edit->AppendSeparator();
    edit->Append(ID_DUPLICATE, TR(MENU_DUPLICATE));
    edit->AppendSubMenu(align, TR(MENU_ALIGN));
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

    mb->Append(view, TR(MENU_VIEW));

    // Options menu — Language + Measuring Units with radio items
    auto* opts    = new wxMenu();

    auto* langSub = new wxMenu();
    langSub->AppendRadioItem(ID_LANG_ENGLISH, TR(STR_LANG_ENGLISH));
    langSub->AppendRadioItem(ID_LANG_SERBIAN, TR(STR_LANG_SERBIAN));
    langSub->Check(I18n::GetLanguage() == AppLang::Serbian ? ID_LANG_SERBIAN : ID_LANG_ENGLISH, true);
    opts->AppendSubMenu(langSub, TR(MENU_LANGUAGE));

    opts->AppendSeparator();

    auto* unitsSub = new wxMenu();
    unitsSub->AppendRadioItem(ID_UNITS_METRIC,   TR(MENU_UNITS_METRIC));
    unitsSub->AppendRadioItem(ID_UNITS_IMPERIAL, TR(MENU_UNITS_IMPERIAL));
    unitsSub->Check(AppConfig::Get().units == MeasureUnit::Metric
                    ? ID_UNITS_METRIC : ID_UNITS_IMPERIAL, true);
    opts->AppendSubMenu(unitsSub, TR(MENU_UNITS));

    opts->AppendSeparator();

    auto* gridSub = new wxMenu();
    gridSub->AppendRadioItem(ID_GRID_2,  TR(MENU_GRID_2));
    gridSub->AppendRadioItem(ID_GRID_5,  TR(MENU_GRID_5));
    gridSub->AppendRadioItem(ID_GRID_10, TR(MENU_GRID_10));
    {
        int snap = AppConfig::Get().snapSize;
        wxString customLabel = (snap != 2 && snap != 5 && snap != 10)
            ? wxString::Format(TR(MENU_GRID_CUSTOM_N), snap)
            : TR(MENU_GRID_CUSTOM);
        gridSub->AppendRadioItem(ID_GRID_CUSTOM, customLabel);
        int gridId = (snap == 2) ? ID_GRID_2 :
                     (snap == 5) ? ID_GRID_5 :
                     (snap == 10) ? ID_GRID_10 : ID_GRID_CUSTOM;
        gridSub->Check(gridId, true);
    }
    opts->AppendSubMenu(gridSub, TR(MENU_GRID_SIZE));

    mb->Append(opts, TR(MENU_OPTIONS_MENU));

    SetMenuBar(mb);
}

void MainFrame::BuildToolBar()
{
    CreateStatusBar(3);
    SetStatusText(TR(STATUS_READY));
    SetStatusText(TR(STATUS_NO_LABEL), 1);

    wxToolBar* tb = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT | wxNO_BORDER, wxID_ANY);

    tb->AddTool(ID_NEW,  TR(MENU_NEW).BeforeFirst('\t'),  BundleMainToolbarIcon(MainToolbarIcon::New),  TR(TB_NEW));
    tb->AddTool(ID_OPEN, TR(MENU_OPEN).BeforeFirst('\t'), BundleMainToolbarIcon(MainToolbarIcon::Open), TR(TB_OPEN));
    tb->AddTool(ID_SAVE, TR(MENU_SAVE).BeforeFirst('\t'), BundleMainToolbarIcon(MainToolbarIcon::Save), TR(TB_SAVE));
    tb->AddTool(ID_UNDO, TR(MENU_UNDO).BeforeFirst('\t'), BundleMainToolbarIcon(MainToolbarIcon::Undo), TR(TB_UNDO));
    tb->AddTool(ID_REDO, TR(MENU_REDO).BeforeFirst('\t'), BundleMainToolbarIcon(MainToolbarIcon::Redo), TR(TB_REDO));

    tb->AddSeparator();

    tb->AddTool(ID_ALIGN_LEFT,    TR(MENU_ALIGN_LEFT).BeforeFirst('\t'),    BundleMainToolbarIcon(MainToolbarIcon::AlignLeft),    TR(TB_ALIGN_LEFT));
    tb->AddTool(ID_ALIGN_RIGHT,   TR(MENU_ALIGN_RIGHT).BeforeFirst('\t'),   BundleMainToolbarIcon(MainToolbarIcon::AlignRight),   TR(TB_ALIGN_RIGHT));
    tb->AddTool(ID_ALIGN_TOP,     TR(MENU_ALIGN_TOP).BeforeFirst('\t'),     BundleMainToolbarIcon(MainToolbarIcon::AlignTop),     TR(TB_ALIGN_TOP));
    tb->AddTool(ID_ALIGN_BOTTOM,      TR(MENU_ALIGN_BOTTOM).BeforeFirst('\t'),      BundleMainToolbarIcon(MainToolbarIcon::AlignBottom),    TR(TB_ALIGN_BOTTOM));
    tb->AddTool(ID_ALIGN_EL_CENTERH,  TR(MENU_ALIGN_EL_CENTERH).BeforeFirst('\t'),  BundleMainToolbarIcon(MainToolbarIcon::AlignElCenterH),  TR(TB_ALIGN_EL_CENTERH));
    tb->AddTool(ID_ALIGN_EL_CENTERV,  TR(MENU_ALIGN_EL_CENTERV).BeforeFirst('\t'),  BundleMainToolbarIcon(MainToolbarIcon::AlignElCenterV),  TR(TB_ALIGN_EL_CENTERV));
    tb->AddTool(ID_ALIGN_CENTERH,     TR(MENU_ALIGN_CENTERH).BeforeFirst('\t'),     BundleMainToolbarIcon(MainToolbarIcon::AlignCenterH),    TR(TB_ALIGN_CENTERH));
    tb->AddTool(ID_ALIGN_CENTERV, TR(MENU_ALIGN_CENTERV).BeforeFirst('\t'), BundleMainToolbarIcon(MainToolbarIcon::AlignCenterV), TR(TB_ALIGN_CENTERV));

    tb->AddSeparator();

    tb->AddTool(ID_ZOOM_IN,  TR(MENU_ZOOM_IN).BeforeFirst('\t'),  BundleMainToolbarIcon(MainToolbarIcon::ZoomIn),  TR(TB_ZOOM_IN));
    tb->AddTool(ID_ZOOM_OUT, TR(MENU_ZOOM_OUT).BeforeFirst('\t'), BundleMainToolbarIcon(MainToolbarIcon::ZoomOut), TR(TB_ZOOM_OUT));
    tb->AddTool(ID_ZOOM_FIT, TR(MENU_ZOOM_FIT).BeforeFirst('\t'), BundleMainToolbarIcon(MainToolbarIcon::ZoomFit), TR(TB_ZOOM_FIT));

    tb->Realize();
}

void MainFrame::RefreshToolBar()
{
    wxToolBar* tb = GetToolBar();
    if (!tb) return;
    tb->SetToolShortHelp(ID_NEW,          TR(TB_NEW));
    tb->SetToolShortHelp(ID_OPEN,         TR(TB_OPEN));
    tb->SetToolShortHelp(ID_SAVE,         TR(TB_SAVE));
    tb->SetToolShortHelp(ID_UNDO,         TR(TB_UNDO));
    tb->SetToolShortHelp(ID_REDO,         TR(TB_REDO));
    tb->SetToolShortHelp(ID_ALIGN_LEFT,   TR(TB_ALIGN_LEFT));
    tb->SetToolShortHelp(ID_ALIGN_RIGHT,  TR(TB_ALIGN_RIGHT));
    tb->SetToolShortHelp(ID_ALIGN_TOP,    TR(TB_ALIGN_TOP));
    tb->SetToolShortHelp(ID_ALIGN_BOTTOM,     TR(TB_ALIGN_BOTTOM));
    tb->SetToolShortHelp(ID_ALIGN_EL_CENTERH, TR(TB_ALIGN_EL_CENTERH));
    tb->SetToolShortHelp(ID_ALIGN_EL_CENTERV, TR(TB_ALIGN_EL_CENTERV));
    tb->SetToolShortHelp(ID_ALIGN_CENTERH,    TR(TB_ALIGN_CENTERH));
    tb->SetToolShortHelp(ID_ALIGN_CENTERV,TR(TB_ALIGN_CENTERV));
    tb->SetToolShortHelp(ID_ZOOM_IN,      TR(TB_ZOOM_IN));
    tb->SetToolShortHelp(ID_ZOOM_OUT,     TR(TB_ZOOM_OUT));
    tb->SetToolShortHelp(ID_ZOOM_FIT,     TR(TB_ZOOM_FIT));
}

void MainFrame::BuildPanels()
{
    // Centre: canvas
    m_canvas = new LabelCanvas(this);
    m_canvas->SetSnapSize(AppConfig::Get().snapSize);
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
            .Left().Layer(1).BestSize(56, -1).MinSize(56, -1).MaxSize(56, -1)
            .CloseButton(false).Gripper(false).Resizable(false));

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
    m_toolbox->SetToolsEnabled(true);
    UpdateLabelStatus();
}

void MainFrame::UpdateTitle()
{
    wxString name = m_currentFile.empty()
        ? TR(TITLE_NEW_LABEL)
        : TR(APP_TITLE) + wxString(" - ") + wxFileName(m_currentFile).GetFullName();
    if (m_canvas && m_canvas->IsModified())
        name += wxString(" *");
    SetTitle(name);
}

bool MainFrame::PromptSaveIfDirty()
{
    if (!m_canvas || !m_canvas->IsModified()) return true;
    wxMessageDialog dlg(this, TR(UNSAVED_MSG), TR(UNSAVED_TITLE),
                        wxYES_NO | wxCANCEL | wxICON_QUESTION);
    dlg.SetYesNoCancelLabels(TR(BTN_YES), TR(BTN_NO), TR(BTN_CANCEL));
    int answer = dlg.ShowModal();
    if (answer == wxID_YES)
    {
        wxCommandEvent dummy;
        OnSave(dummy);
        return !m_canvas->IsModified();
    }
    if (answer == wxID_NO)  return true;
    return false; // wxID_CANCEL
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
    // Read as raw bytes so that UTF-8 encoded text (^CI28 / Serbian etc.) is preserved.
    // wxFile::ReadAll(wxString*) converts through the platform locale (CP1252 on Windows)
    // which would corrupt multi-byte UTF-8 sequences like Š (C5 A0).
    wxFileOffset fileSize = f.Length();
    std::string rawZpl;
    if (fileSize > 0)
    {
        rawZpl.resize(static_cast<size_t>(fileSize));
        f.Read(&rawZpl[0], static_cast<size_t>(fileSize));
    }
    wxString zplText = wxString::FromUTF8(rawZpl);

    // Ask the user which DPI this label was designed for.
    // ZPL files store all coordinates in dots and do NOT embed DPI.
    wxArrayString dpiChoices;
    dpiChoices.Add(TR(DPI_CHOICE_152));
    dpiChoices.Add(TR(DPI_CHOICE_203));
    dpiChoices.Add(TR(DPI_CHOICE_300));
    dpiChoices.Add(TR(DPI_CHOICE_600));
    wxSingleChoiceDialog dpiDlg(this, TR(DPI_DLG_MSG), TR(DPI_DLG_TITLE), dpiChoices);
    dpiDlg.SetSelection(1);  // default: 203
    if (dpiDlg.ShowModal() != wxID_OK)
        return;
    static const PrinterDPI kDpiMap[] = {
        PrinterDPI::DPI_152, PrinterDPI::DPI_203,
        PrinterDPI::DPI_300, PrinterDPI::DPI_600 };
    PrinterDPI dpi = kDpiMap[dpiDlg.GetSelection()];

    ZPLParser parser;
    auto result = parser.Parse(rawZpl, dpi);
    if (!result.ok)
    {
        wxMessageBox(TR(ERR_PARSE) + result.errorMsg,
                     TR(ERR_PARSE_TITLE), wxOK | wxICON_ERROR, this);
        return;
    }

    m_currentFile = path;
    LoadLabel(result.config);
    m_canvas->SetElements(std::move(result.elements));
    m_canvas->MarkHistoryClean();
    m_codePanel->SetZPL(zplText);
    UpdateTitle();

    m_fileHistory.AddFileToHistory(path);
    m_fileHistory.Save(*wxConfig::Get());
}

void MainFrame::OnNew(wxCommandEvent&)
{
    if (!PromptSaveIfDirty()) return;
    NewLabelDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_currentFile.clear();
        LoadLabel(dlg.GetConfig());
        m_canvas->SetElements({});
        m_canvas->MarkHistoryClean();
        UpdateTitle();
    }
}

void MainFrame::OnOpen(wxCommandEvent&)
{
    if (!PromptSaveIfDirty()) return;
    wxFileDialog dlg(this, TR(DLG_OPEN_TITLE), "", "",
        TR(FILE_FILTER_ZPL),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() != wxID_OK)
        return;

    LoadFile(dlg.GetPath());
}

void MainFrame::OnRecentFile(wxCommandEvent& evt)
{
    if (!PromptSaveIfDirty()) return;
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
    // Ensure the file has a recognised extension (.zpl or .prn).
    // If the current extension is neither, default to .zpl.
    wxFileName fn(m_currentFile);
    wxString ext = fn.GetExt().Lower();
    if (ext != "zpl" && ext != "prn")
    {
        fn.SetExt("zpl");
        m_currentFile = fn.GetFullPath();
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
    m_canvas->MarkHistoryClean();
    UpdateTitle();
    m_codePanel->SetZPL(wxString::FromUTF8(zpl));
}

void MainFrame::OnSaveAs(wxCommandEvent&)
{
    // Filter: index 0 = ZPL, index 1 = PRN, index 2 = All files
    int filterIndex = 0;  // default: .zpl
    if (!m_currentFile.empty())
    {
        wxString ext = wxFileName(m_currentFile).GetExt().Lower();
        if (ext == "prn") filterIndex = 1;
    }

    wxFileDialog dlg(this, TR(DLG_SAVE_TITLE),
                     m_currentFile.empty() ? wxString() : wxFileName(m_currentFile).GetPath(),
                     m_currentFile.empty() ? wxString() : wxFileName(m_currentFile).GetFullName(),
                     TR(FILE_FILTER_ZPL_SAVE),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    dlg.SetFilterIndex(filterIndex);

    if (dlg.ShowModal() != wxID_OK)
        return;

    m_currentFile = dlg.GetPath();

    // Enforce the correct extension based on which filter was chosen.
    wxFileName fn(m_currentFile);
    int chosen = dlg.GetFilterIndex();
    if (chosen == 0 && fn.GetExt().Lower() != "zpl")
        fn.SetExt("zpl");
    else if (chosen == 1 && fn.GetExt().Lower() != "prn")
        fn.SetExt("prn");
    m_currentFile = fn.GetFullPath();

    wxCommandEvent dummy;
    OnSave(dummy);
    // UpdateTitle() already done inside OnSave on success

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
    if (!m_canvas || m_canvas->GetConfig().totalWidth() == 0)
    {
        evt.Enable(false);
        return;
    }
    const auto& sel = m_canvas->GetSelection();
    // Center H/V work on a single item; all edge-align tools need 2+.
    const bool isCentre = (evt.GetId() == ID_ALIGN_CENTERH ||
                           evt.GetId() == ID_ALIGN_CENTERV);
    evt.Enable(isCentre ? !sel.empty() : sel.size() >= 2);
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
    if (m_canvas) { m_canvas->SetSnapToGrid(!m_canvas->GetSnapToGrid()); UpdateZoomStatus(); }
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

void MainFrame::OnUpdateToggleZPL(wxUpdateUIEvent& evt)
{
    evt.Check(m_auiMgr.GetPane("zplcode").IsShown());
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

void MainFrame::OnUpdateZoom(wxUpdateUIEvent& evt)
{
    evt.Enable(m_canvas && m_canvas->GetConfig().totalWidth() > 0);
}

void MainFrame::OnClose(wxCloseEvent& evt)
{
    if (evt.CanVeto() && m_canvas && m_canvas->IsModified())
    {
        if (!PromptSaveIfDirty())
        {
            evt.Veto();
            return;
        }
    }
    AppConfig::Get().Save();
    evt.Skip();
}

// ── Canvas changed (live ZPL + properties sync) ───────────────────────────────

void MainFrame::OnCanvasChanged(wxCommandEvent&)
{
    // Update title bar (* indicator) whenever canvas notifies us
    if (m_canvas->GetConfig().totalWidth() > 0)
        UpdateTitle();

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

    // Refresh main toolbar tooltips
    RefreshToolBar();

    // Refresh AUI pane captions
    m_auiMgr.GetPane("canvas")    .Caption(TR(PANEL_CANVAS));
    m_auiMgr.GetPane("toolbox")   .Caption(TR(PANEL_TOOLBOX));
    m_auiMgr.GetPane("properties").Caption(TR(PANEL_PROPERTIES));
    m_auiMgr.GetPane("zplcode")   .Caption(TR(PANEL_ZPL_CODE));
    m_auiMgr.Update();

    // Status bar — retranslate field 1
    if (m_canvas->GetConfig().totalWidth() > 0)
        UpdateLabelStatus();
    else
        SetStatusText(TR(STATUS_NO_LABEL), 1);
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

void MainFrame::OnUnits(wxCommandEvent& evt)
{
    AppConfig::Get().units = (evt.GetId() == ID_UNITS_METRIC)
                             ? MeasureUnit::Metric
                             : MeasureUnit::Imperial;
    AppConfig::Get().Save();
    if (m_properties)
        m_properties->RefreshUnits();
    UpdateLabelStatus();
}

void MainFrame::UpdateZoomStatus()
{
    if (!m_canvas) return;
    wxString snapStr = m_canvas->GetSnapToGrid()
        ? wxString::Format(TR(STATUS_SNAP_ON), m_canvas->GetSnapSize())
        : wxString(TR(STATUS_SNAP_OFF));
    SetStatusText(wxString::Format(TR(STATUS_ZOOM),
        static_cast<int>(m_canvas->GetZoom() * 100.0), snapStr), 0);
}

void MainFrame::UpdateLabelStatus()
{
    const LabelConfig& cfg = m_canvas->GetConfig();
    if (cfg.totalWidth() <= 0)
    {
        SetStatusText(TR(STATUS_NO_LABEL), 1);
        return;
    }

    bool metric = (AppConfig::Get().units == MeasureUnit::Metric);
    wxString unitStr;
    if (metric)
    {
        double wMM = DotsToMM(cfg.totalWidth(),  cfg.dpi);
        double hMM = DotsToMM(cfg.totalHeight(), cfg.dpi);
        unitStr = wxString::Format(" (%.1f x %.1f mm)", wMM, hMM);
    }
    else
    {
        double wIn = DotsToInches(cfg.totalWidth(),  cfg.dpi);
        double hIn = DotsToInches(cfg.totalHeight(), cfg.dpi);
        unitStr = wxString::Format(" (%.2f x %.2f in)", wIn, hIn);
    }

    SetStatusText(
        wxString::Format(TR(STATUS_LABEL_INFO),
            cfg.totalWidth(), cfg.totalHeight(),
            static_cast<int>(cfg.dpi)) + unitStr,
        1);
}

void MainFrame::OnGridSize(wxCommandEvent& evt)
{
    int newSize = AppConfig::Get().snapSize;
    if (evt.GetId() == ID_GRID_2)       newSize = 2;
    else if (evt.GetId() == ID_GRID_5)  newSize = 5;
    else if (evt.GetId() == ID_GRID_10) newSize = 10;
    else // ID_GRID_CUSTOM
    {
        long val = wxGetNumberFromUser(
            TR(MENU_GRID_DLG_MSG),
            wxEmptyString,
            TR(MENU_GRID_DLG_TITLE),
            AppConfig::Get().snapSize,
            1, 500, this);
        if (val < 0) return; // user cancelled
        newSize = static_cast<int>(val);
    }
    AppConfig::Get().snapSize = newSize;
    AppConfig::Get().Save();
    m_canvas->SetSnapSize(newSize);
    UpdateZoomStatus();
    // Rebuild menu to update the Custom label if needed
    wxMenuBar* old = GetMenuBar();
    SetMenuBar(nullptr);
    delete old;
    BuildMenuBar();
}
