#include "BoxElement.h"
#include <sstream>
#include <cmath>

void BoxElement::Draw(wxDC& dc, wxPoint off, double zoom) const
{
    int px = off.x + static_cast<int>(x * zoom);
    int py = off.y + static_cast<int>(y * zoom);
    int pw = std::max(2, static_cast<int>(w * zoom));
    int ph = std::max(2, static_cast<int>(h * zoom));
    int pt = std::max(1, static_cast<int>(borderThick * zoom));

    dc.SetPen(wxPen(borderColour, pt));
    dc.SetBrush(filled ? wxBrush(fillColour) : *wxTRANSPARENT_BRUSH);

    switch (shape)
    {
    case BoxShape::Rectangle:
        dc.DrawRectangle(px, py, pw, ph);
        break;
    case BoxShape::Circle:
    case BoxShape::Ellipse:
        dc.DrawEllipse(px, py, pw, ph);
        break;
    case BoxShape::DiagonalL:
        dc.DrawLine(px, py, px + pw, py + ph);
        break;
    case BoxShape::DiagonalR:
        dc.DrawLine(px + pw, py, px, py + ph);
        break;
    }
}

std::string BoxElement::GetZPL() const
{
    std::ostringstream oss;
    oss << "^FO" << x << "," << y;

    // ZPL colour: B=black, W=white
    char col = borderColour.GetLuminance() < 0.5f ? 'B' : 'W';

    switch (shape)
    {
    case BoxShape::Rectangle:
        oss << "^GB" << w << "," << h << "," << borderThick << "," << col;
        break;
    case BoxShape::Circle:
        oss << "^GC" << std::min(w, h) << "," << borderThick << "," << col;
        break;
    case BoxShape::DiagonalL:
        oss << "^GD" << w << "," << h << "," << borderThick << "," << col;
        break;
    case BoxShape::DiagonalR:
        oss << "^GE" << w << "," << h << "," << borderThick << "," << col;
        break;
    case BoxShape::Ellipse:
        oss << "^GE" << w << "," << h << "," << borderThick << "," << col;
        break;
    }
    oss << "^FS";
    return oss.str();
}
