#pragma once
#include "LabelElement.h"
#include <string>

enum class BarcodeType
{
    Code128,   // ^BC
    Code39,    // ^B3
    EAN13,     // ^BE
    UPCA,      // ^BU
    I2of5,     // ^BI
    QRCode,    // ^BQ
};

class BarcodeElement : public LabelElement
{
public:
    BarcodeType barcodeType   = BarcodeType::Code128;
    std::string data          = "123456789";
    int         barHeight     = 100;   // dots
    bool        showText      = true;
    bool        checkDigit    = false;

    BarcodeElement() { w = 300; h = 120; }

    void Draw(wxDC& dc, wxPoint off, double zoom) const override;
    std::string GetZPL() const override;
    std::unique_ptr<LabelElement> Clone() const override
    {
        return std::make_unique<BarcodeElement>(*this);
    }
};
