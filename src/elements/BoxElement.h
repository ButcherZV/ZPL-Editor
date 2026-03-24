#pragma once
#include "LabelElement.h"
#include <wx/colour.h>

enum class BoxShape
{
    Rectangle,   // ^GB
    Circle,      // ^GC
    DiagonalL,   // ^GD  (top-left to bottom-right)
    DiagonalR,   // ^GE  (bottom-left to top-right)
    Ellipse,     // ^GE with equal aspect — rendered as ellipse
};

class BoxElement : public LabelElement
{
public:
    BoxShape   shape         = BoxShape::Rectangle;
    int        borderThick   = 3;       // dots
    wxColour   borderColour  = *wxBLACK;
    wxColour   fillColour    = *wxWHITE;
    bool       filled        = false;

    BoxElement() { w = 200; h = 100; }

    void Draw(wxDC& dc, wxPoint off, double zoom) const override;
    std::string GetZPL() const override;
    std::unique_ptr<LabelElement> Clone() const override
    {
        return std::make_unique<BoxElement>(*this);
    }
};
