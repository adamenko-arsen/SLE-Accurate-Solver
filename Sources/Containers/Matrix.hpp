#pragma once

#include <cstdint>

#include <vector>

class Matrix
{
public:
    Matrix();

    explicit Matrix(std::size_t height, std::size_t width);

    double At(std::size_t y, std::size_t x) const;
    double& At(std::size_t y, std::size_t x);

    std::size_t Width() const noexcept;
    std::size_t Height() const noexcept;

    std::size_t TryGetEdgeSize() const noexcept;

    bool IsSquare() const noexcept;

private:
    std::size_t width = 0, height = 0;

    std::vector<double> flattenMatrixVH{};

    double flatMtxElemRef(std::size_t y, std::size_t x) const;
    double& flatMtxElemRef(std::size_t y, std::size_t x);
};