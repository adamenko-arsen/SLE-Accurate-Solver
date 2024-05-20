#pragma once

#include "../SLESolver.hpp"

class RotationSolver : public SLESolver
{
public:
    ~RotationSolver() override = default;

private:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B);

    static bool isCloseToZero(double x);
};