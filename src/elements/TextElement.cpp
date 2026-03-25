#include "TextElement.h"
#include <sstream>
#include <cmath>
#include <vector>

void TextElement::Draw(wxDC& dc, wxPoint off, double zoom) const
{
    int px = off.x + static_cast<int>(x * zoom);
    int py = off.y + static_cast<int>(y * zoom);
    int ph = std::max(8, static_cast<int>(fontSize * zoom));

    wxFontWeight fw = bold   ? wxFONTWEIGHT_BOLD   : wxFONTWEIGHT_NORMAL;
    wxFontStyle  fs = italic ? wxFONTSTYLE_ITALIC   : wxFONTSTYLE_NORMAL;
    // ZPL fontSize is in printer dots; at any zoom level we want the rendered
    // glyph to be exactly ph *pixels* tall so that the dot↔pixel correspondence
    // is preserved.  wxFont(int, …) treats the first argument as POINT SIZE
    // (1 pt = 96/72 px at typical screen DPI), which makes text ~33 % taller
    // than intended and corrupts all baseline-corrected ^FT positions.
    // wxFontInfo(wxSize(0, ph)) uses pixel size instead.
    wxFont font;
    if (!fontName.empty())
    {
        font = wxFont(wxFontInfo(wxSize(0, ph))
                      .FaceName(wxString::FromUTF8(fontName))
                      .Style(fs).Weight(fw));
        if (!font.IsOk())
            font = wxFont(wxFontInfo(wxSize(0, ph))
                          .Family(wxFONTFAMILY_MODERN).Style(fs).Weight(fw));
    }
    else
    {
        font = wxFont(wxFontInfo(wxSize(0, ph))
                      .Family(wxFONTFAMILY_MODERN).Style(fs).Weight(fw));
    }
    dc.SetFont(font);
    dc.SetTextForeground(*wxBLACK);

    wxString wxText = wxString::FromUTF8(text);

    if (useFieldBlock && fieldBlockWidth > 0)
    {
        // Draw text with word-wrap and field-block justification.
        const int blockPx = static_cast<int>(fieldBlockWidth * zoom);

        // Measure a space so word-wrap can account for it.
        int spaceW = 0, spaceH = ph;
        dc.GetTextExtent(wxString(" "), &spaceW, &spaceH);
        if (spaceW <= 0) spaceW = ph / 4; // fallback

        // Split the raw text on explicit newlines to get paragraphs,
        // then word-wrap each paragraph into visual lines.
        struct VisLine { wxString text; bool paraLast; };
        std::vector<VisLine> visLines;

        wxArrayString paragraphs = wxSplit(wxText, '\n');
        for (size_t pi = 0; pi < paragraphs.size(); ++pi)
        {
            const wxString& para = paragraphs[pi];
            if (para.empty())
            {
                visLines.push_back({wxString(), true});
                continue;
            }

            wxArrayString words = wxSplit(para, ' ');
            wxString curLine;
            int curW = 0;

            for (size_t wi = 0; wi < words.size(); ++wi)
            {
                const wxString& word = words[wi];
                int wordW = 0, wordH = ph;
                if (!word.empty())
                    dc.GetTextExtent(word, &wordW, &wordH);

                if (curLine.empty())
                {
                    curLine = word;
                    curW    = wordW;
                }
                else if (curW + spaceW + wordW <= blockPx)
                {
                    curLine += ' ';
                    curLine += word;
                    curW    += spaceW + wordW;
                }
                else
                {
                    // Current line is full — wrap.
                    visLines.push_back({curLine, false});
                    curLine = word;
                    curW    = wordW;
                }
            }
            // Last line of this paragraph.
            visLines.push_back({curLine, true});
        }

        // Respect fieldBlockMaxLines (0 = unlimited).
        int maxL = (fieldBlockMaxLines > 0)
                   ? fieldBlockMaxLines
                   : static_cast<int>(visLines.size());

        int lineSpacingPx = static_cast<int>(fieldBlockLineSpacing * zoom);
        int lineY  = py;
        int totalH = 0;

        for (int li = 0; li < static_cast<int>(visLines.size()) && li < maxL; ++li)
        {
            const wxString& line     = visLines[li].text;
            const bool      paraLast = visLines[li].paraLast;

            int textW = 0, textH = ph;
            if (!line.empty())
                dc.GetTextExtent(line, &textW, &textH);
            else
                dc.GetTextExtent(wxString("A"), nullptr, &textH);

            if (fieldBlockJustify == 1) // Right
            {
                dc.DrawText(line, px + std::max(0, blockPx - textW), lineY);
            }
            else if (fieldBlockJustify == 2) // Center
            {
                dc.DrawText(line, px + std::max(0, (blockPx - textW) / 2), lineY);
            }
            else if (fieldBlockJustify == 3 && !paraLast) // Justified (non-last line)
            {
                wxArrayString lwords = wxSplit(line, ' ');
                int numWords = static_cast<int>(lwords.size());
                if (numWords > 1)
                {
                    int totalWordW = 0;
                    for (const wxString& w : lwords)
                    {
                        int ww, wh;
                        dc.GetTextExtent(w, &ww, &wh);
                        totalWordW += ww;
                    }
                    int totalGap    = std::max(0, blockPx - totalWordW);
                    int spacePerGap = totalGap / (numWords - 1);
                    int extra       = totalGap % (numWords - 1);
                    int wx2 = px;
                    for (int wi = 0; wi < numWords; ++wi)
                    {
                        dc.DrawText(lwords[wi], wx2, lineY);
                        int ww, wh;
                        dc.GetTextExtent(lwords[wi], &ww, &wh);
                        wx2 += ww + spacePerGap + (wi < extra ? 1 : 0);
                    }
                }
                else
                {
                    dc.DrawText(line, px, lineY);
                }
            }
            else // Left (0) or last line of Justified
            {
                dc.DrawText(line, px, lineY);
            }

            lineY  += textH + lineSpacingPx;
            totalH += textH + lineSpacingPx;
        }

        // Update bounding box: width always = fieldBlockWidth
        const_cast<TextElement*>(this)->w = fieldBlockWidth;
        if (autoSize && zoom > 0.0)
            const_cast<TextElement*>(this)->h = std::max(1, static_cast<int>(totalH / zoom));
    }
    else
    {
        dc.DrawText(wxText, px, py);

        if (autoSize)
        {
            wxSize ext;
            dc.GetTextExtent(wxText, &ext.x, &ext.y);
            if (zoom > 0.0)
            {
                const_cast<TextElement*>(this)->w = std::max(1, static_cast<int>(ext.x / zoom));
                const_cast<TextElement*>(this)->h = std::max(1, static_cast<int>(ext.y / zoom));
            }
        }
    }
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
