#pragma once

#include "Containers/Marix.hpp"
#include "Containers/Vector.hpp"

#include <cstdint>

#include <optional>

class ParsedSLE
{
public:
    ParsedSLE();
    ParsedSLE(std::size_t n, auto&& a, auto&& b)
    {
        eqsCount = n;

        varsCoeffs = std::forward<decltype(a)>(a);
        freeCoeffs = std::forward<decltype(b)>(b);
    }

    std::size_t GetEquationsCount() const noexcept;

    Matrix& GetVariablesCoefficients();
    const Matrix& GetVariablesCoefficients() const;

    Vector& GetFreeCoefficients();
    const Vector& GetFreeCoefficients() const;

private:
    std::size_t eqsCount = 0;

    Matrix varsCoeffs{};
    Vector freeCoeffs{};
}

struct SLEParser final
{
    SLEParser() = delete;
    ~SLEParser() = delete;

    static std::optional<ParsedSLE> Parse(const std::string& formattedSLE);
};
