#pragma once
#include "LabelElement.h"
#include <string>

class TextElement : public LabelElement
{
public:
    std::string text     = "Text";
    int         fontSize = 30;    // in dots (ZPL ^A font height)
    int         fontWidth = 0;   // 0 = same as fontSize
    std::string fontPath = "";   // empty = built-in scalable ^A0
    bool        bold     = false;
    bool        italic   = false;
    int         rotation = 0;    // 0, 90, 180, 270

    // Field Block (^FB)
    bool useFieldBlock       = false;
    int  fieldBlockWidth     = 0;  // in dots; 0 = auto
    int  fieldBlockMaxLines  = 1;
    int  fieldBlockLineSpacing = 0;
    int  fieldBlockJustify   = 0;  // 0=L 1=R 2=C 3=J

    TextElement() { w = 200; h = 40; }

    void Draw(wxDC& dc, wxPoint off, double zoom) const override;
    std::string GetZPL() const override;
    std::unique_ptr<LabelElement> Clone() const override
    {
        return std::make_unique<TextElement>(*this);
    }
};
