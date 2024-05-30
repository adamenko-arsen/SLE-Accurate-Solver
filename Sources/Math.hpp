#pragma once

#include <cstdint>

struct Math final
{
    Math() = delete;
    ~Math() = delete;

    static double PutNumberToBounds(double value, double from, double to);

    static double FloorWithPrecision(double number, std::size_t maxDigits);

    static double Logarithm(double poweredValue, double base);
};
