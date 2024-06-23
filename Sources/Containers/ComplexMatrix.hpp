#pragma once

#include <cstdint>

#include <complex>
#include <vector>

class ComplexMatrix
{
public:
    ComplexMatrix();

    explicit ComplexMatrix(std::size_t height, std::size_t width);

    std::complex<double> At(std::size_t y, std::size_t x) const;
    std::complex<double>& At(std::size_t y, std::size_t x);

    std::size_t Width() const noexcept;
    std::size_t Height() const noexcept;

    std::size_t TryGetEdgeSize() const noexcept;

    bool IsSquare() const noexcept;

private:
    std::size_t width = 0, height = 0;

    std::vector<std::complex<double>> flattenMatrixVH{};

    std::complex<double> flatMtxElemRef(std::size_t y, std::size_t x) const;
    std::complex<double>& flatMtxElemRef(std::size_t y, std::size_t x);
};