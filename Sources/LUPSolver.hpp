#pragma once

#include <cstdint>

#include "LSESolver.hpp"

struct LUPDecResult
{
    Matrix L, U;
    std::vector<std::size_t> P;
};

class LUPSolver : public LSESolver
{
public:
    ~LUPSolver() override = default;

private:
    static bool isCloseToZero(double x);

    static std::size_t maxDiagLine(const Matrix& A, std::size_t baseColumn);

    static std::optional<LUPDecResult> lupDecompose(Matrix A, IterationsCounter& itersCounter);

    static std::optional<Vector> solveY(
          const Matrix& L
        , const std::vector<std::size_t>& P
        , const Vector& B
        , IterationsCounter& itersCounter
    );

    static Vector solveX(const Matrix& U, const Vector& Y, IterationsCounter& itersCounter);

protected:
    SolvingResult SolveLSEInternal(Matrix&& A, Vector&& B) override;
};
