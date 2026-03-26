#pragma once
#include "LabelElement.h"
#include <string>

class TextElement : public LabelElement
{
public:
    std::string text     = "Text";
    int         fontSize = 30;    // in dots (ZPL ^A font height)
    int         fontWidth = 30;  // in dots (ZPL ^A font width); kept in sync with fontSize when linked
    bool        fontSizeLinked = true;  // link height and width together
    std::string fontPath = "";   // printer font path for ZPL output (e.g. "E:ARIALR.FNT")
    std::string fontName = "";   // system font face name for canvas rendering (empty = default)
    bool        bold     = false;
    bool        italic   = false;
    int         rotation = 0;    // 0, 90, 180, 270

    // Field Block (^FB)
    bool useFieldBlock       = false;
    int  fieldBlockWidth     = 0;  // in dots; 0 = auto
    int  fieldBlockMaxLines  = 1;
    int  fieldBlockLineSpacing = 0;
    int  fieldBlockJustify   = 0;  // 0=L 1=R 2=C 3=J

    // When true, w/h are overwritten each paint from measured text extent.
    // Set to false by the user when they manually resize the element.
    bool autoSize = true;

    TextElement() { w = 200; h = 40; }

    void Draw(wxDC& dc, wxPoint off, double zoom) const override;
    std::string GetZPL() const override;
    std::unique_ptr<LabelElement> Clone() const override
    {
        return std::make_unique<TextElement>(*this);
    }
};
