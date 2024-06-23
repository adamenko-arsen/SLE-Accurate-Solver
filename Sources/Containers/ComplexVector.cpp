#include "ComplexVector.hpp"

ComplexVector::ComplexVector() = default;

ComplexVector::ComplexVector(std::size_t vectorSize)
{
    numbersVector.resize(vectorSize, 0);
}

std::complex<double> ComplexVector::operator[](std::size_t index) const
{
    return numbersVector[index];
}
std::complex<double>& ComplexVector::operator[](std::size_t index)
{
    return numbersVector[index];
}

std::size_t ComplexVector::Size() const noexcept
{
    return numbersVector.size();
}
