#pragma once

#include <string>
#include <vector>

struct StringUtility final
{
    StringUtility() = delete;
    ~StringUtility() = delete;

    static std::vector<std::string> Split(const std::string& baseString, const std::string& delimiter);
};
