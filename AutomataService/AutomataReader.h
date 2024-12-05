#pragma once
#include <iostream>
#include <sstream>
#include <vector>

#include "../Automata/Automata.h"

constexpr char FINAL_STATE_INDEX = 'F';

class AutomataReader
{
public:
    static Automata GetAutomataFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::invalid_argument("Could not open input file " + filename);
        }

        std::string line;
        std::getline(file, line);
        auto finalStateIndex = GetFinalStateIndex(line);

        std::getline(file, line);
        auto states = GetStates(line);

        std::string startState = states.front();
        std::string finalState = states[finalStateIndex];

        std::set<std::string> inputs;
        Transitions transitions;

        SetTransitionsTableData(transitions, file, states, inputs);

        auto statesSet = GetSetFromStringVector(states);

        return { inputs, statesSet, transitions, startState, finalState };
    }

private:
    static void SplitTransitionsLine(const std::string& line, Transitions& transitions,
        const std::string& state, const std::string& input)
    {
        std::stringstream ss(line);
        std::string nextState;

        Transition transition(input);

        while (std::getline(ss, nextState, ','))
        {
            transition.AddState(nextState);
        }

        if (!transitions.contains(state))
        {
            transitions.emplace(state, std::map<std::string, Transition>());
        }

        if (!transitions[state].contains(input))
        {
            transitions[state].emplace(input, transition);
        }
        else
        {
            transitions[state].at(input) = transition;
        }
    }

    static void SetTransitionsTableData(Transitions& transitions, std::ifstream& file,
        std::vector<std::string>& states, std::set<std::string>& inputs)
    {
        std::string line;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string inputSymbol;

            size_t stateIndex = 0;

            if (std::getline(ss, inputSymbol, ';'))
            {
                std::string transition;
                while (std::getline(ss, transition, ';'))
                {
                    if (stateIndex >= states.size())
                    {
                        throw std::invalid_argument("State index out of range");
                    }

                    if (!transition.empty())
                    {
                        SplitTransitionsLine(transition, transitions, states[stateIndex], inputSymbol);
                    }

                    ++stateIndex;
                }

                inputs.emplace(inputSymbol);
            }
        }
    }

    static std::set<std::string> GetSetFromStringVector(std::vector<std::string>& vec)
    {
        return {vec.begin(), vec.end()};
    }

    static std::vector<std::string> GetStates(std::string& line)
    {
        std::vector<std::string> states;

        std::stringstream ss(line);
        std::string state;

        while (std::getline(ss, state, ';'))
        {
            if (!state.empty())
            {
                states.emplace_back(state);
            }
        }

        return states;
    }

    static size_t GetFinalStateIndex(const std::string& line)
    {
        size_t index = line.find(FINAL_STATE_INDEX);
        if (index != std::string::npos)
        {
            return index - 1;
        }

        throw std::invalid_argument("Could not find 'F' (final state) in input file " + line);
    }
};
