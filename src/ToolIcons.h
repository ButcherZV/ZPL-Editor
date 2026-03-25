#pragma once
#include <wx/wx.h>
#include <wx/dcmemory.h>
#include "ActiveTool.h"

// Generates a simple wxBitmap icon for each tool by drawing into a MemoryDC.
// Uses system button-face/text colours so it looks correct on any theme.
inline wxBitmap CreateToolIcon(ActiveTool tool, int sz = 24)
{
    wxBitmap bmp(sz, sz);
    {
        wxMemoryDC dc(bmp);

        wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
        wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
        wxColour ac = wxColour(0, 120, 215);   // accent blue

        dc.SetBackground(wxBrush(bg));
        dc.Clear();

        const int m  = sz / 7;        // margin
        const int w  = sz - 2 * m;    // working area width/height

        switch (tool)
        {
        // ── Arrow / Select ────────────────────────────────────────────────────
        case ActiveTool::Select:
        {
            dc.SetPen(wxPen(fg, 1));
            dc.SetBrush(wxBrush(fg));
            // ox / oy shift the whole icon inside the button.
            // Increase ox to move right, increase oy to move down.
            const int ox = 4, oy = 0;
            wxPoint pts[] = {
                wxPoint(ox+ 3, oy+ 2),   // tip
                wxPoint(ox+ 3, oy+19),   // bottom-left of stem
                wxPoint(ox+ 7, oy+15),   // inner notch  (where tail meets stem)
                wxPoint(ox+10, oy+21),   // tail bottom
                wxPoint(ox+12, oy+20),   // tail right
                wxPoint(ox+ 9, oy+14),   // tail inner join
                wxPoint(ox+12, oy+11),   // right shoulder
            };
            dc.DrawPolygon(WXSIZEOF(pts), pts);
            break;
        }

        // ── Text ──────────────────────────────────────────────────────────────
        case ActiveTool::Text:
        {
            int thick = std::max(2, sz / 10);
            dc.SetPen(wxPen(ac, thick, wxPENSTYLE_SOLID));
            // Horizontal bar of the T
            dc.DrawLine(m,       m + thick/2, m + w,     m + thick/2);
            // Vertical stem
            dc.DrawLine(m + w/2, m + thick/2, m + w/2,  m + w);
            break;
        }

        // ── Barcode ───────────────────────────────────────────────────────────
        case ActiveTool::Barcode:
        {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(fg));
            // widths pattern: narrow/wide bars
            static const int kW[] = { 1, 2, 1, 3, 1, 2, 1, 2, 1 };
            int x = m;
            for (int bw : kW)
            {
                if (x + bw > m + w) break;
                dc.DrawRectangle(x, m, bw, w);
                x += bw + 1;
            }
            break;
        }

        // ── Box / Rectangle ───────────────────────────────────────────────────
        case ActiveTool::Box:
        {
            int thick = std::max(2, sz / 10);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(wxPen(ac, thick));
            dc.DrawRectangle(m, m, w, w);
            break;
        }

        // ── Image ────────────────────────────────────────────────────────────
        case ActiveTool::Image:
        {
            // Outer frame
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(wxPen(fg, 1));
            dc.DrawRectangle(m, m, w, w);

            // Mountain triangle (filled, accent)
            dc.SetBrush(wxBrush(ac));
            dc.SetPen(*wxTRANSPARENT_PEN);
            wxPoint mtn[] = {
                { m + w * 1/6,  m + w - 1  },
                { m + w * 1/2,  m + w * 1/3 },
                { m + w * 5/6,  m + w - 1  },
            };
            dc.DrawPolygon(3, mtn);

            // Sun (small filled circle)
            dc.SetBrush(wxBrush(fg));
            dc.DrawCircle(m + w * 3/4, m + w * 1/4 + 1, std::max(2, w / 8));
            break;
        }
        }
    }
    return bmp;
}

// ── Main toolbar icons (New / Open / Save / Align variants) ──────────────────

enum class MainToolbarIcon
{
    New,
    Open,
    Save,
    Undo,
    Redo,
    AlignLeft,
    AlignRight,
    AlignTop,
    AlignBottom,
    AlignCenterH,
    AlignCenterV,
    ZoomIn,
    ZoomOut,
    ZoomFit,
};

inline wxBitmap CreateMainToolbarIcon(MainToolbarIcon icon, int sz = 24)
{
    const wxColour bg  = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    const wxColour ink(30, 30, 30);
    const wxColour paper(255, 255, 255);
    const wxColour fold(200, 200, 200);
    const wxColour rectA(100, 160, 220);
    const wxColour rectB(140, 200, 100);
    const wxColour guide(210, 40, 40);

    wxBitmap bmp(sz, sz, 24);
    {
        wxMemoryDC dc(bmp);
        dc.SetBackground(wxBrush(bg));
        dc.Clear();

        const wxPen thinInk(ink, 1);
        const wxPen guidePen(guide, 2);

        switch (icon)
        {
        case MainToolbarIcon::New:
        {
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(paper));
            wxPoint page[] = { {4,2},{17,2},{22,7},{22,22},{4,22} };
            dc.DrawPolygon(5, page);
            dc.SetBrush(wxBrush(fold));
            wxPoint tri[] = { {17,2},{22,7},{17,7} };
            dc.DrawPolygon(3, tri);
            dc.SetPen(thinInk);
            dc.DrawLine(17,2,17,7); dc.DrawLine(17,7,22,7);
            dc.SetPen(wxPen(ink, 1));
            dc.DrawLine(7,11,19,11);
            dc.DrawLine(7,14,19,14);
            dc.DrawLine(7,17,15,17);
            break;
        }
        case MainToolbarIcon::Open:
        {
            const wxColour folderBack(255, 193, 60);
            const wxColour folderFront(255, 213, 100);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(folderBack));
            dc.DrawRectangle(2, 9, 20, 12);
            dc.DrawRoundedRectangle(2, 6, 9, 5, 1);
            dc.SetBrush(wxBrush(folderFront));
            wxPoint flap[] = { {2,10},{22,10},{20,21},{4,21} };
            dc.DrawPolygon(4, flap);
            dc.SetPen(thinInk);
            dc.DrawLine(2,10,22,10);
            break;
        }
        case MainToolbarIcon::Save:
        {
            const wxColour diskBody(80, 80, 80);
            const wxColour diskLabel(230, 230, 250);
            const wxColour diskSlot(50, 50, 50);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(diskBody));
            wxPoint body[] = { {3,2},{18,2},{21,5},{21,22},{3,22} };
            dc.DrawPolygon(5, body);
            dc.SetBrush(wxBrush(diskLabel));
            dc.DrawRectangle(6, 3, 12, 8);
            dc.SetPen(wxPen(diskSlot, 1));
            dc.DrawLine(9,3,9,11);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(wxColour(140,140,140)));
            dc.DrawRectangle(5, 14, 14, 7);
            dc.SetPen(wxPen(wxColour(80,80,80), 1));
            dc.DrawLine(9,14,9,21); dc.DrawLine(13,14,13,21);
            break;
        }
        case MainToolbarIcon::Undo:
        {
            // Counter-clockwise curved arrow
            const wxColour arrowCol(0, 100, 210);
            dc.SetPen(wxPen(arrowCol, 2));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            // Arc: left half of a circle, top portion
            dc.DrawArc(18, 12,  6, 12,  12, 12);  // CW arc from right to left across top
            // Arrowhead pointing down-left
            dc.SetBrush(wxBrush(arrowCol));
            dc.SetPen(*wxTRANSPARENT_PEN);
            wxPoint head[] = { {4,10},{10,10},{7,16} };
            dc.DrawPolygon(3, head);
            break;
        }
        case MainToolbarIcon::Redo:
        {
            // Clockwise curved arrow (mirror of Undo)
            const wxColour arrowCol(0, 100, 210);
            dc.SetPen(wxPen(arrowCol, 2));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawArc(6, 12,  18, 12,  12, 12);  // arc from left to right across top
            // Arrowhead pointing down-right
            dc.SetBrush(wxBrush(arrowCol));
            dc.SetPen(*wxTRANSPARENT_PEN);
            wxPoint head[] = { {14,10},{20,10},{17,16} };
            dc.DrawPolygon(3, head);
            break;
        }
        case MainToolbarIcon::AlignLeft:
            dc.SetPen(guidePen); dc.DrawLine(4,2,4,22);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(rectA)); dc.DrawRectangle(4,4,10,6);
            dc.SetBrush(wxBrush(rectB)); dc.DrawRectangle(4,13,15,5);
            break;
        case MainToolbarIcon::AlignRight:
            dc.SetPen(guidePen); dc.DrawLine(20,2,20,22);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(rectA)); dc.DrawRectangle(10,4,10,6);
            dc.SetBrush(wxBrush(rectB)); dc.DrawRectangle(5,13,15,5);
            break;
        case MainToolbarIcon::AlignTop:
            dc.SetPen(guidePen); dc.DrawLine(2,4,22,4);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(rectA)); dc.DrawRectangle(4,4,6,10);
            dc.SetBrush(wxBrush(rectB)); dc.DrawRectangle(13,4,5,15);
            break;
        case MainToolbarIcon::AlignBottom:
            dc.SetPen(guidePen); dc.DrawLine(2,20,22,20);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(rectA)); dc.DrawRectangle(4,10,6,10);
            dc.SetBrush(wxBrush(rectB)); dc.DrawRectangle(13,5,5,15);
            break;
        case MainToolbarIcon::AlignCenterH:
            dc.SetPen(guidePen); dc.DrawLine(12,2,12,22);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(rectA)); dc.DrawRectangle(7,4,10,6);
            dc.SetBrush(wxBrush(rectB)); dc.DrawRectangle(5,13,14,5);
            break;
        case MainToolbarIcon::AlignCenterV:
            dc.SetPen(guidePen); dc.DrawLine(2,12,22,12);
            dc.SetPen(thinInk);
            dc.SetBrush(wxBrush(rectA)); dc.DrawRectangle(4,7,6,10);
            dc.SetBrush(wxBrush(rectB)); dc.DrawRectangle(13,5,5,14);
            break;
        case MainToolbarIcon::ZoomIn:
        {
            // Magnifying glass
            dc.SetPen(wxPen(ink, 2));
            dc.SetBrush(wxBrush(paper));
            dc.DrawCircle(9, 9, 6);
            // + inside
            dc.SetPen(wxPen(ink, 2));
            dc.DrawLine(6, 9, 12, 9);
            dc.DrawLine(9, 6, 9, 12);
            // handle
            dc.DrawLine(14, 14, 21, 21);
            break;
        }
        case MainToolbarIcon::ZoomOut:
        {
            dc.SetPen(wxPen(ink, 2));
            dc.SetBrush(wxBrush(paper));
            dc.DrawCircle(9, 9, 6);
            // - inside
            dc.SetPen(wxPen(ink, 2));
            dc.DrawLine(6, 9, 12, 9);
            // handle
            dc.DrawLine(14, 14, 21, 21);
            break;
        }
        case MainToolbarIcon::ZoomFit:
        {
            dc.SetPen(wxPen(ink, 2));
            dc.SetBrush(wxBrush(paper));
            dc.DrawCircle(9, 9, 6);
            // Expand arrows inside (two corner-arrows NW and SE)
            dc.SetPen(wxPen(ink, 1));
            dc.DrawLine(8, 8, 6, 6);
            dc.DrawLine(6, 6, 8, 6);
            dc.DrawLine(6, 6, 6, 8);
            dc.DrawLine(10, 10, 12, 12);
            dc.DrawLine(12, 12, 10, 12);
            dc.DrawLine(12, 12, 12, 10);
            // handle
            dc.SetPen(wxPen(ink, 2));
            dc.DrawLine(14, 14, 21, 21);
            break;
        }
        }
    }
    return bmp;
}
