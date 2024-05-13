#pragma once

#include <vector>

class Vector
{
public:
    Vector();

    explicit Vector(std::size_t vectorSize);

    double operator[](std::size_t index) const;
    double& operator[](std::size_t index);

    std::size_t Size() const noexcept;

private:
    std::vector<double> numbersVector{};
};