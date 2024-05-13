#pragma once

#include <string>
#include <optional>

bool WriteToFile(const std::string& fileName, const std::string& content);

std::optional<std::string> ReadFromFile(const std::string& inputFileName);
