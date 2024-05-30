#include "StringUtility.hpp"

std::vector<std::string> StringUtility::Split(const std::string& baseString, const std::string& delimiter)
{
    std::vector<std::string> splittedStrings{};

    std::size_t start = 0;
    auto end = baseString.find(delimiter);

    while (end != std::string::npos)
    {
        splittedStrings.push_back(baseString.substr(start, end - start));

        start = end + delimiter.length();

        end = baseString.find(delimiter, start);
    }

    splittedStrings.push_back(baseString.substr(start, end));

    return splittedStrings;
}
