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
#include "commands/CompoundCommand.h"
#include "commands/EditPropertyCommand.h"
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

bool LabelCanvas::IsSelected(LabelElement* el) const
{
    return std::find(m_selection.begin(), m_selection.end(), el) != m_selection.end();
}

void LabelCanvas::SetLabelConfig(const LabelConfig& cfg)
{
    m_config = cfg;
    m_elements.clear();
    m_selection.clear();
    m_selected = nullptr;
    ZoomFit();
    UpdateScrollbars();
    Refresh();
}

void LabelCanvas::SetElements(std::vector<std::shared_ptr<LabelElement>> elements)
{
    m_elements = std::move(elements);
    m_selection.clear();
    m_selected = nullptr;
    m_history->Clear();  // don't let old undo history bleed across files
    Refresh();
}

// ── Coordinate helpers ────────────────────────────────────────────────────────

wxPoint LabelCanvas::LabelOffset() const
{
    // Virtual-space origin of the label area.
    // Extra PAN_MARGIN means Scroll(PAN_MARGIN/xu) shows the label at the natural position,
    // and the user can pan by up to PAN_MARGIN pixels in every direction using middle-mouse.
    return wxPoint(RULER_SIZE + 20 + PAN_MARGIN, RULER_SIZE + 20 + PAN_MARGIN);
}

int LabelCanvas::LabelPixelWidth() const
{
    return static_cast<int>(std::round(m_config.totalWidth() * m_zoom));
}

int LabelCanvas::LabelPixelHeight() const
{
    return static_cast<int>(std::round(m_config.totalHeight() * m_zoom));
}

wxPoint LabelCanvas::PixelToDot(wxPoint clientPx) const
{
    // Mouse events use client coordinates; LabelOffset() is in virtual (unscrolled) coords.
    // Convert client → virtual before subtracting the label origin.
    int vx, vy;
    CalcUnscrolledPosition(clientPx.x, clientPx.y, &vx, &vy);
    wxPoint off = LabelOffset();
    int x = static_cast<int>((vx - off.x) / m_zoom);
    int y = static_cast<int>((vy - off.y) / m_zoom);
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
    // Add PAN_MARGIN on each side so there is always scrollable room for middle-mouse pan.
    int vw = LabelPixelWidth()  + (RULER_SIZE + 20) * 2 + PAN_MARGIN * 2;
    int vh = LabelPixelHeight() + (RULER_SIZE + 20) * 2 + PAN_MARGIN * 2;
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

void LabelCanvas::MarkHistoryClean() { m_history->MarkClean(); }
bool LabelCanvas::IsModified()  const { return m_history->IsModified(); }

void LabelCanvas::ZoomFit()
{
    if (m_config.totalWidth() <= 0 || m_config.totalHeight() <= 0)
    {
        m_zoom = 1.0;
        return;
    }
    wxSize client = GetClientSize();
    const int pad = (RULER_SIZE + 20) * 2;
    int lpr = std::max(1, m_config.labelsPerRow);
    (void)lpr; // lpr only affects ZPL export; canvas always shows one column
    double zx = static_cast<double>(client.x - pad) / m_config.totalWidth();
    double zy = static_cast<double>(client.y - pad) / m_config.totalHeight();
    SetZoom(std::min(zx, zy));  // also calls UpdateScrollbars

    // Scroll so the label appears at its natural position (PAN_MARGIN into virtual space)
    int xu, yu;
    GetScrollPixelsPerUnit(&xu, &yu);
    if (xu > 0 && yu > 0)
        Scroll(PAN_MARGIN / xu, PAN_MARGIN / yu);
}

// ── Undo / Redo ───────────────────────────────────────────────────────────────

void LabelCanvas::Undo()
{
    m_history->Undo();
    m_selection.clear();
    m_selected = nullptr;
    NotifyChanged();
    Refresh();
}

void LabelCanvas::Redo()
{
    m_history->Redo();
    m_selection.clear();
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
    if (m_selection.empty()) return;
    const auto& c = m_config;

    // For edge alignments (0-3) compute the reference from the selection itself:
    //   Left   → leftmost left edge   (min x)
    //   Right  → rightmost right edge (max x+w)
    //   Top    → topmost top edge     (min y)
    //   Bottom → bottommost bot edge  (max y+h)
    // For center alignments (4-5) we still center relative to the label area.
    int refLeft = INT_MAX, refTop = INT_MAX;
    int refRight = INT_MIN, refBottom = INT_MIN;
    for (const LabelElement* el : m_selection)
    {
        refLeft   = std::min(refLeft,   el->x);
        refTop    = std::min(refTop,    el->y);
        refRight  = std::max(refRight,  el->x + el->w);
        refBottom = std::max(refBottom, el->y + el->h);
    }

    auto compound = std::make_unique<CompoundCommand>();
    for (LabelElement* el : m_selection)
    {
        int nx = el->x, ny = el->y;
        switch (type)
        {
        case 0: nx = refLeft;              break;  // align left edges
        case 1: nx = refRight  - el->w;    break;  // align right edges
        case 2: ny = refTop;               break;  // align top edges
        case 3: ny = refBottom - el->h;    break;  // align bottom edges
        case 4: nx = c.marginLeft + (c.widthDots  - el->w) / 2; break;  // center H on label
        case 5: ny = c.marginTop  + (c.heightDots - el->h) / 2; break;  // center V on label
        }
        if (nx != el->x || ny != el->y)
            compound->Add(std::make_unique<MoveElementCommand>(
                el, wxPoint(el->x, el->y), wxPoint(nx, ny)));
    }
    if (!compound->Empty())
        ExecuteCommand(std::move(compound));
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

        // Draw rubber-band marquee on top of everything (client coords)
        if (m_marquee)
        {
            wxRect r(wxPoint(std::min(m_marqueeStart.x, m_marqueeEnd.x),
                             std::min(m_marqueeStart.y, m_marqueeEnd.y)),
                     wxPoint(std::max(m_marqueeStart.x, m_marqueeEnd.x),
                             std::max(m_marqueeStart.y, m_marqueeEnd.y)));
            dc.SetPen(wxPen(wxColour(0, 120, 215), 1, wxPENSTYLE_DOT));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle(r);
        }

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

    // Label-count badge (top-right corner) so user knows how many columns exist.
    int lpr = m_config.labelsPerRow;
    if (lpr > 1)
    {
        wxString badge = wxString::Format("x%d", lpr);
        dc.SetFont(wxFont(wxFontInfo(wxSize(0,11)).Family(wxFONTFAMILY_SWISS).Bold()));
        dc.SetTextForeground(wxColour(200,80,80));
        wxSize ext = dc.GetTextExtent(badge);
        dc.DrawText(badge, off.x + pw - ext.x - 4, off.y + 3);
    }
}

void LabelCanvas::DrawGrid(wxDC& dc)
{
    int gridDots = MMToDots(10.0, m_config.dpi);
    if (gridDots <= 0) return;
    double gridPx = gridDots * m_zoom;
    if (gridPx < 4.0) return;

    wxPoint off = LabelOffset();
    int pw = LabelPixelWidth();
    int ph = LabelPixelHeight();

    dc.SetPen(wxPen(wxColour(210, 210, 255), 1));
    for (double x = off.x; x <= off.x + pw; x += gridPx)
        dc.DrawLine(static_cast<int>(x), off.y, static_cast<int>(x), off.y + ph);
    for (double y = off.y; y <= off.y + ph; y += gridPx)
        dc.DrawLine(off.x, static_cast<int>(y), off.x + pw, static_cast<int>(y));
}

void LabelCanvas::DrawElements(wxDC& dc)
{
    for (auto& el : m_elements)
    {
        el->Draw(dc, LabelOffset(), m_zoom);
        if (el.get() == m_selected)
            DrawSelectionHandles(dc, el.get());
        else if (IsSelected(el.get()))
            DrawSelectionOutline(dc, el.get());
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

void LabelCanvas::DrawSelectionOutline(wxDC& dc, LabelElement* el)
{
    wxRect bounds = el->GetBoundsPixels(LabelOffset(), m_zoom);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(wxColour(0, 120, 215), 1, wxPENSTYLE_SHORT_DASH));
    dc.DrawRectangle(bounds);
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

LabelElement* LabelCanvas::HitTest(wxPoint clientPx) const
{
    int vx, vy;
    CalcUnscrolledPosition(clientPx.x, clientPx.y, &vx, &vy);
    wxPoint vpx(vx, vy);
    wxPoint off = LabelOffset();
    // Iterate in reverse (top element first)
    for (int i = static_cast<int>(m_elements.size()) - 1; i >= 0; --i)
    {
        wxRect b = m_elements[i]->GetBoundsPixels(off, m_zoom);
        if (b.Contains(vpx))
            return m_elements[i].get();
    }
    return nullptr;
}

int LabelCanvas::HitTestHandle(LabelElement* el, wxPoint clientPx) const
{
    if (!el) return -2;
    int vx, vy;
    CalcUnscrolledPosition(clientPx.x, clientPx.y, &vx, &vy);
    wxPoint vpx(vx, vy);
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
        wxRect hr(pts[i].x - HANDLE_HIT_RADIUS, pts[i].y - HANDLE_HIT_RADIUS,
                  HANDLE_HIT_RADIUS * 2,         HANDLE_HIT_RADIUS * 2);
        if (hr.Contains(vpx))
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
        // Convert client coords to virtual (unscrolled) coords for the label-bounds check.
        int vx, vy;
        CalcUnscrolledPosition(px.x, px.y, &vx, &vy);
        wxPoint vpx(vx, vy);
        wxPoint off = LabelOffset();  // already in virtual coords
        wxRect  labelRect(off.x, off.y, LabelPixelWidth(), LabelPixelHeight());
        if (labelRect.Contains(vpx))
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
    // Resize handles only test against the primary selected element
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
            if (!HasCapture()) CaptureMouse();
            return;
        }
    }

    LabelElement* hit = HitTest(px);

    if (evt.ControlDown())
    {
        // Ctrl+click: toggle element in / out of selection
        if (hit)
        {
            auto it = std::find(m_selection.begin(), m_selection.end(), hit);
            if (it != m_selection.end())
            {
                m_selection.erase(it);
                m_selected = m_selection.empty() ? nullptr : m_selection.back();
            }
            else
            {
                m_selection.push_back(hit);
                m_selected = hit;
            }
            NotifyChanged();
        }
        Refresh();
        return;
    }

    if (hit)
    {
        // Clicking on an element outside the current selection clears selection first
        if (!IsSelected(hit))
        {
            m_selection.clear();
            m_selection.push_back(hit);
        }
        m_selected     = hit;
        m_dragHandle   = -1;  // body drag
        m_dragging     = true;
        m_dragStartPx  = px;
        m_dragOrigDot  = wxPoint(hit->x, hit->y);
        m_dragOrigSize = wxSize(hit->w, hit->h);
        // Record original positions for ALL selected elements (multi-drag)
        m_dragOrigDots.clear();
        for (LabelElement* el : m_selection)
            m_dragOrigDots.push_back(wxPoint(el->x, el->y));
        if (!HasCapture()) CaptureMouse();
        NotifyChanged();
    }
    else
    {
        // Click on empty space — start rubber-band marquee selection
        m_selection.clear();
        m_selected = nullptr;
        m_marquee      = true;
        m_marqueeStart = px;
        m_marqueeEnd   = px;
        if (!HasCapture()) CaptureMouse();
        NotifyChanged();
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
            // ScrollWindow() override handles Freeze/Refresh/Thaw automatically.
            Scroll(m_panStartScroll.x - delta.x / xu,
                   m_panStartScroll.y - delta.y / yu);
        }
        return;
    }

    if (!m_dragging || !m_selected)
    {
        // Update marquee rect if we are in rubber-band mode
        if (m_marquee)
        {
            m_marqueeEnd = evt.GetPosition();
            Refresh();
        }
        evt.Skip();
        return;
    }

    wxPoint px    = evt.GetPosition();
    wxPoint delta = px - m_dragStartPx;
    int     ddx   = static_cast<int>(delta.x / m_zoom);
    int     ddy   = static_cast<int>(delta.y / m_zoom);

    if (m_dragHandle == -1)
    {
        // Body drag — move ALL selected elements together (snap based on primary)
        auto snapped = SnapDot(wxPoint(m_dragOrigDot.x + ddx, m_dragOrigDot.y + ddy));
        int dx = snapped.x - m_dragOrigDot.x;
        int dy = snapped.y - m_dragOrigDot.y;
        for (size_t i = 0; i < m_selection.size() && i < m_dragOrigDots.size(); ++i)
        {
            m_selection[i]->x = m_dragOrigDots[i].x + dx;
            m_selection[i]->y = m_dragOrigDots[i].y + dy;
        }
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

        if (nw > 4)
        {
            m_selected->x = nx;
            m_selected->w = nw;
            // Keep fieldBlockWidth in sync so Draw() doesn't override w on repaint
            if (auto* t = dynamic_cast<TextElement*>(m_selected); t && t->useFieldBlock)
                t->fieldBlockWidth = nw;
        }
        if (nh > 4) { m_selected->y = ny; m_selected->h = nh; }
    }
    Refresh();
}

void LabelCanvas::OnMouseUp(wxMouseEvent& evt)
{
    // ── Finish rubber-band marquee ────────────────────────────────────────────
    if (m_marquee)
    {
        if (HasCapture()) ReleaseMouse();
        m_marquee = false;

        // Build the selection rectangle in virtual (scrolled) canvas coords
        int ax, ay, bx, by;
        CalcUnscrolledPosition(m_marqueeStart.x, m_marqueeStart.y, &ax, &ay);
        CalcUnscrolledPosition(m_marqueeEnd.x,   m_marqueeEnd.y,   &bx, &by);
        wxRect selRect(wxPoint(std::min(ax, bx), std::min(ay, by)),
                       wxPoint(std::max(ax, bx), std::max(ay, by)));

        wxPoint off = LabelOffset();
        m_selection.clear();
        m_selected = nullptr;
        for (auto& el : m_elements)
        {
            wxRect b = el->GetBoundsPixels(off, m_zoom);
            if (selRect.Intersects(b))
            {
                m_selection.push_back(el.get());
                m_selected = el.get();
            }
        }
        Refresh();
        NotifyChanged();
        return;
    }

    if (!m_dragging || !m_selected) { evt.Skip(); return; }

    if (HasCapture()) ReleaseMouse();
    m_dragging = false;

    // Commit to undo history
    if (m_dragHandle == -1)
    {
        // Build a compound move command covering all elements that actually moved
        auto compound = std::make_unique<CompoundCommand>();
        for (size_t i = 0; i < m_selection.size() && i < m_dragOrigDots.size(); ++i)
        {
            auto orig = m_dragOrigDots[i];
            auto curr = wxPoint(m_selection[i]->x, m_selection[i]->y);
            if (orig.x != curr.x || orig.y != curr.y)
                compound->Add(std::make_unique<MoveElementCommand>(m_selection[i], orig, curr));
        }
        if (!compound->Empty())
            m_history->Execute(std::move(compound));
    }
    else
    {
        if (m_selected->w != m_dragOrigSize.x ||
            m_selected->h != m_dragOrigSize.y)
        {
            // Special case: resizing a field-block text element changes fieldBlockWidth
            if (auto* t = dynamic_cast<TextElement*>(m_selected);
                t && t->useFieldBlock && t->fieldBlockWidth > 0)
            {
                auto compound = std::make_unique<CompoundCommand>();
                // Position change (left-edge handles move x)
                wxPoint newPos(t->x, t->y);
                if (newPos != m_dragOrigDot)
                    compound->Add(std::make_unique<MoveElementCommand>(
                        m_selected, m_dragOrigDot, newPos));
                // fieldBlockWidth change
                int oldFBW = m_dragOrigSize.x; // original w == original fieldBlockWidth
                if (t->w != oldFBW)
                    compound->Add(std::make_unique<EditPropertyCommand<int>>(
                        [t]{ return t->fieldBlockWidth; },
                        [t](int v){ t->fieldBlockWidth = v; },
                        oldFBW, t->w));
                if (!compound->Empty())
                    m_history->Execute(std::move(compound));
            }
            else
            {
                // Once the user manually resizes a TextElement, stop auto-sizing it to text extent
                if (auto* t = dynamic_cast<TextElement*>(m_selected))
                    t->autoSize = false;

                auto cmd = std::make_unique<ResizeElementCommand>(
                    m_selected,
                    m_dragOrigDot, m_dragOrigSize,
                    wxPoint(m_selected->x, m_selected->y),
                    wxSize(m_selected->w, m_selected->h));
                m_history->Execute(std::move(cmd));
            }
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

void LabelCanvas::ScrollWindow(int dx, int dy, const wxRect* rect)
{
    // wxScrolledWindow::Scroll() / scrollbar messages all funnel through
    // here.  The base-class implementation calls ::ScrollWindow() (Win32)
    // which immediately BitBlts existing screen pixels to the new position,
    // carrying our ruler tick-marks with them — causing ghost artefacts.
    //
    // Solution: Freeze() suppresses all visual output (WM_SETREDRAW=FALSE),
    // let the base class update the scroll position and backing state, then
    // mark the entire client dirty and Thaw() to flush one clean repaint.
    // Because rulers are painted in fixed client-coords (device-origin reset
    // to 0,0 in OnPaint) that single repaint always shows them correctly.
    Freeze();
    wxScrolledWindow::ScrollWindow(dx, dy, rect);
    Refresh(false);
    Thaw();
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
    if (!m_selection.empty() && (key == WXK_LEFT || key == WXK_RIGHT ||
                                  key == WXK_UP   || key == WXK_DOWN))
    {
        int step = evt.ShiftDown() ? 1 : m_snapSize;
        int dx = 0, dy = 0;
        if (key == WXK_LEFT)  dx = -step;
        if (key == WXK_RIGHT) dx =  step;
        if (key == WXK_UP)    dy = -step;
        if (key == WXK_DOWN)  dy =  step;
        auto compound = std::make_unique<CompoundCommand>();
        for (LabelElement* el : m_selection)
            compound->Add(std::make_unique<MoveElementCommand>(
                el, wxPoint(el->x, el->y), wxPoint(el->x + dx, el->y + dy)));
        ExecuteCommand(std::move(compound));
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
        if (!m_selection.empty())
        {
            auto compound = std::make_unique<CompoundCommand>();
            for (LabelElement* selEl : m_selection)
            {
                for (auto& sp : m_elements)
                {
                    if (sp.get() == selEl)
                    {
                        compound->Add(std::make_unique<DeleteElementCommand>(m_elements, sp));
                        break;
                    }
                }
            }
            m_selection.clear();
            m_selected = nullptr;
            if (!compound->Empty())
                m_history->Execute(std::move(compound));
            NotifyChanged();
            Refresh();
            return;
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

    // Select element under cursor (or clear selection).
    // Right-clicking an already-selected element keeps the current selection.
    LabelElement* hit = HitTest(pt);
    if (hit)
    {
        if (!IsSelected(hit))
        {
            m_selection.clear();
            m_selection.push_back(hit);
            m_selected = hit;
            Refresh();
            NotifyChanged();
        }
        else
        {
            m_selected = hit;  // promote to primary within existing selection
        }
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
