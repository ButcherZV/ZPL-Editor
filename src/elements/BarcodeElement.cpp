#include "BarcodeElement.h"
#include <sstream>
#include <cmath>

// ZINT forward declarations — included only when rendering
#include <zint.h>

void BarcodeElement::Draw(wxDC& dc, wxPoint off, double zoom) const
{
    int px = off.x + static_cast<int>(x * zoom);
    int py = off.y + static_cast<int>(y * zoom);
    int pw = std::max(10, static_cast<int>(w * zoom));
    int ph = std::max(10, static_cast<int>(h * zoom));

    // Map our type to ZINT symbology
    int symbology = BARCODE_CODE128;
    switch (barcodeType)
    {
        case BarcodeType::Code128: symbology = BARCODE_CODE128;    break;
        case BarcodeType::Code39:  symbology = BARCODE_CODE39;     break;
        case BarcodeType::EAN13:   symbology = BARCODE_EANX;       break;
        case BarcodeType::UPCA:    symbology = BARCODE_UPCA;       break;
        case BarcodeType::I2of5:   symbology = BARCODE_C25INTER;   break;
        case BarcodeType::QRCode:  symbology = BARCODE_QRCODE;     break;
    }

    zint_symbol* sym = ZBarcode_Create();
    sym->symbology   = symbology;
    sym->height      = static_cast<float>(ph - 20);
    sym->show_hrt    = showText ? 1 : 0;

    if (ZBarcode_Encode(sym,
            reinterpret_cast<const unsigned char*>(data.c_str()),
            static_cast<int>(data.size())) != 0)
    {
        // Encode failed — draw placeholder
        dc.SetBrush(wxBrush(wxColour(255, 200, 200)));
        dc.SetPen(wxPen(wxColour(200, 0, 0), 1));
        dc.DrawRectangle(px, py, pw, ph);
        dc.DrawText("Bad data", px + 4, py + 4);
        ZBarcode_Delete(sym);
        return;
    }

    // Render to bitmap via ZINT buffer
    if (ZBarcode_Buffer(sym, 0) == 0 && sym->bitmap)
    {
        int bw = sym->bitmap_width;
        int bh = sym->bitmap_height;
        wxImage img(bw, bh, false);

        unsigned char* src = sym->bitmap;
        unsigned char* dst = img.GetData();
        // ZINT returns 24-bit RGB
        memcpy(dst, src, static_cast<size_t>(bw) * bh * 3);

        wxBitmap bmp(img.Scale(pw, ph, wxIMAGE_QUALITY_NORMAL));
        dc.DrawBitmap(bmp, px, py, false);
    }
    else
    {
        // Fallback: plain rectangle
        dc.SetBrush(*wxWHITE_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawRectangle(px, py, pw, ph);
        dc.DrawText(wxString::FromUTF8(data), px + 4, py + 4);
    }

    ZBarcode_Delete(sym);
}

std::string BarcodeElement::GetZPL() const
{
    std::ostringstream oss;
    oss << "^FO" << x << "," << y;

    switch (barcodeType)
    {
    case BarcodeType::Code128:
        oss << "^BCN," << barHeight << "," << (showText ? "Y" : "N")
            << ",N," << (checkDigit ? "Y" : "N");
        break;
    case BarcodeType::Code39:
        oss << "^B3N,N," << barHeight << "," << (showText ? "Y" : "N") << ",N";
        break;
    case BarcodeType::EAN13:
        oss << "^BEN," << barHeight << "," << (showText ? "Y" : "N");
        break;
    case BarcodeType::UPCA:
        oss << "^BUN," << barHeight << "," << (showText ? "Y" : "N");
        break;
    case BarcodeType::I2of5:
        oss << "^BIN," << barHeight << "," << (showText ? "Y" : "N");
        break;
    case BarcodeType::QRCode:
        oss << "^BQN,2,10";
        break;
    }

    oss << "^FD" << data << "^FS";
    return oss.str();
}
