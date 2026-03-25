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
        ++m_version;
    }

    void Undo()
    {
        if (m_undo.empty()) return;
        m_undo.top()->Undo();
        m_redo.push(std::move(m_undo.top()));
        m_undo.pop();
        --m_version;
    }

    void Redo()
    {
        if (m_redo.empty()) return;
        m_redo.top()->Execute();
        m_undo.push(std::move(m_redo.top()));
        m_redo.pop();
        ++m_version;
    }

    bool CanUndo() const { return !m_undo.empty(); }
    bool CanRedo() const { return !m_redo.empty(); }

    // Mark the current state as clean (just saved / just loaded).
    void MarkClean() { m_savedVersion = m_version; }

    // Returns true if the content has changed since the last MarkClean().
    bool IsModified() const { return m_version != m_savedVersion; }

    // Clear all history and reset to a clean state.
    void Clear()
    {
        while (!m_undo.empty()) m_undo.pop();
        while (!m_redo.empty()) m_redo.pop();
        m_version = 0;
        m_savedVersion = 0;
    }

private:
    std::stack<std::unique_ptr<Command>> m_undo;
    std::stack<std::unique_ptr<Command>> m_redo;
    int m_version      = 0;
    int m_savedVersion = 0;
};
