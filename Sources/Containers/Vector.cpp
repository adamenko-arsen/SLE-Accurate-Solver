#include "Vector.hpp"

Vector::Vector() = default;

Vector::Vector(std::size_t vectorSize)
{
    numbersVector.resize(vectorSize, 0);
}

double Vector::operator[](std::size_t index) const
{
    return numbersVector[index];
}
double& Vector::operator[](std::size_t index)
{
    return numbersVector[index];
}

std::size_t Vector::Size() const noexcept
{
    return numbersVector.size();
}
