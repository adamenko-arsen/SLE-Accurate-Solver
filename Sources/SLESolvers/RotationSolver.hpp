#pragma once

#include "../SLESolver.hpp"

class RotationSolver : public SLESolver
{
public:
    ~RotationSolver() override = default;

private:
    static bool isCloseToZero(double x);
    static bool isCloseToZeroForSolves(double x);

    static bool isSolveSuitable(const Matrix& A, const Vector& B, const Vector& X, IterationsCounter& itersCounter);

    SolvingResult SolveInternally(Matrix&& A, Vector&& B);

};