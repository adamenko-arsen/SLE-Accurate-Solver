#pragma once

#include "../SLESolver.hpp"

#include "../Containers/ComplexMatrix.hpp"
#include "../Containers/ComplexVector.hpp"

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

    static bool isSymmetrixMembersCloseEnough(double firstMember, double secondMember);
    static bool isMatrixSymmetrix(const Matrix& maySymmetricMatrix, IterationsCounter& itersCounter);

    static bool isSolveSuitable(const Matrix& A, const Vector& B, const Vector& X, IterationsCounter& IterationsCounter);

    static std::optional<ComplexMatrix> llDecompose(const Matrix& A, IterationsCounter& itersCounter);

    static std::optional<ComplexVector> solveY(const ComplexMatrix& L, const Vector& B, IterationsCounter& itersCounter);
    static std::optional<Vector> solveX(const ComplexMatrix& L, const ComplexVector& Y, IterationsCounter& itersCounter);

protected:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B);
};
