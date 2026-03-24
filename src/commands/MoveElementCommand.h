#pragma once
#include "Command.h"
#include "../elements/LabelElement.h"
#include <wx/gdicmn.h>

class MoveElementCommand : public Command
{
public:
    MoveElementCommand(LabelElement* el,
                       wxPoint oldPos, wxPoint newPos)
        : m_el(el), m_old(oldPos), m_new(newPos) {}

    void Execute() override { m_el->x = m_new.x; m_el->y = m_new.y; }
    void Undo()    override { m_el->x = m_old.x; m_el->y = m_old.y; }

private:
    LabelElement* m_el;
    wxPoint       m_old, m_new;
};
