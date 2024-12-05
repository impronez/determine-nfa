#pragma once
#include <stdexcept>
#include <string>

struct Args
{
    std::string inputFilename;
    std::string outputFilename;
};

inline Args ParseArgs(int argc, char** argv)
{
    if (argc != 3)
    {
        const std::string err = "Usage: <program> <inputFilename> <outputFilename>";
        throw std::invalid_argument(err);
    }

    Args args;
    args.inputFilename = argv[1];
    args.outputFilename = argv[2];

    return args;
}