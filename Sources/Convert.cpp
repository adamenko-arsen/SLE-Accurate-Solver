#include "Convert.hpp"

#include <sstream>
#include <string>
#include <iomanip>

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
    streamForFormatting << std::fixed << std::setprecision(12) << number;

    std::string formattedNumber = streamForFormatting.str();

    formattedNumber.erase(formattedNumber.find_last_not_of('0') + 1, std::string::npos);

    // Remove the decimal point if it is the last character
    if (formattedNumber.back() == '.')
    {
        formattedNumber.pop_back();
    }

    if (formattedNumber.front() == '-')
    {
        formattedNumber.erase(0, 1);
    }

    return formattedNumber;
}
