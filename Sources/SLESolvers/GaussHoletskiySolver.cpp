#include "GaussHoletskiySolver.hpp"

#include <cmath>

bool GaussHoletskiySolver::isCloseToZero(double x)
{
    return std::fabs(x) < 1e-12;
}

bool GaussHoletskiySolver::isCloseToZeroForSolves(double x)
{
    return std::fabs(x) < 1e-12;
}

bool GaussHoletskiySolver::isCloseToZeroForAmbigiousCheck(double x)
{
    return std::fabs(x) < 1e-12;
}

bool GaussHoletskiySolver::isSolveSuitable(const Matrix& A, const Vector& B, const Vector& X, IterationsCounter& itersCounter)
{
    auto n = B.Size();

    Vector NewB(n);

    for (std::size_t y = 0; y < n; y++)
    {
        NewB[y] = 0;

        for (std::size_t x = 0; x < n; x++)
        {
            NewB[y] += A.At(y, x) * X[x];

            itersCounter.AddNew();
        }
    }

    for (std::size_t y = 0; y < n; y++)
    {
        itersCounter.AddNew();

        if (! isCloseToZeroForSolves(B[y] - NewB[y]))
        {
            return false;
        }
    }

    return true;
}

std::optional<Matrix> GaussHoletskiySolver::llDecompose(const Matrix& A, IterationsCounter& itersCounter)
{
    auto n = A.TryGetEdgeSize();

    Matrix L(n, n);

    for (std::size_t j = 0; j < n; j++)
    {
        double sum = 0;

        for (std::size_t k = 0; k < j; k++)
        {
            sum += std::pow(L.At(j, k), 2);

            itersCounter.AddNew();
        }

        L.At(j, j) = std::sqrt(A.At(j, j) - sum);

        for (std::size_t i = j + 1; i < n; i++)
        {
            double sum = 0;

            for (std::size_t k = 0; k < j; k++)
            {
                sum += L.At(i, k) * L.At(j, k);

                itersCounter.AddNew();
            }

            L.At(i, j) = (A.At(i, j) - sum) / L.At(j, j);
        }
    }

    for (std::size_t y = 0; y < n; y++)
    {
        for (std::size_t x = y + 1; x < n; x++)
        {
            L.At(y, x) = 0;

            itersCounter.AddNew();
        }
    }

    return L;
}

std::optional<Vector> GaussHoletskiySolver::solveY(const Matrix& L, const Vector& B, IterationsCounter& itersCounter)
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

        Y[i] = (B[i] - sum) / L.At(i, i);
    }

    return Y;
}

std::optional<Vector> GaussHoletskiySolver::solveX(const Matrix& L, const Vector& Y, IterationsCounter& itersCounter)
{
    auto n = Y.Size();

    Vector X(n);

    for (std::ptrdiff_t i = n - 1; i >= 0; i--)
    {
        double sum = 0;

        for (std::size_t j = i + 1; j < n; j++)
        {
            sum += L.At(j, i) * X[j];

            itersCounter.AddNew();
        }

        X[i] = (Y[i] - sum) / L.At(i, i);
    }

    return X;
}

SolvingResult GaussHoletskiySolver::SolveInternally(Matrix&& A, Vector&& B)
{
    IterationsCounter itersCounter{};

    auto mayL = llDecompose(A, itersCounter);
    if (! mayL)
    {
        return SolvingResult::Error();
    }
    auto& L = mayL.value();

    auto mayY = solveY(L, B, itersCounter);
    if (! mayY)
    {
        return SolvingResult::Error();
    }
    auto& Y = mayY.value();

    auto mayX = solveX(L, Y, itersCounter);
    if (! mayX)
    {
        return SolvingResult::Error();
    }
    auto& X = mayX.value();

    if (! isSolveSuitable(A, B, X, itersCounter))
    {
        return SolvingResult::Error();
    }

    return SolvingResult::Successful
    (
        std::move(X)
    )
    .SetItersCountChainly(itersCounter.GetTotalCount());
}
