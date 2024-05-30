#include "Time.hpp"

#include <ctime>

#include <format>

std::ptrdiff_t Time::GetTimeSeconds()
{
    return time(NULL);
}

std::string Time::GetCurrentFormalTime()
{
    auto secs = GetTimeSeconds();

    return std::format
    (
        "{}:{}:{} UTC+00:00"
        , secs / (60 * 60) % 24
        , secs / 60 % 60
        , secs % 60
    );
}
