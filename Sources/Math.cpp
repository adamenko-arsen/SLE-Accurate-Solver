#include "Math.hpp"

#include <cmath>

double Math::PutNumberToBounds(double value, double from, double to)
{
    if (! (from <= value))
    {
        return from;
    }
    if (! (value <= to))
    {
        return to;
    }
    return value;
}

double Math::Logarithm(double poweredValue, double base)
{
    return std::log(poweredValue) / std::log(base);
}

double Math::FloorWithPrecision(double number, std::size_t maxDigits)
{
    bool isPositive = number >= 0;

    auto multiplier = std::pow(10, maxDigits);

    return (isPositive ? 1 : -1) * (std::floor(std::fabs(number) * multiplier) / multiplier);
}
