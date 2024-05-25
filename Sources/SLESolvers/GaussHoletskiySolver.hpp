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

    static bool isSolveSuitable(const Matrix& A, const Vector& B, const Vector& X);
    static bool isSolveNotAmbigious(const Vector& B, const Vector& X);

    static std::optional<LDLDecResult> ldlDecompose(const Matrix& A, IterationsCounter& itersCounter);

    static Vector solveY(const Matrix& L, const Vector& B, IterationsCounter& itersCounter);
    static std::optional<Vector> solveZ(const Matrix& D, const Vector& Y, IterationsCounter& itersCounter);
    static Vector solveX(const Matrix& L, const Vector& Z, IterationsCounter& itersCounter);

protected:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B);
};
