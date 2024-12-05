cmake_minimum_required(VERSION 3.29)
project(determine_nfa)

set(CMAKE_CXX_STANDARD 20)
if(WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

add_executable(determine_nfa main.cpp
        Automata/Automata.h
        Automata/Transition.h
        AutomataService/AutomataReader.h
        ArgumentsParser.h
)
