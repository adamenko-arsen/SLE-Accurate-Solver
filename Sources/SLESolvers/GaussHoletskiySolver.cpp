#include "GaussHoletskiySolver.hpp"

#include <cmath>

bool GaussHoletskiySolver::isCloseToZero(double x)
{
    return std::fabs(x) < 1e-9;
}

bool GaussHoletskiySolver::isCloseToZeroForSolves(double x)
{
    return std::fabs(x) < 1e-9;
}

bool GaussHoletskiySolver::isCloseToZeroForAmbigiousCheck(double x)
{
    return std::fabs(x) < 1e-9;
}

bool GaussHoletskiySolver::isSolveSuitable(const Matrix& A, const Vector& B, const Vector& X)
{
    auto n = B.Size();

    Vector NewB(n);

    for (std::size_t y = 0; y < n; y++)
    {
        NewB[y] = 0;

        for (std::size_t x = 0; x < n; x++)
        {
            NewB[y] += A.At(y, x) * X[x];
        }
    }

    for (std::size_t y = 0; y < n; y++)
    {
        if (! isCloseToZeroForSolves(B[y] - NewB[y]))
        {
            return false;
        }
    }

    return true;
}

#include <iostream>

bool GaussHoletskiySolver::isSolveNotAmbigious(const Vector& B, const Vector& X)
{
    if (! (B.Size() >= 2))
    {
        return true;
    }

    for (std::size_t i = 0; i < B.Size(); i++)
    {
        std::cout << B[i] << std::endl;

        if (! isCloseToZeroForAmbigiousCheck(B[i]))
        {
            return true;
        }
    }

    for (std::size_t i = 0; i < X.Size(); i++)
    {
        std::cout << X[i] << std::endl;

        if (! isCloseToZeroForAmbigiousCheck(X[i]))
        {
            return true;
        }
    }

    return false;
}

std::optional<LDLDecResult> GaussHoletskiySolver::ldlDecompose(const Matrix& A, IterationsCounter& itersCounter)
{
    auto n = A.TryGetEdgeSize();

    Matrix L(n, n);
    Matrix D(n, n);

    for (std::size_t y = 0; y < n; y++)
    {
        for (std::size_t x = 0; x < n; x++)
        {
            L.At(y, x) = 0;
            D.At(y, x) = 0;

            itersCounter.AddNew();
        }
    }

    for (std::size_t j = 0; j < n; j++)
    {
        L.At(j, j) = 1;

        {
            double sum = 0;

            for (std::size_t k = 0; k < j; k++)
            {
                sum += std::pow(L.At(j, k), 2) * D.At(k, k);

                itersCounter.AddNew();
            }

            D.At(j, j) = A.At(j, j) - sum;

            itersCounter.AddNew();
        }

        for (std::size_t i = j + 1; i < n; i++)
        {
            auto diagElem = D.At(j, j);

            if (isCloseToZero(diagElem))
            {
                return std::nullopt;
            }

            double sum = 0;

            for (std::size_t k = 0; k < j; k++)
            {
                sum += L.At(i, k) * D.At(k, k) * L.At(j, k);

                itersCounter.AddNew();
            }

            L.At(i, j) = (A.At(i, j) - sum) / D.At(j, j);

            itersCounter.AddNew();
        }
    }

    return LDLDecResult
    {
            .L = L
        , .D = D
    };
}

Vector GaussHoletskiySolver::solveY(const Matrix& L, const Vector& B, IterationsCounter& itersCounter)
{
    auto n = B.Size();

    Vector Y(n);

    for (std::size_t i = 0; i < n; i++)
    {
        double sum = 0;

        for (std::size_t j = 0; j < i; j++)
        {
            sum += L.At(i, j) * Y[j];

            itersCounter.AddNew();
        }

        Y[i] = B[i] - sum;

        itersCounter.AddNew();
    }

    return Y;
}

std::optional<Vector> GaussHoletskiySolver::solveZ(const Matrix& D, const Vector& Y, IterationsCounter& itersCounter)
{
    auto n = Y.Size();

    Vector Z(n);

    for (std::size_t i = 0; i < n; i++)
    {
        auto diagElem = D.At(i, i);

        if (isCloseToZero(diagElem))
        {
            return std::nullopt;

            itersCounter.AddNew();
        }

        Z[i] = Y[i] / diagElem;

        itersCounter.AddNew();
    }

    return Z;
}

Vector GaussHoletskiySolver::solveX(const Matrix& L, const Vector& Z, IterationsCounter& itersCounter)
{
    auto n = Z.Size();

    Vector X(n);

    for (std::ptrdiff_t i = n - 1; i >= 0; i--)
    {
        double sum = 0;

        for (std::size_t j = i + 1; j < n; j++)
        {
            sum += L.At(j, i) * X[j];

            itersCounter.AddNew();
        }

        X[i] = Z[i] - sum;

        itersCounter.AddNew();
    }

    return X;
}

SolvingResult GaussHoletskiySolver::SolveInternally(Matrix&& A, Vector&& B)
{
    IterationsCounter itersCounter{};

    auto mayLDL = ldlDecompose(A, itersCounter);

    if (! mayLDL.has_value())
    {
        return SolvingResult::Error();
    }

    const auto& ldl = mayLDL.value();

    auto Y = solveY(ldl.L, B, itersCounter);
    auto mayZ = solveZ(ldl.D, Y, itersCounter);

    if (! mayZ.has_value())
    {
        return SolvingResult::Error();
    }

    auto X = solveX(ldl.L, mayZ.value(), itersCounter);

    if (! isSolveSuitable(A, B, X))
    {
        return SolvingResult::Error();
    }

    if (! isSolveNotAmbigious(B, X))
    {
        return SolvingResult::Error();
    }

    return SolvingResult::Successful(X).SetItersCountChainly(itersCounter.GetTotalCount());
}
