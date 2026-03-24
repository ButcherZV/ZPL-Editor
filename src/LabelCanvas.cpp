#include "LabelCanvas.h"
#include "I18n.h"
#include "AppConfig.h"
#include "elements/LabelElement.h"
#include "elements/TextElement.h"
#include "elements/BarcodeElement.h"
#include "elements/BoxElement.h"
#include "elements/ImageElement.h"
#include "commands/CommandHistory.h"
#include "commands/Command.h"
#include "commands/AddElementCommand.h"
#include "commands/DeleteElementCommand.h"
#include "commands/MoveElementCommand.h"
#include "commands/ResizeElementCommand.h"
#include <wx/dcbuffer.h>
#include <wx/filedlg.h>
#include <algorithm>
#include <cmath>

wxDEFINE_EVENT(wxEVT_CANVAS_CHANGED, wxCommandEvent);

namespace {
    enum {
        ID_CTX_DELETE      = wxID_HIGHEST + 300,
        ID_CTX_DUPLICATE,
        ID_CTX_BRING_FRONT,
        ID_CTX_SEND_BACK,
    };
}

wxBEGIN_EVENT_TABLE(LabelCanvas, wxScrolledWindow)
    EVT_PAINT       (LabelCanvas::OnPaint)
    EVT_LEFT_DOWN   (LabelCanvas::OnMouseDown)
    EVT_LEFT_UP     (LabelCanvas::OnMouseUp)
    EVT_MOTION      (LabelCanvas::OnMouseMove)
    EVT_MOUSEWHEEL  (LabelCanvas::OnMouseWheel)
    EVT_MIDDLE_DOWN (LabelCanvas::OnMiddleDown)
    EVT_MIDDLE_UP   (LabelCanvas::OnMiddleUp)
    EVT_KEY_DOWN    (LabelCanvas::OnKeyDown)
    EVT_CONTEXT_MENU(LabelCanvas::OnContextMenu)
    EVT_MENU(ID_CTX_DELETE,      LabelCanvas::OnCtxDelete)
    EVT_MENU(ID_CTX_DUPLICATE,   LabelCanvas::OnCtxDuplicate)
    EVT_MENU(ID_CTX_BRING_FRONT, LabelCanvas::OnCtxBringFront)
    EVT_MENU(ID_CTX_SEND_BACK,   LabelCanvas::OnCtxSendBack)
wxEND_EVENT_TABLE()

LabelCanvas::LabelCanvas(wxWindow* parent)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxHSCROLL | wxVSCROLL | wxFULL_REPAINT_ON_RESIZE)
    , m_history(std::make_unique<CommandHistory>())
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(wxColour(180, 180, 180));
    SetScrollRate(5, 5);
}

LabelCanvas::~LabelCanvas() = default;

void LabelCanvas::SetLabelConfig(const LabelConfig& cfg)
{
    m_config = cfg;
    m_elements.clear();
    m_selected = nullptr;
    ZoomFit();
    UpdateScrollbars();
    Refresh();
}

void LabelCanvas::SetElements(std::vector<std::shared_ptr<LabelElement>> elements)
{
    m_elements = std::move(elements);
    m_selected = nullptr;
    Refresh();
}

// ── Coordinate helpers ────────────────────────────────────────────────────────

wxPoint LabelCanvas::LabelOffset() const
{
    // Offset for ruler strips (RULER_SIZE) plus 20px padding on each side
    return wxPoint(RULER_SIZE + 20, RULER_SIZE + 20);
}

int LabelCanvas::LabelPixelWidth() const
{
    return static_cast<int>(std::round(m_config.totalWidth() * m_zoom));
}

int LabelCanvas::LabelPixelHeight() const
{
    return static_cast<int>(std::round(m_config.totalHeight() * m_zoom));
}

wxPoint LabelCanvas::PixelToDot(wxPoint px) const
{
    wxPoint off = LabelOffset();
    int x = static_cast<int>((px.x - off.x) / m_zoom);
    int y = static_cast<int>((px.y - off.y) / m_zoom);
    return wxPoint(x, y);
}

wxPoint LabelCanvas::DotToPixel(wxPoint dot) const
{
    wxPoint off = LabelOffset();
    int x = static_cast<int>(std::round(dot.x * m_zoom)) + off.x;
    int y = static_cast<int>(std::round(dot.y * m_zoom)) + off.y;
    return wxPoint(x, y);
}

void LabelCanvas::UpdateScrollbars()
{
    int vw = LabelPixelWidth()  + (RULER_SIZE + 20) * 2;
    int vh = LabelPixelHeight() + (RULER_SIZE + 20) * 2;
    SetVirtualSize(vw, vh);
}

// ── Zoom ──────────────────────────────────────────────────────────────────────

void LabelCanvas::SetZoom(double zoom)
{
    m_zoom = std::clamp(zoom, ZOOM_MIN, ZOOM_MAX);
    UpdateScrollbars();
    Refresh();
    // Notify parent so status bar zoom display can update
    NotifyChanged();
}

void LabelCanvas::ZoomIn()  { SetZoom(m_zoom * ZOOM_STEP); }
void LabelCanvas::ZoomOut() { SetZoom(m_zoom / ZOOM_STEP); }

// ── Active tool + command execution ──────────────────────────────────────────

void LabelCanvas::SetActiveTool(ActiveTool tool)
{
    m_activeTool = tool;
    SetCursor(tool == ActiveTool::Select
              ? wxCursor(wxCURSOR_ARROW)
              : wxCursor(wxCURSOR_CROSS));
}

void LabelCanvas::ExecuteCommand(std::unique_ptr<Command> cmd)
{
    m_history->Execute(std::move(cmd));
    Refresh();
    NotifyChanged();
}

void LabelCanvas::NotifyChanged()
{
    wxCommandEvent evt(wxEVT_CANVAS_CHANGED);
    wxPostEvent(GetParent(), evt);
}

void LabelCanvas::ZoomFit()
{
    if (m_config.totalWidth() <= 0 || m_config.totalHeight() <= 0)
    {
        m_zoom = 1.0;
        return;
    }
    wxSize client = GetClientSize();
    const int pad = (RULER_SIZE + 20) * 2;
    double zx = static_cast<double>(client.x - pad) / m_config.totalWidth();
    double zy = static_cast<double>(client.y - pad) / m_config.totalHeight();
    SetZoom(std::min(zx, zy));
}

// ── Undo / Redo ───────────────────────────────────────────────────────────────

void LabelCanvas::Undo()
{
    m_history->Undo();
    m_selected = nullptr;
    NotifyChanged();
    Refresh();
}

void LabelCanvas::Redo()
{
    m_history->Redo();
    m_selected = nullptr;
    NotifyChanged();
    Refresh();
}

bool LabelCanvas::CanUndo() const { return m_history->CanUndo(); }
bool LabelCanvas::CanRedo() const { return m_history->CanRedo(); }

// ── Snap / Align ──────────────────────────────────────────────────────────────

wxPoint LabelCanvas::SnapDot(wxPoint dot) const
{
    if (!m_snapToGrid || m_snapSize <= 0) return dot;
    auto snap = [&](int v) {
        return static_cast<int>(std::round(static_cast<double>(v) / m_snapSize)) * m_snapSize;
    };
    return wxPoint(snap(dot.x), snap(dot.y));
}

void LabelCanvas::AlignSelected(int type)
{
    if (!m_selected) return;
    const auto& c = m_config;

    int nx = m_selected->x, ny = m_selected->y;
    switch (type)
    {
    case 0: nx = c.marginLeft; break;                                                      // Left
    case 1: nx = c.marginLeft + c.widthDots  - m_selected->w; break;                      // Right
    case 2: ny = c.marginTop;  break;                                                      // Top
    case 3: ny = c.marginTop  + c.heightDots - m_selected->h; break;                      // Bottom
    case 4: nx = c.marginLeft + (c.widthDots  - m_selected->w) / 2; break;                // Center H
    case 5: ny = c.marginTop  + (c.heightDots - m_selected->h) / 2; break;                // Center V
    }
    if (nx != m_selected->x || ny != m_selected->y)
    {
        auto cmd = std::make_unique<MoveElementCommand>(
            m_selected,
            wxPoint(m_selected->x, m_selected->y),
            wxPoint(nx, ny));
        ExecuteCommand(std::move(cmd));
    }
}

void LabelCanvas::CopySelected()
{
    if (m_selected)
        m_clipboard = m_selected->Clone();
}

void LabelCanvas::PasteClipboard()
{
    if (!m_clipboard) return;
    auto el = m_clipboard->Clone();
    el->x += 10;
    el->y += 10;
    m_selected = el.get();
    ExecuteCommand(std::make_unique<AddElementCommand>(m_elements, std::move(el)));
}

void LabelCanvas::DuplicateSelected()
{
    if (!m_selected) return;
    auto el = m_selected->Clone();
    el->x += 10;
    el->y += 10;
    m_selected = el.get();
    ExecuteCommand(std::make_unique<AddElementCommand>(m_elements, std::move(el)));
}
void LabelCanvas::BringToFront()
{
    if (!m_selected) return;
    for (int i = 0; i < static_cast<int>(m_elements.size()); ++i)
    {
        if (m_elements[i].get() == m_selected)
        {
            if (i == static_cast<int>(m_elements.size()) - 1) return;
            auto el = m_elements[i];
            m_elements.erase(m_elements.begin() + i);
            m_elements.push_back(el);
            NotifyChanged(); Refresh(); return;
        }
    }
}

void LabelCanvas::SendToBack()
{
    if (!m_selected) return;
    for (int i = 0; i < static_cast<int>(m_elements.size()); ++i)
    {
        if (m_elements[i].get() == m_selected)
        {
            if (i == 0) return;
            auto el = m_elements[i];
            m_elements.erase(m_elements.begin() + i);
            m_elements.insert(m_elements.begin(), el);
            NotifyChanged(); Refresh(); return;
        }
    }
}
// ── Painting ──────────────────────────────────────────────────────────────────

void LabelCanvas::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    DoPrepareDC(dc);
    DrawBackground(dc);
    if (m_config.totalWidth() > 0)
    {
        DrawLabel(dc);
        DrawGrid(dc);
        DrawElements(dc);
    }
    // Draw rulers as a fixed overlay in unscrolled client coords
    {
        wxCoord ox, oy;
        dc.GetDeviceOrigin(&ox, &oy);
        dc.SetDeviceOrigin(0, 0);
        DrawRulers(dc);
        dc.SetDeviceOrigin(ox, oy);
    }
}

void LabelCanvas::DrawBackground(wxDC& dc)
{
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
}

void LabelCanvas::DrawLabel(wxDC& dc)
{
    wxPoint off = LabelOffset();
    int pw = LabelPixelWidth();
    int ph = LabelPixelHeight();

    // Drop shadow
    dc.SetBrush(wxBrush(wxColour(100, 100, 100)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(off.x + 3, off.y + 3, pw, ph);

    // White label surface
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(wxPen(wxColour(60, 60, 60), 1));
    dc.DrawRectangle(off.x, off.y, pw, ph);

    // Margin rectangle (dashed)
    if (m_config.marginLeft || m_config.marginTop ||
        m_config.marginRight || m_config.marginBottom)
    {
        int mx = static_cast<int>(m_config.marginLeft   * m_zoom) + off.x;
        int my = static_cast<int>(m_config.marginTop    * m_zoom) + off.y;
        int mw = static_cast<int>(m_config.widthDots    * m_zoom);
        int mh = static_cast<int>(m_config.heightDots   * m_zoom);

        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(wxPen(wxColour(180, 80, 80), 1, wxPENSTYLE_SHORT_DASH));
        dc.DrawRectangle(mx, my, mw, mh);
    }
}

void LabelCanvas::DrawGrid(wxDC& dc)
{
    // Draw a subtle grid every 10 mm (or proportional in dots)
    if (m_config.dpi == PrinterDPI::DPI_203 || true)
    {
        int gridDots = MMToDots(10.0, m_config.dpi);   // 10 mm grid
        if (gridDots <= 0) return;

        double gridPx = gridDots * m_zoom;
        if (gridPx < 4.0) return;  // too dense to show

        wxPoint off = LabelOffset();
        int pw = LabelPixelWidth();
        int ph = LabelPixelHeight();

        dc.SetPen(wxPen(wxColour(210, 210, 255), 1));

        for (double x = off.x; x <= off.x + pw; x += gridPx)
            dc.DrawLine(static_cast<int>(x), off.y,
                        static_cast<int>(x), off.y + ph);
        for (double y = off.y; y <= off.y + ph; y += gridPx)
            dc.DrawLine(off.x, static_cast<int>(y),
                        off.x + pw, static_cast<int>(y));
    }
}

void LabelCanvas::DrawElements(wxDC& dc)
{
    for (auto& el : m_elements)
    {
        el->Draw(dc, LabelOffset(), m_zoom);
        if (el.get() == m_selected)
            DrawSelectionHandles(dc, el.get());
    }
}

void LabelCanvas::DrawSelectionHandles(wxDC& dc, LabelElement* el)
{
    wxPoint off    = LabelOffset();
    wxRect  bounds = el->GetBoundsPixels(off, m_zoom);

    // Highlight border
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(wxColour(0, 120, 215), 1, wxPENSTYLE_SHORT_DASH));
    dc.DrawRectangle(bounds);

    // 8 handles: TL, TM, TR, ML, MR, BL, BM, BR
    std::array<wxPoint, 8> pts = {
        wxPoint(bounds.x,                     bounds.y),
        wxPoint(bounds.x + bounds.width / 2,  bounds.y),
        wxPoint(bounds.x + bounds.width,      bounds.y),
        wxPoint(bounds.x,                     bounds.y + bounds.height / 2),
        wxPoint(bounds.x + bounds.width,      bounds.y + bounds.height / 2),
        wxPoint(bounds.x,                     bounds.y + bounds.height),
        wxPoint(bounds.x + bounds.width / 2,  bounds.y + bounds.height),
        wxPoint(bounds.x + bounds.width,      bounds.y + bounds.height),
    };

    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(wxPen(wxColour(0, 120, 215), 1));
    for (auto& p : pts)
        dc.DrawRectangle(p.x - HANDLE_SIZE / 2, p.y - HANDLE_SIZE / 2,
                         HANDLE_SIZE, HANDLE_SIZE);
}
// ── Rulers ────────────────────────────────────────────────────────────────────────────
void LabelCanvas::DrawRulers(wxDC& dc)
{
    // DC is already in client (unscrolled) coords when called from OnPaint
    wxSize cl = GetClientSize();
    const wxColour bgCol(235, 235, 235);
    const wxColour fgCol( 80,  80,  80);
    const wxColour mkCol(160, 160, 160);

    // Backgrounds
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(bgCol));
    dc.DrawRectangle(RULER_SIZE, 0,          cl.x - RULER_SIZE, RULER_SIZE); // top
    dc.DrawRectangle(0,          RULER_SIZE, RULER_SIZE, cl.y - RULER_SIZE); // left
    dc.SetBrush(wxBrush(wxColour(210, 210, 210)));
    dc.DrawRectangle(0, 0, RULER_SIZE, RULER_SIZE);                          // corner

    // Border lines
    dc.SetPen(wxPen(mkCol, 1));
    dc.DrawLine(0,          RULER_SIZE, cl.x, RULER_SIZE); // top ruler bottom edge
    dc.DrawLine(RULER_SIZE, 0,          RULER_SIZE, cl.y); // left ruler right edge

    if (m_config.totalWidth() <= 0) return;

    // Where does dot-0 appear in client coords?
    int xu, yu;  GetScrollPixelsPerUnit(&xu, &yu);
    int vx, vy;  GetViewStart(&vx, &vy);
    wxPoint off  = LabelOffset();
    double labelCX = off.x - vx * xu;
    double labelCY = off.y - vy * yu;

    bool   metric  = (AppConfig::Get().units == MeasureUnit::Metric);
    double dpu     = metric ? DotsPerMM(m_config.dpi) : DotsPerInch(m_config.dpi);
    double pxPerU  = dpu * m_zoom;          // pixels per mm (or inch)

    const double tickStep  = metric ? 1.0 : 0.1;
    const int    labelStep = 5;             // label every 5 ticks (5 mm or 0.5")

    dc.SetFont(wxFont(7, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetTextForeground(fgCol);

    // ── Horizontal ruler ───────────────────────────────────────────────────────────────────
    {
        double firstU = std::ceil((RULER_SIZE - labelCX) / pxPerU / tickStep) * tickStep;
        for (double u = std::max(firstU, 0.0); ; u += tickStep)
        {
            double px = labelCX + u * pxPerU;
            if (px > cl.x)      break;
            if (px < RULER_SIZE) continue;
            int ix  = static_cast<int>(px);
            int idx = static_cast<int>(std::round(u / tickStep));
            bool big = (idx % labelStep == 0);
            int  th  = big ? RULER_SIZE * 2 / 3 : RULER_SIZE / 3;
            dc.SetPen(wxPen(mkCol, 1));
            dc.DrawLine(ix, RULER_SIZE - th, ix, RULER_SIZE - 1);
            if (big && pxPerU * labelStep >= 16.0 && ix + 2 < cl.x)
            {
                wxString s = metric ? wxString::Format("%.0f", u)
                                    : wxString::Format("%.1f", u);
                dc.DrawText(s, ix + 2, 2);
            }
        }
    }

    // ── Vertical ruler ────────────────────────────────────────────────────────────────────
    {
        double firstU = std::ceil((RULER_SIZE - labelCY) / pxPerU / tickStep) * tickStep;
        for (double u = std::max(firstU, 0.0); ; u += tickStep)
        {
            double py = labelCY + u * pxPerU;
            if (py > cl.y)      break;
            if (py < RULER_SIZE) continue;
            int iy  = static_cast<int>(py);
            int idx = static_cast<int>(std::round(u / tickStep));
            bool big = (idx % labelStep == 0);
            int  tw  = big ? RULER_SIZE * 2 / 3 : RULER_SIZE / 3;
            dc.SetPen(wxPen(mkCol, 1));
            dc.DrawLine(RULER_SIZE - tw, iy, RULER_SIZE - 1, iy);
            if (big && pxPerU * labelStep >= 16.0 && iy > RULER_SIZE + 4)
            {
                wxString s = metric ? wxString::Format("%.0f", u)
                                    : wxString::Format("%.1f", u);
                wxSize   ext = dc.GetTextExtent(s);
                dc.DrawText(s, std::max(1, RULER_SIZE - ext.x - 2), iy - ext.y - 1);
            }
        }
    }
}
// ── Hit testing ───────────────────────────────────────────────────────────────

LabelElement* LabelCanvas::HitTest(wxPoint px) const
{
    wxPoint off = LabelOffset();
    // Iterate in reverse (top element first)
    for (int i = static_cast<int>(m_elements.size()) - 1; i >= 0; --i)
    {
        wxRect b = m_elements[i]->GetBoundsPixels(off, m_zoom);
        if (b.Contains(px))
            return m_elements[i].get();
    }
    return nullptr;
}

int LabelCanvas::HitTestHandle(LabelElement* el, wxPoint px) const
{
    if (!el) return -2;
    wxPoint off    = LabelOffset();
    wxRect  bounds = el->GetBoundsPixels(off, m_zoom);

    std::array<wxPoint, 8> pts = {
        wxPoint(bounds.x,                     bounds.y),
        wxPoint(bounds.x + bounds.width / 2,  bounds.y),
        wxPoint(bounds.x + bounds.width,      bounds.y),
        wxPoint(bounds.x,                     bounds.y + bounds.height / 2),
        wxPoint(bounds.x + bounds.width,      bounds.y + bounds.height / 2),
        wxPoint(bounds.x,                     bounds.y + bounds.height),
        wxPoint(bounds.x + bounds.width / 2,  bounds.y + bounds.height),
        wxPoint(bounds.x + bounds.width,      bounds.y + bounds.height),
    };

    for (int i = 0; i < 8; ++i)
    {
        wxRect hr(pts[i].x - HANDLE_SIZE, pts[i].y - HANDLE_SIZE,
                  HANDLE_SIZE * 2, HANDLE_SIZE * 2);
        if (hr.Contains(px))
            return i;
    }
    return -1;  // no handle hit, but might be body
}

// ── Mouse interaction ─────────────────────────────────────────────────────────

void LabelCanvas::OnMouseDown(wxMouseEvent& evt)
{
    SetFocus();
    wxPoint px = evt.GetPosition();

    // ── Tool placement mode ───────────────────────────────────────────────────
    if (m_activeTool != ActiveTool::Select)
    {
        wxPoint off = LabelOffset();
        wxRect  labelRect(off.x, off.y, LabelPixelWidth(), LabelPixelHeight());
        if (labelRect.Contains(px))
        {
            wxPoint dot = SnapDot(PixelToDot(px));
            std::shared_ptr<LabelElement> newEl;

            switch (m_activeTool)
            {
            case ActiveTool::Text:
            {
                auto el = std::make_shared<TextElement>();
                el->x = dot.x; el->y = dot.y;
                newEl = el;
                break;
            }
            case ActiveTool::Barcode:
            {
                auto el = std::make_shared<BarcodeElement>();
                el->x = dot.x; el->y = dot.y;
                newEl = el;
                break;
            }
            case ActiveTool::Box:
            {
                auto el = std::make_shared<BoxElement>();
                el->x = dot.x; el->y = dot.y;
                newEl = el;
                break;
            }
            case ActiveTool::Image:
            {
                wxFileDialog fdlg(this, TR(DLG_CHOOSE_IMAGE), "", "",
                    TR(FILE_FILTER_IMAGE),
                    wxFD_OPEN | wxFD_FILE_MUST_EXIST);
                if (fdlg.ShowModal() != wxID_OK)
                {
                    SetActiveTool(ActiveTool::Select);
                    return;
                }
                auto el = std::make_shared<ImageElement>();
                el->LoadFromFile(fdlg.GetPath());
                el->x = dot.x; el->y = dot.y;
                newEl = el;
                break;
            }
            default: break;
            }

            if (newEl)
            {
                m_selected = newEl.get();
                m_history->Execute(
                    std::make_unique<AddElementCommand>(m_elements, newEl));
                SetActiveTool(ActiveTool::Select);
                NotifyChanged();
                Refresh();
                return;
            }
        }
        // Click outside label: cancel tool
        SetActiveTool(ActiveTool::Select);
        Refresh();
        return;
    }

    // ── Select tool: handle drag / resize ────────────────────────────────────
    if (m_selected)
    {
        int handle = HitTestHandle(m_selected, px);
        if (handle >= 0)
        {
            m_dragHandle   = handle;
            m_dragging     = true;
            m_dragStartPx  = px;
            m_dragOrigDot  = wxPoint(m_selected->x, m_selected->y);
            m_dragOrigSize = wxSize(m_selected->w, m_selected->h);
            CaptureMouse();
            return;
        }
    }

    LabelElement* hit = HitTest(px);
    if (hit)
    {
        m_selected     = hit;
        m_dragHandle   = -1;  // body drag
        m_dragging     = true;
        m_dragStartPx  = px;
        m_dragOrigDot  = wxPoint(hit->x, hit->y);
        m_dragOrigSize = wxSize(hit->w, hit->h);
        CaptureMouse();
    }
    else
    {
        m_selected = nullptr;
    }
    Refresh();
    evt.Skip();
}

void LabelCanvas::OnMouseMove(wxMouseEvent& evt)
{
    m_cursorDot = PixelToDot(evt.GetPosition());

    if (m_panning)
    {
        wxPoint delta = evt.GetPosition() - m_panStartPx;
        int xu, yu;
        GetScrollPixelsPerUnit(&xu, &yu);
        if (xu > 0 && yu > 0)
        {
            Scroll(m_panStartScroll.x - delta.x / xu,
                   m_panStartScroll.y - delta.y / yu);
            // Rulers are drawn as a device-space overlay so they won't be
            // repainted by the partial-region update that Scroll() triggers.
            // Force the ruler strips to redraw explicitly.
            wxSize cl = GetClientSize();
            RefreshRect(wxRect(0, 0, cl.x, RULER_SIZE), false); // top ruler
            RefreshRect(wxRect(0, 0, RULER_SIZE, cl.y), false); // left ruler
        }
        return;
    }

    if (!m_dragging || !m_selected) { evt.Skip(); return; }

    wxPoint px    = evt.GetPosition();
    wxPoint delta = px - m_dragStartPx;
    int     ddx   = static_cast<int>(delta.x / m_zoom);
    int     ddy   = static_cast<int>(delta.y / m_zoom);

    if (m_dragHandle == -1)
    {
        // Body drag — move element (with snap)
        auto snapped = SnapDot(wxPoint(m_dragOrigDot.x + ddx, m_dragOrigDot.y + ddy));
        m_selected->x = snapped.x;
        m_selected->y = snapped.y;
    }
    else
    {
        // Handle drag — resize element
        // Handles: 0=TL,1=TM,2=TR,3=ML,4=MR,5=BL,6=BM,7=BR
        int nx = m_dragOrigDot.x;
        int ny = m_dragOrigDot.y;
        int nw = m_dragOrigSize.x;
        int nh = m_dragOrigSize.y;

        switch (m_dragHandle)
        {
        case 0: nx += ddx; ny += ddy; nw -= ddx; nh -= ddy; break; // TL
        case 1:             ny += ddy;             nh -= ddy; break; // TM
        case 2:             ny += ddy; nw += ddx; nh -= ddy; break; // TR
        case 3: nx += ddx;             nw -= ddx;             break; // ML
        case 4:             nw += ddx;             break;            // MR
        case 5: nx += ddx;             nw -= ddx; nh += ddy; break; // BL
        case 6:                         nh += ddy; break;            // BM
        case 7:             nw += ddx; nh += ddy; break;            // BR
        }

        if (nw > 4) { m_selected->x = nx; m_selected->w = nw; }
        if (nh > 4) { m_selected->y = ny; m_selected->h = nh; }
    }
    Refresh();
}

void LabelCanvas::OnMouseUp(wxMouseEvent& evt)
{
    if (!m_dragging || !m_selected) { evt.Skip(); return; }

    if (HasCapture()) ReleaseMouse();
    m_dragging = false;

    // Commit to undo history
    if (m_dragHandle == -1)
    {
        if (m_selected->x != m_dragOrigDot.x ||
            m_selected->y != m_dragOrigDot.y)
        {
            auto cmd = std::make_unique<MoveElementCommand>(
                m_selected,
                m_dragOrigDot,
                wxPoint(m_selected->x, m_selected->y));
            m_history->Execute(std::move(cmd));
        }
    }
    else
    {
        if (m_selected->w != m_dragOrigSize.x ||
            m_selected->h != m_dragOrigSize.y)
        {
            auto cmd = std::make_unique<ResizeElementCommand>(
                m_selected,
                m_dragOrigDot, m_dragOrigSize,
                wxPoint(m_selected->x, m_selected->y),
                wxSize(m_selected->w, m_selected->h));
            m_history->Execute(std::move(cmd));
        }
    }
    Refresh();
    NotifyChanged();
}

void LabelCanvas::OnMouseWheel(wxMouseEvent& evt)
{
    if (evt.ControlDown())
    {
        if (evt.GetWheelRotation() > 0) ZoomIn();
        else                             ZoomOut();
    }
    else
    {
        evt.Skip();
    }
}

void LabelCanvas::OnMiddleDown(wxMouseEvent& evt)
{
    m_panning = true;
    m_panStartPx = evt.GetPosition();
    GetViewStart(&m_panStartScroll.x, &m_panStartScroll.y);
    SetCursor(wxCursor(wxCURSOR_HAND));
    CaptureMouse();
}

void LabelCanvas::OnMiddleUp(wxMouseEvent& evt)
{
    if (!m_panning) { evt.Skip(); return; }
    m_panning = false;
    if (HasCapture()) ReleaseMouse();
    SetCursor(m_activeTool == ActiveTool::Select
              ? wxCursor(wxCURSOR_ARROW)
              : wxCursor(wxCURSOR_CROSS));
}

void LabelCanvas::OnKeyDown(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();

    // ── Clipboard / duplicate ─────────────────────────────────────────────────
    if (evt.ControlDown())
    {
        if (key == 'C' || key == WXK_CONTROL_C) { CopySelected();      return; }
        if (key == 'V' || key == WXK_CONTROL_V) { PasteClipboard();    return; }
        if (key == 'D' || key == WXK_CONTROL_D) { DuplicateSelected(); return; }
        evt.Skip(); return;
    }

    // ── Arrow key nudge ──────────────────────────────────────────────────────
    if (m_selected && (key == WXK_LEFT || key == WXK_RIGHT ||
                       key == WXK_UP   || key == WXK_DOWN))
    {
        int step = evt.ShiftDown() ? 1 : m_snapSize;
        int nx = m_selected->x, ny = m_selected->y;
        if (key == WXK_LEFT)  nx -= step;
        if (key == WXK_RIGHT) nx += step;
        if (key == WXK_UP)    ny -= step;
        if (key == WXK_DOWN)  ny += step;
        auto cmd = std::make_unique<MoveElementCommand>(
            m_selected,
            wxPoint(m_selected->x, m_selected->y),
            wxPoint(nx, ny));
        ExecuteCommand(std::move(cmd));
        return;
    }

    // ── Tool shortcuts + snap + delete ────────────────────────────────────────
    switch (key)
    {
    case WXK_ESCAPE:
    case 'V':
        SetActiveTool(ActiveTool::Select);  NotifyChanged(); break;
    case 'T':
        SetActiveTool(ActiveTool::Text);    NotifyChanged(); break;
    case 'B':
        SetActiveTool(ActiveTool::Barcode); NotifyChanged(); break;
    case 'R':
        SetActiveTool(ActiveTool::Box);     NotifyChanged(); break;
    case 'I':
        SetActiveTool(ActiveTool::Image);   NotifyChanged(); break;
    case 'G':
        SetSnapToGrid(!m_snapToGrid);
        NotifyChanged();
        break;

    // ── Delete ────────────────────────────────────────────────────────────────
    case WXK_DELETE:
    case WXK_BACK:
        if (m_selected)
        {
            for (auto& sp : m_elements)
            {
                if (sp.get() == m_selected)
                {
                    auto cmd = std::make_unique<DeleteElementCommand>(m_elements, sp);
                    m_selected = nullptr;
                    m_history->Execute(std::move(cmd));
                    NotifyChanged();
                    Refresh();
                    return;
                }
            }
        }
        break;

    default:
        evt.Skip();
        return;
    }
    Refresh();
}

// ── Context menu ──────────────────────────────────────────────────────────────

void LabelCanvas::OnContextMenu(wxContextMenuEvent& evt)
{
    wxPoint pt;
    if (evt.GetPosition() == wxDefaultPosition)
        pt = wxPoint(GetClientSize().x / 2, GetClientSize().y / 2);
    else
        pt = ScreenToClient(evt.GetPosition());

    // Select element under cursor (or clear selection)
    LabelElement* hit = HitTest(pt);
    if (hit != m_selected)
    {
        m_selected = hit;
        Refresh();
        if (hit) NotifyChanged();
    }

    if (!m_selected) return;  // nothing under cursor — no menu

    wxMenu menu;
    menu.Append(ID_CTX_DUPLICATE,   TR(CTX_DUPLICATE));
    menu.AppendSeparator();
    menu.Append(ID_CTX_BRING_FRONT, TR(CTX_BRING_FRONT));
    menu.Append(ID_CTX_SEND_BACK,   TR(CTX_SEND_BACK));
    menu.AppendSeparator();
    menu.Append(ID_CTX_DELETE,      TR(CTX_DELETE));
    PopupMenu(&menu, pt);
}

void LabelCanvas::OnCtxDelete(wxCommandEvent&)
{
    if (!m_selected) return;
    for (auto& sp : m_elements)
    {
        if (sp.get() == m_selected)
        {
            auto cmd = std::make_unique<DeleteElementCommand>(m_elements, sp);
            m_selected = nullptr;
            m_history->Execute(std::move(cmd));
            NotifyChanged(); Refresh();
            return;
        }
    }
}

void LabelCanvas::OnCtxDuplicate(wxCommandEvent&)  { DuplicateSelected(); }
void LabelCanvas::OnCtxBringFront(wxCommandEvent&) { BringToFront(); }
void LabelCanvas::OnCtxSendBack(wxCommandEvent&)   { SendToBack(); }
