#include "TextElement.h"
#include <sstream>
#include <cmath>

void TextElement::Draw(wxDC& dc, wxPoint off, double zoom) const
{
    int px = off.x + static_cast<int>(x * zoom);
    int py = off.y + static_cast<int>(y * zoom);
    int ph = std::max(8, static_cast<int>(fontSize * zoom));

    wxFontWeight fw = bold   ? wxFONTWEIGHT_BOLD   : wxFONTWEIGHT_NORMAL;
    wxFontStyle  fs = italic ? wxFONTSTYLE_ITALIC   : wxFONTSTYLE_NORMAL;
    wxFont font(ph, wxFONTFAMILY_MODERN, fs, fw);
    dc.SetFont(font);
    dc.SetTextForeground(*wxBLACK);
    dc.DrawText(wxString::FromUTF8(text), px, py);
}

std::string TextElement::GetZPL() const
{
    std::ostringstream oss;
    char orient = 'N';
    if (rotation == 90)  orient = 'R';
    if (rotation == 180) orient = 'I';
    if (rotation == 270) orient = 'B';

    int fw = (fontWidth > 0) ? fontWidth : fontSize;

    oss << "^FO" << x << "," << y;
    if (!fontPath.empty())
        oss << "^A@" << orient << "," << fontSize << "," << fw << "," << fontPath;
    else
        oss << "^A0" << orient << "," << fontSize << "," << fw;

    if (useFieldBlock)
    {
        static const char just[] = { 'L', 'R', 'C', 'J' };
        int j = std::max(0, std::min(3, fieldBlockJustify));
        oss << "^FB" << (fieldBlockWidth > 0 ? fieldBlockWidth : w)
            << "," << fieldBlockMaxLines
            << "," << fieldBlockLineSpacing
            << "," << just[j] << ",0";
    }

    oss << "^FD" << text << "^FS";
    return oss.str();
}
