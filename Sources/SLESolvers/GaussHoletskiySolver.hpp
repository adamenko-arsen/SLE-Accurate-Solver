#pragma once

#include "../SLESolver.hpp"

struct LDLDecResult
{
    Matrix L, D;
};

class GaussHoletskiySolver : public SLESolver
{
public:
    ~GaussHoletskiySolver() override = default;

private:
    static bool isCloseToZero(double x);
    static bool isCloseToZeroForSolves(double x);
    static bool isCloseToZeroForAmbigiousCheck(double x);

    static bool isSolveSuitable(const Matrix& A, const Vector& B, const Vector& X, IterationsCounter& IterationsCounter);

    static std::optional<Matrix> llDecompose(const Matrix& A, IterationsCounter& itersCounter);

    static std::optional<Vector> solveY(const Matrix& L, const Vector& B, IterationsCounter& itersCounter);
    static std::optional<Vector> solveX(const Matrix& L, const Vector& Y, IterationsCounter& itersCounter);

protected:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B);
};
