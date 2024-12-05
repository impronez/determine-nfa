#pragma once
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <string>

#include "Transition.h"

using Transitions = std::map<std::string, std::map<std::string, Transition>>; // [state][input][transition]
using TransitionTable = std::map<std::set<std::string>, std::map<std::string, std::set<std::string>>>;// [states][input][nextStates]

constexpr std::string E_CLOSE = "ε";

class MooreAutomata
{
public:
    MooreAutomata(
        std::set<std::string> &inputs,
        std::set<std::string> &states,
        Transitions &transitions,
        std::string &startState,
        std::string &finalState
    )
        : m_inputs(std::move(inputs)),
          m_states(std::move(states)),
          m_transitions(std::move(transitions)),
          m_startState(std::move(startState)),
          m_finalStates({std::move(finalState)})
    {}

    void ExportToFile(const std::string &filename)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open output file " + filename);
        }

        std::string outputs = ";";
        std::string states = ";";

        for (int i = 1; auto &state: m_states)
        {
            states += state;

            if (IsFinalState(state))
            {
                outputs += "F";
            }

            if (i++ != m_states.size())
            {
                outputs += ";";
                states += ";";
            }
        }

        file << outputs << std::endl;
        file << states << std::endl;

        for (auto &input: m_inputs)
        {
            file << input << ";";
            for (int i = 1; auto &state: m_states)
            {
                if (m_transitions.contains(state) &&
                    m_transitions[state].contains(input))
                {
                    file << m_transitions[state].at(input).GetStatesString();

                    if (i != m_states.size())
                    {
                        file << ";";
                    }
                } else if (i != m_states.size())
                {
                    file << ";";
                }
                ++i;
            }
            file << "\n";
        }

        file.close();
    }

    void Determine()
    {
        if (!m_inputs.contains(E_CLOSE))
        {
            return;
        }

        auto transitiveClosures = GetTransitiveClosures();
        auto inputs = GetNonEmptyInputs();

        auto startState = std::set<std::string>();
        auto transitionTable = GetStartTransitionTable(startState, transitiveClosures);

        std::queue<std::set<std::string>> statesQueue;
        statesQueue.push(startState);

        while (!statesQueue.empty())
        {
            std::set<std::string> state = statesQueue.front();
            std::map<std::string, std::set<std::string>> nextStates;
            for (auto &input: inputs)
            {
                for (auto &sourceState: state)
                {
                    for (auto &transitiveState: transitiveClosures.at(sourceState))
                    {
                        if (AreThereStateTransitions(transitiveState, input))
                        {
                            if (!nextStates.contains(input))
                            {
                                nextStates.emplace(input, std::set<std::string>());
                            }

                            for (auto &nextState: m_transitions.at(transitiveState).at(input).GetStates())
                            {
                                nextStates[input].insert(nextState);
                            }
                        }
                    }
                }
            }

            AddStatesFromTransitiveClosures(transitiveClosures, nextStates);

            transitionTable.at(state) = nextStates;

            for (auto &row: nextStates)
            {
                if (!transitionTable.contains(row.second))
                {
                    transitionTable.emplace(row.second, std::map<std::string, std::set<std::string>>());
                    statesQueue.push(row.second);
                }
            }

            statesQueue.pop();
        }

        SetNewStatesAndTransitions(transitionTable, startState, inputs);
    }

private:
    static void AddStatesFromTransitiveClosures(std::map<std::string, std::set<std::string>>& transitiveClosures,
        std::map<std::string, std::set<std::string>>& nextStates)
    {
        for (auto& it: nextStates)
        {
            for (auto& nextState: it.second)
            {
                if (transitiveClosures.at(nextState).size() > 1)
                {
                    for (auto& state: transitiveClosures.at(nextState))
                    {
                        if (!it.second.contains(state))
                        {
                            it.second.insert(state);
                        }
                    }
                }
            }
        }
    }

    void SetNewStatesAndTransitions(TransitionTable& table, std::set<std::string>& startState,
        std::set<std::string>& inputs)
    {
        auto newStateNames = GetNewStateNames(table, startState);
        auto newStates = GetNewStates(newStateNames);
        std::string newStartState = newStateNames[startState];

        Transitions transitions;
        for (auto &it: table)
        {
            std::string newState = newStateNames[it.first];
            transitions.emplace(newState, std::map<std::string, Transition>());

            for (auto &transition: it.second)
            {
                std::string nextState = newStateNames[transition.second];

                transitions.at(newState).emplace(transition.first, Transition(transition.first, nextState));
            }
        }

        std::set<std::string> finalStates = GetFinalStates(newStateNames);

        m_states = newStates;
        m_transitions = transitions;
        m_finalStates = finalStates;
        m_startState = newStartState;
        m_inputs = inputs;
    }

    std::set<std::string> GetFinalStates(std::map<std::set<std::string>, std::string>& newStateNames)
    {
        std::set<std::string> finalStates;
        for (auto &it: newStateNames)
        {
            for (auto &state: it.first)
            {
                if (IsFinalState(state))
                {
                    finalStates.insert(it.second);
                    break;
                }
            }
        }

        return finalStates;
    }

    [[nodiscard]] bool IsFinalState(const std::string& state) const
    {
        return m_finalStates.contains(state);
    }

    static std::set<std::string> GetNewStates(std::map<std::set<std::string>, std::string>& stateNames)
    {
        std::set<std::string> newStates;
        for (auto &it: stateNames)
        {
            newStates.emplace(it.second);
        }

        return newStates;
    }

    static std::map<std::set<std::string>, std::string> GetNewStateNames(
        const TransitionTable& table, const std::set<std::string>& startState)
    {
        std::map<std::set<std::string>, std::string> newStates;
        unsigned index = 1;

        for (auto &it: table)
        {
            std::string newState;
            if (it.first == startState)
            {
                newState = NEW_STATE_CHAR + std::to_string(0);
            } else
            {
                newState = NEW_STATE_CHAR + std::to_string(index++);
            }

            newStates[it.first] = newState;
        }

        return newStates;
    }

    [[nodiscard]] bool AreThereStateTransitions(const std::string& state, const std::string& input) const
    {
        return m_transitions.contains(state) && m_transitions.at(state).contains(input);
    }

    [[nodiscard]] TransitionTable GetStartTransitionTable(std::set<std::string> &startStates,
          const std::map<std::string, std::set<std::string>>& transitiveClosures) const
    {
        TransitionTable table;

        startStates.insert(m_startState);

        for (auto& state: startStates)
        {
            for (auto &transitiveState: transitiveClosures.at(state))
            {
                startStates.insert(transitiveState);
            }
        }

        table.emplace(startStates, std::map<std::string, std::set<std::string> >());

        return table;
    }

    static constexpr char NEW_STATE_CHAR = 'S';

    [[nodiscard]] std::set<std::string> GetNonEmptyInputs() const
    {
        std::set<std::string> inputs = m_inputs;
        inputs.erase(E_CLOSE);

        return inputs;
    }

    [[nodiscard]] std::map<std::string, std::set<std::string>> GetTransitiveClosures() const
    {
        std::map<std::string, std::set<std::string>> transitiveClosures;

        for (auto &state: m_states)
        {
            transitiveClosures.emplace(state, std::set<std::string>());
            transitiveClosures.at(state).emplace(state);

            if (m_transitions.contains(state) &&
                m_transitions.at(state).contains(E_CLOSE))
            {
                for (auto &nextState: m_transitions.at(state).at(E_CLOSE).GetStates())
                {
                    transitiveClosures[state].emplace(nextState);
                }
            }
        }

        return transitiveClosures;
    }

    std::set<std::string> m_inputs;
    std::set<std::string> m_states;

    Transitions m_transitions;

    std::string m_startState;
    std::set<std::string> m_finalStates;
};

using Automata = MooreAutomata;
