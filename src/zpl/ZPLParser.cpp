#include "ZPLParser.h"
#include "../elements/TextElement.h"
#include "../elements/BarcodeElement.h"
#include "../elements/BoxElement.h"
#include "../elements/ImageElement.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>

// Minimal ZPL tokeniser: splits on ^ and ~ command boundaries.
// Handles the most common label commands needed to reconstruct element model.

static std::string trim(const std::string& s)
{
    auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

ParseResult ZPLParser::Parse(const std::string& zpl, PrinterDPI dpiHint)
{
    ParseResult res;
    res.config.dpi = dpiHint;  // caller-supplied; overridden below only if ZPL contains explicit cmd

    // State machine — we track the current ^FO position and build elements
    int foX = 0, foY = 0;
    bool inLabel = false;

    // Tokenise by splitting at ^ boundaries (keep ^ prefix)
    std::vector<std::string> tokens;
    {
        std::string cur;
        for (size_t i = 0; i < zpl.size(); ++i)
        {
            if (zpl[i] == '^' && !cur.empty())
            {
                tokens.push_back(trim(cur));
                cur.clear();
            }
            cur += zpl[i];
        }
        if (!cur.empty()) tokens.push_back(trim(cur));
    }

    for (const auto& tok : tokens)
    {
        if (tok.size() < 2) continue;
        std::string cmd = tok.substr(1, 2);
        std::string args = tok.size() > 3 ? tok.substr(3) : "";

        if (cmd == "XA") { inLabel = true; continue; }
        if (cmd == "XZ") { inLabel = false; continue; }
        if (!inLabel) continue;

        if (cmd == "PW")
        {
            res.config.widthDots = std::stoi(args);
        }
        else if (cmd == "LL")
        {
            res.config.heightDots = std::stoi(args);
        }
        else if (cmd == "LH")
        {
            auto comma = args.find(',');
            if (comma != std::string::npos)
            {
                res.config.marginLeft = std::stoi(args.substr(0, comma));
                res.config.marginTop  = std::stoi(args.substr(comma + 1));
            }
        }
        else if (cmd == "FO" || cmd == "FT")
        {
            auto comma = args.find(',');
            if (comma != std::string::npos)
            {
                foX = std::stoi(args.substr(0, comma));
                foY = std::stoi(args.substr(comma + 1));
            }
        }
        else if (cmd == "FD")
        {
            // Stand-alone ^FD without preceding barcode command → text element
            // (Barcode elements with ^FD are handled when barcode cmd is seen)
            auto el = std::make_shared<TextElement>();
            el->x    = foX;
            el->y    = foY;
            el->text = args;
            // Remove trailing ^FS if present
            auto fs = el->text.find("^FS");
            if (fs != std::string::npos) el->text = el->text.substr(0, fs);
            res.elements.push_back(std::move(el));
        }
        else if (cmd == "GB")
        {
            // ^GBwidth,height,thickness,colour,rounding
            std::istringstream ss(args);
            std::string part;
            std::vector<std::string> parts;
            while (std::getline(ss, part, ',')) parts.push_back(part);
            auto el = std::make_shared<BoxElement>();
            el->x    = foX;
            el->y    = foY;
            if (parts.size() >= 1) el->w = std::stoi(parts[0]);
            if (parts.size() >= 2) el->h = std::stoi(parts[1]);
            if (parts.size() >= 3) el->borderThick = std::stoi(parts[2]);
            res.elements.push_back(std::move(el));
        }
        else if (cmd == "BC" || cmd == "B3" || cmd == "BE" ||
                 cmd == "BU" || cmd == "BI" || cmd == "BQ")
        {
            auto el = std::make_shared<BarcodeElement>();
            el->x = foX;
            el->y = foY;
            if      (cmd == "BC") el->barcodeType = BarcodeType::Code128;
            else if (cmd == "B3") el->barcodeType = BarcodeType::Code39;
            else if (cmd == "BE") el->barcodeType = BarcodeType::EAN13;
            else if (cmd == "BU") el->barcodeType = BarcodeType::UPCA;
            else if (cmd == "BI") el->barcodeType = BarcodeType::I2of5;
            else if (cmd == "BQ") el->barcodeType = BarcodeType::QRCode;
            // Data will be filled when we see the following ^FD token
            // (Peek-ahead: next FD token belongs to this barcode)
            res.elements.push_back(std::move(el));
        }
    }

    // Second pass: match ^FD data to preceding barcode elements
    // (Simple approach: if last element added before an FD is a barcode, assign data)
    // This is already handled above for text; barcodes need post-process:
    // Re-do as a two-token look-ahead in the token list
    {
        std::vector<std::shared_ptr<LabelElement>> rebuilt;
        BarcodeElement* lastBarcode = nullptr;
        foX = foY = 0;

        // Pending font / field-block settings (consumed when ^FD creates a text element)
        std::string pFontPath = ""; int pFontH = 30; int pFontW = 0; int pRot = 0;
        bool pFB = false; int pFBW = 0; int pFBL = 1; int pFBS = 0; int pFBJ = 0;

        auto resetPending = [&]() {
            pFontPath = ""; pFontH = 30; pFontW = 0; pRot = 0;
            pFB = false; pFBW = 0; pFBL = 1; pFBS = 0; pFBJ = 0;
        };

        for (const auto& tok : tokens)
        {
            if (tok.size() < 2) continue;
            std::string cmd = tok.substr(1, 2);
            std::string args = tok.size() > 3 ? tok.substr(3) : "";

            if (cmd == "XA") { inLabel = true; continue; }
            if (cmd == "XZ") { lastBarcode = nullptr; continue; }

            if (cmd == "FO" || cmd == "FT")
            {
                auto comma = args.find(',');
                if (comma != std::string::npos)
                {
                    foX = std::stoi(args.substr(0, comma));
                    foY = std::stoi(args.substr(comma + 1));
                }
                lastBarcode = nullptr;
                resetPending();
            }
            else if (cmd == "A@" || cmd == "A0")
            {
                // ^A@orient,h,w,fontpath   or   ^A0orient,h,w
                if (!args.empty())
                {
                    char ori = args[0];
                    switch (ori) { case 'R': pRot=90; break; case 'I': pRot=180; break;
                                   case 'B': pRot=270; break; default: pRot=0; break; }
                    std::string rest = (args.size() > 1 && args[1] == ',') ? args.substr(2) : "";
                    // split first 3 comma-separated fields: h, w, [rest=fontpath]
                    for (int i = 0; i < 3 && !rest.empty(); ++i)
                    {
                        auto c = rest.find(',');
                        std::string seg = (c == std::string::npos) ? rest : rest.substr(0, c);
                        rest            = (c == std::string::npos) ? "" : rest.substr(c + 1);
                        try {
                            if (i == 0) pFontH = std::stoi(seg);
                            if (i == 1) pFontW = std::stoi(seg);
                        } catch (...) {}
                    }
                    if (cmd == "A@" && !rest.empty())
                    {
                        auto fs = rest.find("^FS"); if (fs != std::string::npos) rest = rest.substr(0, fs);
                        pFontPath = trim(rest);
                    } else { pFontPath = ""; }
                }
            }
            else if (cmd == "FB")
            {
                // ^FBwidth,maxlines,linespacing,justify,hanging
                std::istringstream ss(args); std::string seg;
                std::vector<std::string> parts;
                while (std::getline(ss, seg, ',')) parts.push_back(seg);
                try { if (parts.size() >= 1) pFBW = std::stoi(parts[0]); } catch (...) {}
                try { if (parts.size() >= 2) pFBL = std::stoi(parts[1]); } catch (...) {}
                try { if (parts.size() >= 3) pFBS = std::stoi(parts[2]); } catch (...) {}
                if (parts.size() >= 4 && !parts[3].empty())
                {
                    switch (parts[3][0]) { case 'R': pFBJ=1; break; case 'C': pFBJ=2; break;
                                          case 'J': pFBJ=3; break; default: pFBJ=0; break; }
                }
                pFB = true;
            }
            else if (cmd == "BC" || cmd == "B3" || cmd == "BE" ||
                     cmd == "BU" || cmd == "BI" || cmd == "BQ")
            {
                auto el = std::make_shared<BarcodeElement>();
                el->x = foX; el->y = foY;
                if      (cmd == "BC") el->barcodeType = BarcodeType::Code128;
                else if (cmd == "B3") el->barcodeType = BarcodeType::Code39;
                else if (cmd == "BE") el->barcodeType = BarcodeType::EAN13;
                else if (cmd == "BU") el->barcodeType = BarcodeType::UPCA;
                else if (cmd == "BI") el->barcodeType = BarcodeType::I2of5;
                else if (cmd == "BQ") el->barcodeType = BarcodeType::QRCode;
                lastBarcode = el.get();
                rebuilt.push_back(std::move(el));
            }
            else if (cmd == "FD")
            {
                std::string data = args;
                auto fs = data.find("^FS");
                if (fs != std::string::npos) data = data.substr(0, fs);

                if (lastBarcode)
                {
                    lastBarcode->data = data;
                    lastBarcode = nullptr;
                }
                else
                {
                    auto el = std::make_shared<TextElement>();
                    el->x        = foX;
                    el->y        = foY;
                    el->text     = data;
                    el->fontSize = pFontH;
                    el->fontWidth = pFontW;
                    el->fontPath = pFontPath;
                    el->rotation = pRot;
                    if (pFB)
                    {
                        el->useFieldBlock        = true;
                        el->fieldBlockWidth      = pFBW;
                        el->fieldBlockMaxLines   = pFBL;
                        el->fieldBlockLineSpacing = pFBS;
                        el->fieldBlockJustify    = pFBJ;
                    }
                    rebuilt.push_back(std::move(el));
                    resetPending();
                }
            }
            else if (cmd == "GB")
            {
                std::istringstream ss(args);
                std::string part;
                std::vector<std::string> parts;
                while (std::getline(ss, part, ',')) parts.push_back(part);
                auto el = std::make_shared<BoxElement>();
                el->x = foX; el->y = foY;
                if (parts.size() >= 1) el->w = std::stoi(parts[0]);
                if (parts.size() >= 2) el->h = std::stoi(parts[1]);
                if (parts.size() >= 3) el->borderThick = std::stoi(parts[2]);
                rebuilt.push_back(std::move(el));
                lastBarcode = nullptr;
            }
            else if (cmd == "GF")
            {
                // ^GFformat,dataBytes,totalBytes,rowBytes,data
                // Split only first 4 commas; rest is the data payload
                std::string rest = args;
                std::vector<std::string> parts;
                for (int i = 0; i < 4 && !rest.empty(); ++i)
                {
                    auto c = rest.find(',');
                    if (c == std::string::npos) { parts.push_back(rest); rest.clear(); break; }
                    parts.push_back(rest.substr(0, c));
                    rest = rest.substr(c + 1);
                }
                // parts[2]=totalBytes, parts[3]=rowBytes, rest=data
                if (parts.size() >= 4 && !rest.empty())
                {
                    try
                    {
                        int totalBytes = std::stoi(parts[2]);
                        int rowBytes   = std::stoi(parts[3]);
                        auto el = std::make_shared<ImageElement>();
                        el->x = foX; el->y = foY;
                        if (el->LoadFromZ64(rowBytes, totalBytes, rest))
                            rebuilt.push_back(std::move(el));
                    }
                    catch (...) {}
                }
                lastBarcode = nullptr;
            }
        }  // end for loop over tokens

        res.elements = std::move(rebuilt);
    }  // end second-pass block

    // Derive print area from total - margins
    res.config.widthDots  -= res.config.marginLeft + res.config.marginRight;
    res.config.heightDots -= res.config.marginTop  + res.config.marginBottom;
    if (res.config.widthDots  < 1) res.config.widthDots  = 800;
    if (res.config.heightDots < 1) res.config.heightDots = 1200;

    res.ok = true;
    return res;
}
