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
    std::istringstream streamForFormatting(mayFormattedNumber);
    streamForFormatting.imbue(std::locale::classic());

    double result;
    streamForFormatting >> result;

    if (streamForFormatting.fail() || !streamForFormatting.eof())
    {
        return std::nullopt;
    }

    return result;
}

std::string Convert::NumberToString(double number)
{
    std::ostringstream streamForFormatting;

    streamForFormatting.imbue(std::locale::classic());
    streamForFormatting << number;

    return streamForFormatting.str();
}
