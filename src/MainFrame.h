#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/docview.h>
#include "zpl/ZPLTypes.h"

class LabelCanvas;
class ToolboxPanel;
class PropertiesPanel;
class ZPLCodePanel;

class MainFrame : public wxFrame
{
public:
    explicit MainFrame(wxWindow* parent);
    ~MainFrame();

    void LoadLabel(const LabelConfig& config);

    LabelCanvas*     GetCanvas()     { return m_canvas; }
    PropertiesPanel* GetProperties() { return m_properties; }
    ZPLCodePanel*    GetCodePanel()  { return m_codePanel; }

private:
    enum
    {
        ID_NEW    = wxID_NEW,
        ID_OPEN   = wxID_OPEN,
        ID_SAVE   = wxID_SAVE,
        ID_SAVEAS = wxID_SAVEAS,
        ID_EXIT   = wxID_EXIT,

        ID_UNDO      = wxID_UNDO,
        ID_REDO      = wxID_REDO,
        ID_OPTIONS   = wxID_PREFERENCES,
        ID_DUPLICATE = wxID_HIGHEST + 10,

        ID_ALIGN_LEFT    = wxID_HIGHEST + 20,
        ID_ALIGN_RIGHT,
        ID_ALIGN_TOP,
        ID_ALIGN_BOTTOM,
        ID_ALIGN_CENTERH,
        ID_ALIGN_CENTERV,

        ID_ZOOM_IN  = wxID_ZOOM_IN,
        ID_ZOOM_OUT = wxID_ZOOM_OUT,
        ID_ZOOM_FIT = wxID_ZOOM_FIT,

        ID_PRINT = wxID_PRINT,

        ID_SNAP_GRID      = wxID_HIGHEST + 30,
        ID_TOGGLE_TOOLBOX = wxID_HIGHEST + 1,
        ID_TOGGLE_PROPS,
        ID_TOGGLE_ZPL,

        ID_LANG_ENGLISH   = wxID_HIGHEST + 40,
        ID_LANG_SERBIAN,
    };

    void BuildMenuBar();
    void BuildToolBar();
    void BuildPanels();
    void LoadFile(const wxString& path);  // shared open logic

    // Menu handlers
    void OnNew(wxCommandEvent&);
    void OnOpen(wxCommandEvent&);
    void OnRecentFile(wxCommandEvent&);
    void OnSave(wxCommandEvent&);
    void OnSaveAs(wxCommandEvent&);
    void OnExit(wxCommandEvent&);
    void OnUndo(wxCommandEvent&);
    void OnRedo(wxCommandEvent&);
    void OnUpdateUndo(wxUpdateUIEvent&);
    void OnUpdateRedo(wxUpdateUIEvent&);
    void OnDuplicate(wxCommandEvent&);
    void OnAlignElement(wxCommandEvent&);
    void OnUpdateAlignElement(wxUpdateUIEvent&);
    void OnOptions(wxCommandEvent&);
    void OnSnapGrid(wxCommandEvent&);
    void OnUpdateSnapGrid(wxUpdateUIEvent&);
    void OnToggleZPL(wxCommandEvent&);
    void OnZoomIn(wxCommandEvent&);
    void OnZoomOut(wxCommandEvent&);
    void OnZoomFit(wxCommandEvent&);
    void OnPrint(wxCommandEvent&);
    void OnClose(wxCloseEvent&);
    void OnCanvasChanged(wxCommandEvent&);
    void OnLanguage(wxCommandEvent&);
    void OnCanvasMouseMove(wxMouseEvent&);

    wxAuiManager     m_auiMgr;
    LabelCanvas*     m_canvas     = nullptr;
    ToolboxPanel*    m_toolbox    = nullptr;
    PropertiesPanel* m_properties = nullptr;
    ZPLCodePanel*    m_codePanel  = nullptr;
    wxFileHistory    m_fileHistory;
    wxMenu*          m_recentMenu = nullptr;

    wxString         m_currentFile;

    wxDECLARE_EVENT_TABLE();
};
