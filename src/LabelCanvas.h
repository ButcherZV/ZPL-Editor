#pragma once
#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <vector>
#include <memory>
#include "zpl/ZPLTypes.h"
#include "ActiveTool.h"

class LabelElement;
class CommandHistory;
class Command;

// Fired on the parent window whenever the canvas content changes
wxDECLARE_EVENT(wxEVT_CANVAS_CHANGED, wxCommandEvent);

class LabelCanvas : public wxScrolledWindow
{
public:
    explicit LabelCanvas(wxWindow* parent);
    ~LabelCanvas();

    void SetLabelConfig(const LabelConfig& cfg);
    const LabelConfig& GetConfig() const { return m_config; }

    void SetElements(std::vector<std::shared_ptr<LabelElement>> elements);
    const std::vector<std::shared_ptr<LabelElement>>& GetElements() const { return m_elements; }

    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;

    void CopySelected();
    void PasteClipboard();
    void DuplicateSelected();

    // Active tool (for click-to-place)
    void       SetActiveTool(ActiveTool tool);
    ActiveTool GetActiveTool() const { return m_activeTool; }

    // Execute a command and notify listeners
    void ExecuteCommand(std::unique_ptr<Command> cmd);

    // Current selection
    LabelElement* GetSelected() const { return m_selected; }

    // Grid snapping
    void SetSnapToGrid(bool snap)   { m_snapToGrid = snap; }
    bool GetSnapToGrid()  const     { return m_snapToGrid; }
    void SetSnapSize(int dots)      { m_snapSize = std::max(1, dots); }
    int  GetSnapSize()    const     { return m_snapSize; }

    // Align selected element to label printable area
    // 0=Left 1=Right 2=Top 3=Bottom 4=CenterH 5=CenterV
    void AlignSelected(int type);

    void BringToFront();
    void SendToBack();

    wxPoint GetCursorDot() const { return m_cursorDot; }

    void ZoomIn();
    void ZoomOut();
    void ZoomFit();
    void SetZoom(double zoom);
    double GetZoom() const { return m_zoom; }

    // Convert canvas pixel coords → label dot coords
    wxPoint PixelToDot(wxPoint px) const;
    // Convert label dot coords → canvas pixel coords
    wxPoint DotToPixel(wxPoint dot) const;

private:
    static constexpr int   HANDLE_SIZE   = 8;
    static constexpr int   RULER_SIZE    = 20;
    static constexpr double ZOOM_STEP    = 1.25;
    static constexpr double ZOOM_MIN     = 0.1;
    static constexpr double ZOOM_MAX     = 10.0;

    // Painting
    void OnPaint(wxPaintEvent&);
    void DrawBackground(wxDC& dc);
    void DrawLabel(wxDC& dc);
    void DrawGrid(wxDC& dc);
    void DrawElements(wxDC& dc);
    void DrawSelectionHandles(wxDC& dc, LabelElement* el);
    void DrawRulers(wxDC& dc);

    // Mouse
    void OnMouseDown(wxMouseEvent&);
    void OnMouseMove(wxMouseEvent&);
    void OnMouseUp(wxMouseEvent&);
    void OnMouseWheel(wxMouseEvent&);
    void OnMiddleDown(wxMouseEvent&);
    void OnMiddleUp(wxMouseEvent&);
    void OnKeyDown(wxKeyEvent&);
    void OnContextMenu(wxContextMenuEvent&);
    void OnCtxDelete(wxCommandEvent&);
    void OnCtxDuplicate(wxCommandEvent&);
    void OnCtxBringFront(wxCommandEvent&);
    void OnCtxSendBack(wxCommandEvent&);

    void NotifyChanged();
    wxPoint SnapDot(wxPoint dot) const;

    LabelElement* HitTest(wxPoint pixelPt) const;
    int           HitTestHandle(LabelElement* el, wxPoint pixelPt) const;

    void UpdateScrollbars();
    int  LabelPixelWidth()  const;
    int  LabelPixelHeight() const;
    wxPoint LabelOffset() const;

    LabelConfig  m_config;
    double       m_zoom       = 1.0;
    ActiveTool   m_activeTool = ActiveTool::Select;
    bool         m_snapToGrid = true;
    int          m_snapSize   = 10;   // dots

    std::vector<std::shared_ptr<LabelElement>> m_elements;
    std::unique_ptr<CommandHistory>            m_history;

    // Interaction state
    LabelElement*  m_selected       = nullptr;
    int            m_dragHandle     = -1;
    bool           m_dragging       = false;
    wxPoint        m_dragStartPx;
    wxPoint        m_dragOrigDot;
    wxSize         m_dragOrigSize;

    wxPoint m_cursorDot = {-1, -1};

    // Middle-button pan
    bool    m_panning        = false;
    wxPoint m_panStartPx;
    wxPoint m_panStartScroll;

    // Clipboard (single element)
    std::shared_ptr<LabelElement> m_clipboard;

    wxDECLARE_EVENT_TABLE();
};
