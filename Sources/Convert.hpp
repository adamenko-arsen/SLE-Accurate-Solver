#pragma once

#include <cstdint>

#include <optional>
#include <string>

std::optional<std::ptrdiff_t> ToInteger(const std::string& mayFormattedInt);

std::optional<double> ToNumber(const std::string& str);