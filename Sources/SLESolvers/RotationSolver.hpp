#pragma once

#include "../SLESolver.hpp"

class RotationSolver : public SLESolver
{
public:
    ~RotationSolver() override = default;

private:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B);

    static std::size_t suitableDiagLine(const Matrix& A, std::size_t firstLine, IterationsCounter& itersCounter);

    static bool isCloseToZero(double x);
};