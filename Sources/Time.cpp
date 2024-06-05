#include "Time.hpp"

#include <ctime>

#include <format>

std::ptrdiff_t Time::GetTimeSeconds()
{
    return time(NULL);
}

std::string Time::GetCurrentFormalTime()
{
    auto seconds = GetTimeSeconds();

    return std::format
    (
        "{}:{}:{} UTC+00:00"
        , seconds / (60 * 60) % 24
        , seconds / 60 % 60
        , seconds % 60
    );
}
