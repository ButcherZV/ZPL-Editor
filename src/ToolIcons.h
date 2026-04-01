#pragma once
#include <wx/wx.h>
#include <wx/bmpbndl.h>
#include "ActiveTool.h"

// All toolbar icons are rendered from embedded Lucide SVG strings via
// wxBitmapBundle::FromSVG (NanoSVG backend in wxWidgets 3.2).
// "currentColor" is replaced with "#1e1e1e" so NanoSVG renders the strokes.

namespace {

// ── Tool-button SVGs ─────────────────────────────────────────────────────────

// Move/Select Tool
static const char kSvgSelect[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M4.037 4.688a.495.495 0 0 1 .651-.651l16 6.5a.5.5 0 0 1-.063.947l-6.124 1.58a2 2 0 0 0-1.438 1.435l-1.579 6.126a.5.5 0 0 1-.947.063z"/>
</svg>)svg";

// Text Element
static const char kSvgText[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M12 4v16"/>
  <path d="M4 7V5a1 1 0 0 1 1-1h14a1 1 0 0 1 1 1v2"/>
  <path d="M9 20h6"/>
</svg>)svg";

// Barcode Element
static const char kSvgBarcode[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M3 5v14"/>
  <path d="M8 5v14"/>
  <path d="M12 5v14"/>
  <path d="M17 5v14"/>
  <path d="M21 5v14"/>
</svg>)svg";

// Box Element
static const char kSvgBox[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <rect width="18" height="18" x="3" y="3" rx="2"/>
</svg>)svg";

// Image Element
static const char kSvgImage[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <rect width="18" height="18" x="3" y="3" rx="2" ry="2"/>
  <circle cx="9" cy="9" r="2"/>
  <path d="m21 15-3.086-3.086a2 2 0 0 0-2.828 0L6 21"/>
</svg>)svg";

// ── Main-toolbar SVGs ─────────────────────────────────────────────────────────

static const char kSvgNew[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M6 22a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h8a2.4 2.4 0 0 1 1.704.706l3.588 3.588A2.4 2.4 0 0 1 20 8v12a2 2 0 0 1-2 2z"/>
  <path d="M14 2v5a1 1 0 0 0 1 1h5"/>
  <path d="M10 9H8"/>
  <path d="M16 13H8"/>
  <path d="M16 17H8"/>
</svg>)svg";

// Open
static const char kSvgOpen[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="m6 14 1.5-2.9A2 2 0 0 1 9.24 10H20a2 2 0 0 1 1.94 2.5l-1.54 6a2 2 0 0 1-1.95 1.5H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h3.9a2 2 0 0 1 1.69.9l.81 1.2a2 2 0 0 0 1.67.9H18a2 2 0 0 1 2 2v2"/>
</svg>)svg";

// Save
static const char kSvgSave[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M15.2 3a2 2 0 0 1 1.4.6l3.8 3.8a2 2 0 0 1 .6 1.4V19a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2z"/>
  <path d="M17 21v-7a1 1 0 0 0-1-1H8a1 1 0 0 0-1 1v7"/>
  <path d="M7 3v4a1 1 0 0 0 1 1h7"/>
</svg>)svg";

// Undo
static const char kSvgUndo[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M3 12a9 9 0 1 0 9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/>
  <path d="M3 3v5h5"/>
</svg>)svg";

// Redo
static const char kSvgRedo[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M21 12a9 9 0 1 1-9-9c2.52 0 4.93 1 6.74 2.74L21 8"/>
  <path d="M21 3v5h-5"/>
</svg>)svg";

// Align Left  — lucide-align-start-vertical (vertical guide on left)
static const char kSvgAlignLeft[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <rect width="9" height="6" x="6" y="14" rx="2"/>
  <rect width="16" height="6" x="6" y="4" rx="2"/>
  <path d="M2 2v20"/>
</svg>)svg";

// Align Right — lucide-align-end-vertical (vertical guide on right)
static const char kSvgAlignRight[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <rect width="16" height="6" x="2" y="4" rx="2"/>
  <rect width="9" height="6" x="9" y="14" rx="2"/>
  <path d="M22 22V2"/>
</svg>)svg";

// Align Top — lucide-align-start-horizontal (horizontal guide on top)
static const char kSvgAlignTop[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <rect width="6" height="16" x="4" y="6" rx="2"/>
  <rect width="6" height="9" x="14" y="6" rx="2"/>
  <path d="M22 2H2"/>
</svg>)svg";

// Align Bottom — lucide-align-end-horizontal (horizontal guide on bottom)
static const char kSvgAlignBottom[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <rect width="6" height="16" x="4" y="2" rx="2"/>
  <rect width="6" height="9" x="14" y="9" rx="2"/>
  <path d="M22 22H2"/>
</svg>)svg";

// Align Center Horizontal — lucide-square-centerline-dashed-horizontal
static const char kSvgAlignCenterH[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M8 3H5a2 2 0 0 0-2 2v14c0 1.1.9 2 2 2h3"/>
  <path d="M16 3h3a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2h-3"/>
  <path d="M12 20v2"/>
  <path d="M12 14v2"/>
  <path d="M12 8v2"/>
  <path d="M12 2v2"/>
</svg>)svg";

// Align Center Vertical — lucide-square-centerline-dashed-vertical
static const char kSvgAlignCenterV[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M21 8V5a2 2 0 0 0-2-2H5a2 2 0 0 0-2 2v3"/>
  <path d="M21 16v3a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-3"/>
  <path d="M4 12H2"/>
  <path d="M10 12H8"/>
  <path d="M16 12h-2"/>
  <path d="M22 12h-2"/>
</svg>)svg";

// Align Elements Horizontal Centers — lucide-align-center-horizontal
static const char kSvgAlignElCenterH[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M2 12h20"/>
  <path d="M10 4H7a2 2 0 0 0-2 2v4c0 1.1.9 2 2 2h3"/>
  <path d="M14 4h3a2 2 0 0 1 2 2v4a2 2 0 0 1-2 2h-3"/>
  <path d="M4 20h4a2 2 0 0 0 2-2v-1"/>
  <path d="M20 20h-4a2 2 0 0 1-2-2v-1"/>
</svg>)svg";

// Align Elements Vertical Centers — lucide-align-center-vertical
static const char kSvgAlignElCenterV[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M12 2v20"/>
  <path d="M8 10H4a2 2 0 0 1-2-2V6c0-1.1.9-2 2-2h4"/>
  <path d="M16 10h4a2 2 0 0 0 2-2V6a2 2 0 0 0-2-2h-4"/>
  <path d="M8 20H7a2 2 0 0 1-2-2v-2c0-1.1.9-2 2-2h1"/>
  <path d="M16 14h1a2 2 0 0 1 2 2v2a2 2 0 0 1-2 2h-1"/>
</svg>)svg";

// Zoom in
static const char kSvgZoomIn[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <circle cx="11" cy="11" r="8"/>
  <line x1="21" x2="16.65" y1="21" y2="16.65"/>
  <line x1="11" x2="11" y1="8" y2="14"/>
  <line x1="8" x2="14" y1="11" y2="11"/>
</svg>)svg";

// Zoom out
static const char kSvgZoomOut[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <circle cx="11" cy="11" r="8"/>
  <line x1="21" x2="16.65" y1="21" y2="16.65"/>
  <line x1="8" x2="14" y1="11" y2="11"/>
</svg>)svg";

// Zoom to Fit
static const char kSvgZoomFit[] = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24"
     fill="none" stroke="#1e1e1e" stroke-width="1.5"
     stroke-linecap="round" stroke-linejoin="round">
  <path d="M3 7V5a2 2 0 0 1 2-2h2"/>
  <path d="M17 3h2a2 2 0 0 1 2 2v2"/>
  <path d="M21 17v2a2 2 0 0 1-2 2h-2"/>
  <path d="M7 21H5a2 2 0 0 1-2-2v-2"/>
  <circle cx="12" cy="12" r="3"/>
  <path d="m16 16-1.9-1.9"/>
</svg>)svg";

}

// Returns a wxBitmapBundle from an embedded SVG string.
// Composites onto button-face background at 24 and 48 px so the toolbar gets
// opaque bitmaps and HiDPI displays get a sharp 2× version.
inline wxBitmapBundle BundleFromSvg(const char* svgData)
{
    auto makeComposited = [&](int sz) -> wxBitmap {
        wxBitmap svgBmp = wxBitmapBundle::FromSVG(svgData, wxSize(24, 24)).GetBitmap(wxSize(sz, sz));
        wxBitmap result(sz, sz, 24);
        {
            wxMemoryDC dc(result);
            dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
            dc.Clear();
            dc.DrawBitmap(svgBmp, 0, 0, true);
        }
        return result;
    };
    wxVector<wxBitmap> bitmaps;
    bitmaps.push_back(makeComposited(24));
    bitmaps.push_back(makeComposited(48));
    return wxBitmapBundle::FromBitmaps(bitmaps);
}

// ── Tool-button icons ─────────────────────────────────────────────────────────

inline wxBitmapBundle BundleToolIcon(ActiveTool tool)
{
    switch (tool)
    {
    case ActiveTool::Select:  return BundleFromSvg(kSvgSelect);
    case ActiveTool::Text:    return BundleFromSvg(kSvgText);
    case ActiveTool::Barcode: return BundleFromSvg(kSvgBarcode);
    case ActiveTool::Box:     return BundleFromSvg(kSvgBox);
    case ActiveTool::Image:   return BundleFromSvg(kSvgImage);
    default:                  return wxBitmapBundle();
    }
}

// ── Main-toolbar icons ────────────────────────────────────────────────────────

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
    AlignElCenterH,
    AlignElCenterV,
    ZoomIn,
    ZoomOut,
    ZoomFit,
};

inline wxBitmapBundle BundleMainToolbarIcon(MainToolbarIcon icon)
{
    switch (icon)
    {
    case MainToolbarIcon::New:          return BundleFromSvg(kSvgNew);
    case MainToolbarIcon::Open:         return BundleFromSvg(kSvgOpen);
    case MainToolbarIcon::Save:         return BundleFromSvg(kSvgSave);
    case MainToolbarIcon::Undo:         return BundleFromSvg(kSvgUndo);
    case MainToolbarIcon::Redo:         return BundleFromSvg(kSvgRedo);
    case MainToolbarIcon::AlignLeft:    return BundleFromSvg(kSvgAlignLeft);
    case MainToolbarIcon::AlignRight:   return BundleFromSvg(kSvgAlignRight);
    case MainToolbarIcon::AlignTop:     return BundleFromSvg(kSvgAlignTop);
    case MainToolbarIcon::AlignBottom:  return BundleFromSvg(kSvgAlignBottom);
    case MainToolbarIcon::AlignCenterH: return BundleFromSvg(kSvgAlignCenterH);
    case MainToolbarIcon::AlignCenterV: return BundleFromSvg(kSvgAlignCenterV);
    case MainToolbarIcon::AlignElCenterH: return BundleFromSvg(kSvgAlignElCenterH);
    case MainToolbarIcon::AlignElCenterV: return BundleFromSvg(kSvgAlignElCenterV);
    case MainToolbarIcon::ZoomIn:       return BundleFromSvg(kSvgZoomIn);
    case MainToolbarIcon::ZoomOut:      return BundleFromSvg(kSvgZoomOut);
    case MainToolbarIcon::ZoomFit:      return BundleFromSvg(kSvgZoomFit);
    default:                            return wxBitmapBundle();
    }
}
