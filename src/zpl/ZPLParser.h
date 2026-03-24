#pragma once
#include "ZPLTypes.h"
#include "../elements/LabelElement.h"
#include <vector>
#include <memory>
#include <string>

struct ParseResult
{
    bool   ok       = false;
    wxString errorMsg;
    LabelConfig config;
    std::vector<std::shared_ptr<LabelElement>> elements;
};

class ZPLParser
{
public:
    ParseResult Parse(const std::string& zpl,
                      PrinterDPI dpiHint = PrinterDPI::DPI_203);
};
