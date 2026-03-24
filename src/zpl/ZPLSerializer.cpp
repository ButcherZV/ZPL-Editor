#include "ZPLSerializer.h"
#include <sstream>

std::string ZPLSerializer::Serialize(
    const LabelConfig& config,
    const std::vector<std::shared_ptr<LabelElement>>& elements) const
{
    std::ostringstream oss;

    // Label start
    oss << "^XA\n";

    // Label home (left margin, top margin)
    if (config.marginLeft || config.marginTop)
        oss << "^LH" << config.marginLeft << "," << config.marginTop << "\n";

    // Print width (total width in dots)
    oss << "^PW" << config.totalWidth() << "\n";

    // Label length (total height in dots)
    oss << "^LL" << config.totalHeight() << "\n";

    // UTF-8 encoding
    oss << "^CI28\n";

    // Elements
    for (const auto& el : elements)
        oss << el->GetZPL() << "\n";

    // Label end
    oss << "^XZ\n";

    return oss.str();
}
