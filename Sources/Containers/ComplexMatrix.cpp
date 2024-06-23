#include "ComplexMatrix.hpp"

ComplexMatrix::ComplexMatrix() = default;

ComplexMatrix::ComplexMatrix(std::size_t height, std::size_t width)
{
    this->width = width;
    this->height = height;

    flattenMatrixVH = std::vector<std::complex<double>>(width * height);
}

std::complex<double> ComplexMatrix::At(std::size_t y, std::size_t x) const
{
    return flatMtxElemRef(y, x);
}
std::complex<double>& ComplexMatrix::At(std::size_t y, std::size_t x)
{
    return flatMtxElemRef(y, x);
}

std::size_t ComplexMatrix::Width() const noexcept
{
    return width;
}
std::size_t ComplexMatrix::Height() const noexcept
{
    return height;
}

std::size_t ComplexMatrix::TryGetEdgeSize() const noexcept
{
    return width;
}

bool ComplexMatrix::IsSquare() const noexcept
{
    return width == height;
}

std::complex<double> ComplexMatrix::flatMtxElemRef(std::size_t y, std::size_t x) const
{
    return flattenMatrixVH[width * y + x];
}
std::complex<double>& ComplexMatrix::flatMtxElemRef(std::size_t y, std::size_t x)
{
    return flattenMatrixVH[width * y + x];
}
