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

static std::string GuessFontName(const std::string& printerFontPath)
{
    if (printerFontPath.empty()) return "";
    std::string name = printerFontPath;
    // Strip drive/path prefix (e.g. "E:" or "R:")
    auto colon = name.rfind(':');
    if (colon != std::string::npos) name = name.substr(colon + 1);
    // Strip extension
    auto dot = name.rfind('.');
    if (dot != std::string::npos) name = name.substr(0, dot);
    // Uppercase for lookup
    std::string upper = name;
    for (auto& c : upper) c = static_cast<char>(std::toupper(c));
    // Known printer-font → system-font mappings
    static const std::pair<const char*, const char*> kMap[] = {
        {"ARIALR",  "Arial"}, {"ARIALB",  "Arial"}, {"ARIALI",  "Arial"}, {"ARIALBI", "Arial"},
        {"ARIAL",   "Arial"}, {"ARIALU",  "Arial Unicode MS"},
        {"TIMESNR", "Times New Roman"}, {"TIMESN",  "Times New Roman"},
        {"COURR",   "Courier New"},     {"COUR",    "Courier New"},
        {"HELV",    "Helvetica"},        {"VERD",    "Verdana"},
        {"CALIBRI", "Calibri"},          {"GOTHIC",  "Century Gothic"},
        {"CAMBRIA", "Cambria"},          {"TAHOMA",  "Tahoma"},
    };
    for (auto& [key, val] : kMap)
        if (upper == key) return val;
    // Fallback: strip trailing R/B/I suffix then title-case
    if (!upper.empty() && (upper.back() == 'R' || upper.back() == 'B' || upper.back() == 'I'))
        upper.pop_back();
    if (!upper.empty())
    {
        std::string result;
        result += static_cast<char>(std::toupper(upper[0]));
        for (size_t i = 1; i < upper.size(); ++i)
            result += static_cast<char>(std::tolower(upper[i]));
        return result;
    }
    return "";
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

        // ^BY sets module/bar height and persists for the whole label.
        int pByHeight = 100;

        // Pending font / field-block settings (consumed when ^FD creates a text element)
        std::string pFontPath = ""; int pFontH = 30; int pFontW = 0; int pRot = 0;
        bool pFB = false; int pFBW = 0; int pFBL = 1; int pFBS = 0; int pFBJ = 0;
        bool pFoIsBaseline = false;
        int  pBcHeight = -1;   // -1 = use pByHeight; set by explicit ^BC h param
        bool pBcShowText = true;

        auto resetPending = [&]() {
            pFontPath = ""; pFontH = 30; pFontW = 0; pRot = 0;
            pFB = false; pFBW = 0; pFBL = 1; pFBS = 0; pFBJ = 0;
            pFoIsBaseline = false;
            pBcHeight = -1; pBcShowText = true;
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
                pFoIsBaseline = (cmd == "FT");
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
                    // split first 3 comma-separated fields: h, w, [fontpath for A@]
                    for (int i = 0; i < 3 && !rest.empty(); ++i)
                    {
                        auto c = rest.find(',');
                        std::string seg = (c == std::string::npos) ? rest : rest.substr(0, c);
                        rest            = (c == std::string::npos) ? "" : rest.substr(c + 1);
                        try {
                            if (i == 0) pFontH = std::stoi(seg);
                            if (i == 1) pFontW = std::stoi(seg);
                            if (i == 2 && cmd == "A@") pFontPath = trim(seg); // capture font path
                        } catch (...) {}
                    }
                    // If font path contained commas (unusual), rest holds the remainder
                    if (cmd == "A@" && !rest.empty())
                    {
                        auto fs = rest.find("^FS"); if (fs != std::string::npos) rest = rest.substr(0, fs);
                        pFontPath = trim(rest);
                    }
                    else if (cmd == "A0") { pFontPath = ""; }
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
            else if (cmd == "BY")
            {
                // ^BYw,r,h  — h (3rd param) is the default bar height in dots
                std::istringstream ss(args); std::string seg;
                std::vector<std::string> parts;
                while (std::getline(ss, seg, ',')) parts.push_back(seg);
                try { if (parts.size() >= 3 && !parts[2].empty()) pByHeight = std::stoi(parts[2]); }
                catch (...) {}
            }
            else if (cmd == "BC" || cmd == "B3" || cmd == "BE" ||
                     cmd == "BU" || cmd == "BI" || cmd == "BQ")
            {
                // Parse orientation and, for most types, explicit bar height + showText.
                // ^BCo,h,f,g,e  |  ^B3o,e,h,f,g  |  ^BEo,h,f,g  |  etc.
                // We use a unified approach: split args, orientation is [0], then h is
                // at index 1 for BC/BE/BU/BI and index 2 for B3.
                {
                    std::istringstream ss(args); std::string seg;
                    std::vector<std::string> parts;
                    while (std::getline(ss, seg, ',')) parts.push_back(seg);
                    int hIdx = (cmd == "B3") ? 2 : 1;   // height param index
                    int fIdx = (cmd == "B3") ? 3 : 2;   // show-text param index
                    try { if ((int)parts.size() > hIdx && !parts[hIdx].empty())
                              pBcHeight = std::stoi(parts[hIdx]); } catch (...) {}
                    if ((int)parts.size() > fIdx && !parts[fIdx].empty())
                        pBcShowText = (parts[fIdx][0] != 'N' && parts[fIdx][0] != 'n');
                }
                int barH = (pBcHeight > 0) ? pBcHeight : pByHeight;
                auto el = std::make_shared<BarcodeElement>();
                el->x        = foX;
                // ^FT y = bottom of bars; ^FO y = top of element.
                el->y        = pFoIsBaseline ? foY - barH : foY;
                el->barHeight = barH;
                el->showText  = pBcShowText;
                // h = bars + approx. human-readable text height (25 dots)
                el->h        = barH + (pBcShowText ? 25 : 0);
                el->w        = 300;  // zint auto-sizes horizontally
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
                    // Strip ZPL Code 128 invocation / start codes (documented in
                    // Table 2 of the ZPL Programming Guide).  These are printer
                    // control directives embedded in the field data string, not
                    // part of the actual barcode payload:
                    //   >:  (104) Start Code B  — most common, e.g. ">:%BARKOD%"
                    //   >;  (105) Start Code C
                    //   >9  (103) Start Code A
                    //   >5  (99)  CODE C subset switch
                    //   >6  (100) CODE B / FNC4
                    //   >7  (101) CODE A / FNC4
                    //   >8  (102) FNC1
                    // We remove every occurrence because they are meaningless for
                    // on-screen display and would appear as literal characters.
                    static const std::vector<std::string> invCodes = {
                        ">:", ">;", ">9", ">5", ">6", ">7", ">8", "><", ">0",
                        ">1", ">2", ">3", ">4", ">=",
                    };
                    for (const auto& code : invCodes)
                    {
                        size_t pos = 0;
                        while ((pos = data.find(code, pos)) != std::string::npos)
                            data.erase(pos, code.size());
                    }
                    lastBarcode->data = data;
                    lastBarcode = nullptr;
                }
                else
                {
                    auto el = std::make_shared<TextElement>();
                    el->x        = foX;
                    el->y        = pFoIsBaseline ? foY - pFontH : foY;
                    el->text     = data;
                    el->fontSize = pFontH;
                    el->fontWidth = pFontW;
                    el->fontPath = pFontPath;
                    el->fontName = GuessFontName(pFontPath);
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
