#pragma once
#include "Command.h"
#include "../elements/LabelElement.h"
#include <vector>
#include <memory>

class AddElementCommand : public Command
{
public:
    AddElementCommand(std::vector<std::shared_ptr<LabelElement>>& elements,
                      std::shared_ptr<LabelElement> element)
        : m_elements(elements), m_element(std::move(element)) {}

    void Execute() override { m_elements.push_back(m_element); }
    void Undo()    override
    {
        auto it = std::find(m_elements.begin(), m_elements.end(), m_element);
        if (it != m_elements.end()) m_elements.erase(it);
    }

private:
    std::vector<std::shared_ptr<LabelElement>>& m_elements;
    std::shared_ptr<LabelElement>               m_element;
};
