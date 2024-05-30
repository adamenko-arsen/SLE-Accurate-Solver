#include "Convert.hpp"

#include <sstream>
#include <string>

std::optional<std::ptrdiff_t> Convert::ToInteger(const std::string& mayFormattedInt)
{
    try
    {
        return std::stoi(mayFormattedInt);
    }
    catch (const std::exception& _)
    {
        return std::nullopt;
    }
}

std::optional<double> Convert::ToNumber(const std::string& mayFormattedNumber)
{
    std::istringstream iss(mayFormattedNumber);
    iss.imbue(std::locale::classic());

    double result;
    iss >> result;

    if (iss.fail() || !iss.eof())
    {
        return std::nullopt;
    }

    return result;
}

std::string Convert::NumberToString(double number)
{
    std::ostringstream oss;

    oss.imbue(std::locale::classic());
    oss << number;

    return oss.str();
}
