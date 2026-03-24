#pragma once
#include "Command.h"
#include "../elements/LabelElement.h"
#include <wx/gdicmn.h>

class ResizeElementCommand : public Command
{
public:
    ResizeElementCommand(LabelElement* el,
                         wxPoint oldPos, wxSize oldSize,
                         wxPoint newPos, wxSize newSize)
        : m_el(el)
        , m_oldPos(oldPos), m_oldSize(oldSize)
        , m_newPos(newPos), m_newSize(newSize) {}

    void Execute() override
    {
        m_el->x = m_newPos.x; m_el->y = m_newPos.y;
        m_el->w = m_newSize.x; m_el->h = m_newSize.y;
    }
    void Undo() override
    {
        m_el->x = m_oldPos.x; m_el->y = m_oldPos.y;
        m_el->w = m_oldSize.x; m_el->h = m_oldSize.y;
    }

private:
    LabelElement* m_el;
    wxPoint m_oldPos, m_newPos;
    wxSize  m_oldSize, m_newSize;
};
