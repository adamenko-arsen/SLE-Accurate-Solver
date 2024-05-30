#pragma once

#include <cstdint>

#include <optional>
#include <string>

struct Convert final
{
    Convert() = delete;
    ~Convert() = delete;

    static std::optional<std::ptrdiff_t> ToInteger(const std::string& mayFormattedInt);

    static std::optional<double> ToNumber(const std::string& mayFormattedNumber);

    static std::string NumberToString(double number);
};
