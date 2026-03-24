#pragma once
#include "Command.h"
#include "../elements/LabelElement.h"
#include <functional>
#include <string>

// Generic property edit command using getter/setter lambdas.
// T must be copyable.
template<typename T>
class EditPropertyCommand : public Command
{
public:
    EditPropertyCommand(std::function<T()>    getter,
                        std::function<void(T)> setter,
                        T oldVal, T newVal)
        : m_getter(getter), m_setter(setter)
        , m_old(oldVal), m_new(newVal) {}

    void Execute() override { m_setter(m_new); }
    void Undo()    override { m_setter(m_old); }

private:
    std::function<T()>    m_getter;
    std::function<void(T)> m_setter;
    T m_old, m_new;
};
