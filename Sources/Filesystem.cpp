#include "Filesystem.hpp"

#include <cstdio>

bool Filesystem::WriteToFile(const std::string& outputFileName, const std::string& newContent)
{
    auto outputHandler = fopen(outputFileName.c_str(), "w");

    if (! (outputHandler != NULL))
    {
        return false;
    }

    for (const auto& outputChar : newContent)
    {
        fputc(outputChar, outputHandler);
    }

    fclose(outputHandler);

    return true;
}

std::optional<std::string> Filesystem::ReadFromFile(const std::string& fileName)
{
    auto inputHandler = fopen(fileName.c_str(), "r");

    if (! (inputHandler != NULL))
    {
        return std::nullopt;
    }

    std::string fileContent;

    int inputChar;
    while ((inputChar = fgetc(inputHandler)) != EOF)
    {
        fileContent += inputChar;
    }

    fclose(inputHandler);

    return fileContent;
}
