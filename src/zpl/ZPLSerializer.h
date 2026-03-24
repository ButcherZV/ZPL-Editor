#pragma once
#include "../zpl/ZPLTypes.h"
#include "../elements/LabelElement.h"
#include <vector>
#include <memory>
#include <string>

class ZPLSerializer
{
public:
    std::string Serialize(
        const LabelConfig& config,
        const std::vector<std::shared_ptr<LabelElement>>& elements) const;
};
