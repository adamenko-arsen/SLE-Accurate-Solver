#include "Filesystem.hpp"

#include <cstdio>

bool WriteToFile(const std::string& outputFileName, const std::string& newContent)
{
    auto outputHandler = fopen(outputFileName.c_str(), "w");

    if (! (outputHandler != NULL))
    {
        return false;
    }

    for (const auto& outChr : newContent)
    {
        fputc(outChr, outputHandler);
    }

    fclose(outputHandler);

    return true;
}

std::optional<std::string> ReadFromFile(const std::string& inputFileName)
{
    auto inputHandler = fopen(inputFileName.c_str(), "r");

    if (! (inputHandler != NULL))
    {
        return std::nullopt;
    }

    std::string readContent;

    int inChr;
    while ((inChr = fgetc(inputHandler)) != EOF)
    {
        readContent += inChr;
    }

    fclose(inputHandler);

    return readContent;
}
