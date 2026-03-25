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

    // Print width: full media width covering all label columns.
    // ZPL has no dedicated "labels per row" command — the way to print N labels
    // side-by-side is to set ^PW = N × singleWidth and repeat all field commands
    // at x + col × singleWidth for each column.
    int lpr      = std::max(1, config.labelsPerRow);
    int singleW  = config.totalWidth();
    oss << "^PW" << (singleW * lpr) << "\n";

    // Label length (total height in dots)
    oss << "^LL" << config.totalHeight() << "\n";

    // UTF-8 encoding
    oss << "^CI28\n";

    // Elements — emitted once per column with an x-offset.
    for (int col = 0; col < lpr; ++col)
    {
        int xOff = col * singleW;
        for (const auto& el : elements)
        {
            if (xOff == 0)
            {
                // First column: emit as-is.
                oss << el->GetZPL() << "\n";
            }
            else
            {
                // Mirror columns: temporarily clone and offset.
                auto copy = el->Clone();
                copy->x += xOff;
                oss << copy->GetZPL() << "\n";
            }
        }
    }

    // Label end
    oss << "^XZ\n";

    return oss.str();
}
