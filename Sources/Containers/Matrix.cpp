#include "Matrix.hpp"

Matrix::Matrix() = default;

Matrix::Matrix(std::size_t height, std::size_t width)
{
    this->width = width;
    this->height = height;

    flattenMatrixVH = std::vector<double>(width * height);
}

double Matrix::At(std::size_t y, std::size_t x) const
{
    return flatMtxElemRef(y, x);
}
double& Matrix::At(std::size_t y, std::size_t x)
{
    return flatMtxElemRef(y, x);
}

std::size_t Matrix::Width() const noexcept
{
    return width;
}
std::size_t Matrix::Height() const noexcept
{
    return height;
}

std::size_t Matrix::TryGetEdgeSize() const noexcept
{
    return width;
}

bool Matrix::IsSquare() const noexcept
{
    return width == height;
}

double Matrix::flatMtxElemRef(std::size_t y, std::size_t x) const
{
    return flattenMatrixVH[width * y + x];
}
double& Matrix::flatMtxElemRef(std::size_t y, std::size_t x)
{
    return flattenMatrixVH[width * y + x];
}
