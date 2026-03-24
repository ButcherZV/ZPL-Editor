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
            // Simple arrow-tip triangle pointing top-left
            wxPoint pts[] = {
                wxPoint(m,         m        ),   // tip
                wxPoint(m,         m + w    ),   // bottom-left
                wxPoint(m + w,     m        ),   // top-right
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
