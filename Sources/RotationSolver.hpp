#pragma once

#include "LSESolver.hpp"

class RotationSolver : public LSESolver
{
public:
    ~RotationSolver() override = default;

private:
    SolvingResult SolveLSEInternal(Matrix&& A, Vector&& B);

    static bool isCloseToZero(double x);
};