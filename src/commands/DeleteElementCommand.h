#pragma once
#include "Command.h"
#include "../elements/LabelElement.h"
#include <vector>
#include <memory>

class DeleteElementCommand : public Command
{
public:
    DeleteElementCommand(std::vector<std::shared_ptr<LabelElement>>& elements,
                         std::shared_ptr<LabelElement> element)
        : m_elements(elements), m_element(std::move(element)) {}

    void Execute() override
    {
        auto it = std::find(m_elements.begin(), m_elements.end(), m_element);
        if (it != m_elements.end())
        {
            m_index = static_cast<int>(std::distance(m_elements.begin(), it));
            m_elements.erase(it);
        }
    }
    void Undo() override
    {
        int idx = std::min(m_index, static_cast<int>(m_elements.size()));
        m_elements.insert(m_elements.begin() + idx, m_element);
    }

private:
    std::vector<std::shared_ptr<LabelElement>>& m_elements;
    std::shared_ptr<LabelElement>               m_element;
    int                                         m_index = 0;
};
