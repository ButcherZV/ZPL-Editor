#pragma once
#include "Command.h"
#include <vector>
#include <memory>

// Executes/undoes a group of commands atomically (single undo step).
class CompoundCommand : public Command
{
public:
    void Add(std::unique_ptr<Command> cmd) { m_cmds.push_back(std::move(cmd)); }
    bool Empty() const { return m_cmds.empty(); }

    void Execute() override
    {
        for (auto& c : m_cmds)
            c->Execute();
    }
    void Undo() override
    {
        for (auto it = m_cmds.rbegin(); it != m_cmds.rend(); ++it)
            (*it)->Undo();
    }

private:
    std::vector<std::unique_ptr<Command>> m_cmds;
};
