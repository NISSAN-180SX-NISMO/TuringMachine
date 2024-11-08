#include <iostream>
#include <fstream>
#include <optional>

//#define TURING_MACHINE
#define POST_SYSTEM_EMULATOR

bool readFromFile(const std::string &filename, std::string &output)
{
    std::ifstream inputFile(filename);
    if (!inputFile) {
        std::cerr << "Error open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        output += line += '\n';
    }

    inputFile.close();
    return true;
}

bool writeToFile(const std::string &filename, const std::string &str)
{
    std::ofstream outputFile(filename);
    if (!outputFile) {
        std::cerr << "Error open file: " << filename << std::endl;
        return false;
    }

    outputFile << str;
    outputFile.close();
    return true;
}

#if defined(TURING_MACHINE)
#include "TuringMachine.h"

int _main()
{
    auto turingMachine = (new TuringMachineBuilder())
        ->setAlphabet(A{'1', '*', ' ', 'x', 'c', '='})
        ->addState(State{ /// State 0:
            {'1', Command('x', Shift::kRight, 1)},
            {'*', Command('*', Shift::kStop, 0)}
        })
        ->addState(State{ /// State 1:
            {'1', Command('1', Shift::kRight, 1)},
            {'*', Command('*', Shift::kRight, 2)}
        })
        ->addState(State{ /// State 2:
            {'1', Command('c', Shift::kRight, 3)},
            {'=', Command('=', Shift::kLeft, 6)}
        })
        ->addState(State{ /// State 3:
            {'1', Command('1', Shift::kRight, 3)},
            {'=', Command('=', Shift::kRight, 4)}
        })
        ->addState(State{ /// State 4:
            {'1', Command('1', Shift::kRight, 4)},
            {' ', Command('1', Shift::kLeft, 5)}
        })
        ->addState(State{ /// State 5:
            {'1', Command('1', Shift::kLeft, 5)},
            {'*', Command('*', Shift::kLeft, 5)},
            {'=', Command('=', Shift::kLeft, 5)},
            {'c', Command('c', Shift::kRight, 2)}
        })
        ->addState(State{ /// State 6:
            {'1', Command('1', Shift::kLeft, 6)},
            {'*', Command('*', Shift::kLeft, 6)},
            {'x', Command('x', Shift::kRight, 0)},
            {'c', Command('1', Shift::kLeft, 6)}
        })
        ->get();

    std::string tape = "111*11= ";
    readFromFile("../inputTape.txt", tape);
    if (!turingMachine.isValidTape(tape))
        return -555;

    turingMachine.fillTape(tape);

    std::cout << "Tape: " << turingMachine.getTape() << std::endl;

    turingMachine.run();

    std::cout << "Tape: " << turingMachine.getTape() << std::endl;

    writeToFile("../outputTape.txt", turingMachine.getTape());

    return 0;
}
#elif defined(POST_SYSTEM_EMULATOR)
#include "PostSystemEmulator.h"

int _main()
{
    std::string sourceData;
    {
        readFromFile("../PostSystemIniABOBA.txt", sourceData);

        PostSystemSourceDataParser sourceDataParser(sourceData);
        auto postSystemEmulatorBuilder = (new PostSystemEmulatorBuilder())
            ->setAlphabet(sourceDataParser.getA())
            ->setAxioms(sourceDataParser.getA1())
            ->setVariables(sourceDataParser.getX());

        for (const auto &rule : sourceDataParser.getR())
            postSystemEmulatorBuilder->addRule(rule);

        auto postSystemEmulator = postSystemEmulatorBuilder->get();
        postSystemEmulator.setLine(sourceDataParser.getLine());

        postSystemEmulator.run();
    }
    std::cout << std::endl << std::endl << std::endl << std::endl;
    sourceData.clear();
    {
        readFromFile("../PostSystemIni.txt", sourceData);

        PostSystemSourceDataParser sourceDataParser(sourceData);
        auto postSystemEmulatorBuilder = (new PostSystemEmulatorBuilder())
            ->setAlphabet(sourceDataParser.getA())
            ->setAxioms(sourceDataParser.getA1())
            ->setVariables(sourceDataParser.getX());

        for (const auto &rule : sourceDataParser.getR())
            postSystemEmulatorBuilder->addRule(rule);

        auto postSystemEmulator = postSystemEmulatorBuilder->get();
        postSystemEmulator.setLine(sourceDataParser.getLine());

        postSystemEmulator.run();
    }

    return 0;
}
#else
int _main()
{ return 0; }
#endif

int main()
{
    return _main();
}
