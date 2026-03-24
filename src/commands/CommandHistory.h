#pragma once
#include "Command.h"
#include <vector>
#include <memory>
#include <stack>

class CommandHistory
{
public:
    // Execute a command and push it onto the undo stack.
    // Clears the redo stack.
    void Execute(std::unique_ptr<Command> cmd)
    {
        cmd->Execute();
        m_undo.push(std::move(cmd));
        while (!m_redo.empty()) m_redo.pop();
    }

    void Undo()
    {
        if (m_undo.empty()) return;
        m_undo.top()->Undo();
        m_redo.push(std::move(m_undo.top()));
        m_undo.pop();
    }

    void Redo()
    {
        if (m_redo.empty()) return;
        m_redo.top()->Execute();
        m_undo.push(std::move(m_redo.top()));
        m_redo.pop();
    }

    bool CanUndo() const { return !m_undo.empty(); }
    bool CanRedo() const { return !m_redo.empty(); }

private:
    std::stack<std::unique_ptr<Command>> m_undo;
    std::stack<std::unique_ptr<Command>> m_redo;
};
