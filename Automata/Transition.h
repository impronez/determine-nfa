#pragma once
#include <set>
#include <string>

class Transition
{
public:
    explicit Transition(const std::string& input)
        : m_input(input)
    {
        CheckInputSymbol(input);
    }

    Transition(const std::string& input, const std::string& state)
        : m_input(input)
    {
        CheckInputSymbol(input);

        m_nextStates.insert(state);
    }

    void AddState(const std::string& state)
    {
        m_nextStates.insert(state);
    }

    [[nodiscard]] std::string GetStatesString() const
    {
        std::string states;

        for (int i = 1; auto& state: m_nextStates)
        {
            states += state;

            if (i++ != m_nextStates.size())
            {
                states += ",";
            }
        }

        return states;
    }

    [[nodiscard]] std::set<std::string> GetStates() const
    {
        return m_nextStates;
    }

private:
    static void CheckInputSymbol(const std::string& input)
    {
        if (input.empty())
        {
            throw std::invalid_argument("Empty input symbol in transition");
        }
    }

    std::string m_input;
    std::set<std::string> m_nextStates {};
};
