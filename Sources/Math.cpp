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

    auto atomicUnit = 1.0 / std::pow(10, maxDigits);

    return (isPositive ? 1 : -1) * (std::floor(std::fabs(number) / atomicUnit) * atomicUnit);
}

double Math::IsNumberInRange(double value, double from, double to)
{
    return from <= value && value <= to;
}
