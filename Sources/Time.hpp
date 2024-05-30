#pragma once

#include <string>

struct Time final
{
    Time() = delete;
    ~Time() = delete;

    static std::ptrdiff_t GetTimeSeconds();
    static std::string GetCurrentFormalTime();
};
