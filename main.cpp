#include "ArgumentsParser.h"
#include "AutomataService/AutomataReader.h"

int main(int argc, char *argv[])
{
    try
    {
        Args args = ParseArgs(argc, argv);
        auto automata = AutomataReader::GetAutomataFromFile(args.inputFilename);

        automata.Determine();

        automata.ExportToFile(args.outputFilename);

        std::cout << "Executed!\n";
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;

        return -1;
    }

    return 0;
}
