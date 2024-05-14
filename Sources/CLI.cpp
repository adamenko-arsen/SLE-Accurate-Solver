#include "CLI.hpp"

#include "Convert.hpp"
#include "Filesystem.hpp"
#include "LSESolversData.hpp"

#include <string>
#include <iostream>
#include <optional>
#include <format>

static std::optional<std::string> ParseFormattedLSE(const std::string& formattedLSE)
{
    return std::nullopt;
}

template<typename T>
static void Print(const T& stringable)
{
    std::cout << stringable << std::endl;
}

template<typename T>
static void PrintError(const T& stringable)
{
    std::cout << "Error: " << stringable << std::endl;
}

static std::string Input(const std::string& prompt = "")
{
    std::string inputBuffer{};

    std::cout << prompt;

    std::getline(std::cin, inputBuffer);

    return inputBuffer;
}

int CLISession::Run(int argc, char *argv[])
{
    Print("Welcome to the LSE Solver's strict CLI mode!");
    Print("");
    Print("The method's properties:");
    Print("");

    for (std::size_t i = 0; i < comboBoxMethodRecords.size(); i++)
    {
        auto& currentOption = comboBoxMethodRecords[i];

        Print(std::format(
              "#{}: {}\n  Складність: {}\n"
            , i + 1
            , currentOption.MethodName
            , currentOption.MethodPracticalItersComplexity
        ));
    }

    Print("Type your chose method's index, the input file's name once below without any typo:");
    Print("");

    auto mayFormattedMethodIndex = Input("The method's index: ");
    Print("");

    auto mayMethodIndex = ToInteger(mayFormattedMethodIndex);

    if (! mayMethodIndex.has_value())
    {
        PrintError("the input is not a number");

        return 0;
    }
    auto internalMethodIndex = mayMethodIndex.value();

    if (! (0 <= mayMethodIndex && mayMethodIndex < comboBoxMethodRecords.size()))
    {
        PrintError("no such the method's index");

        return 0;
    }

    Print("");
    auto lseInputFileName = Input("The input file's name: ");
    Print("");

    return 0;
}