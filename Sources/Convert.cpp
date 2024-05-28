#include "Convert.hpp"

#include <sstream>
#include <string>

std::optional<std::ptrdiff_t> ToInteger(const std::string& mayFormattedInt)
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

std::optional<double> ToNumber(const std::string& str)
{
    std::istringstream iss(str);
    iss.imbue(std::locale::classic());

    double result;
    iss >> result;

    if (iss.fail() || !iss.eof())
    {
        return std::nullopt;
    }

    return result;
}
