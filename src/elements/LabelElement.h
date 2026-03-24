#pragma once
#include <wx/wx.h>
#include <string>
#include <memory>

// Abstract base for all label elements.  All positions/sizes are in dots.
class LabelElement
{
public:
    int x = 0, y = 0;   // position in dots (top-left)
    int w = 0, h = 0;   // size in dots

    virtual ~LabelElement() = default;

    // Draw onto the canvas DC. off = label pixel offset, zoom = current zoom.
    virtual void Draw(wxDC& dc, wxPoint labelOffset, double zoom) const = 0;

    // Return the ZPL string that represents this element.
    virtual std::string GetZPL() const = 0;

    // Return a deep copy.
    virtual std::unique_ptr<LabelElement> Clone() const = 0;

    // Bounding rectangle in pixel coords on the canvas.
    wxRect GetBoundsPixels(wxPoint labelOffset, double zoom) const
    {
        return wxRect(
            labelOffset.x + static_cast<int>(x * zoom),
            labelOffset.y + static_cast<int>(y * zoom),
            std::max(1, static_cast<int>(w * zoom)),
            std::max(1, static_cast<int>(h * zoom)));
    }
};
