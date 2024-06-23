#pragma once

#include <complex>
#include <vector>

class ComplexVector
{
public:
    ComplexVector();

    explicit ComplexVector(std::size_t vectorSize);

    std::complex<double> operator[](std::size_t index) const;
    std::complex<double>& operator[](std::size_t index);

    std::size_t Size() const noexcept;

private:
    std::vector<std::complex<double>> numbersVector{};
};