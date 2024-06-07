#pragma once

#include <string>
#include <optional>

struct Filesystem final
{
    Filesystem() = delete;
    ~Filesystem() = delete;

    static bool WriteToFile(const std::string& fileName, const std::string& content);

    static std::optional<std::string> ReadFromFile(const std::string& fileName);
};
