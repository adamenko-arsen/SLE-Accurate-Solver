#include "LUPSolver.hpp"

#include <cmath>

bool LUPSolver::isCloseToZero(double x)
{
    return std::fabs(x) < 1e-9;
}

std::size_t LUPSolver::maxDiagLine(const Matrix& A, std::size_t baseColumn)
{
    auto maxDiagValue = std::fabs(A.At(baseColumn, baseColumn));
    std::size_t indexOfMax = baseColumn;

    for (std::size_t curColumn = 0; curColumn < A.TryGetEdgeSize(); curColumn++)
    {
        auto newDiagValue = std::fabs(A.At(curColumn, baseColumn));

        if (newDiagValue > maxDiagValue)
        {
            maxDiagValue = newDiagValue;
            indexOfMax = curColumn;
        }
    }

    return indexOfMax;
}

std::optional<LUPDecResult> LUPSolver::lupDecompose(Matrix A, IterationsCounter& itersCounter)
{
    auto n = A.TryGetEdgeSize();

    std::vector<std::size_t> P(n);

    for (std::size_t i = 0; i < n; i++)
    {
        P[i] = i;

        itersCounter.AddNew();
    }

    for (std::size_t j = 0; j < n; j++)
    {
        auto maxDiagColumn = maxDiagLine(A, j);

        if (isCloseToZero(A.At(maxDiagColumn, j)))
        {
            return std::nullopt;
        }

        for (std::size_t r = 0; r < n; r++)
        {
            std::swap(A.At(j, r), A.At(maxDiagColumn, r));

            itersCounter.AddNew();
        }

        std::swap(P[j], P[maxDiagColumn]);

        for (std::size_t i = j; i < n; i++)
        {
            {
                double sum = 0;

                for (std::size_t k = 0; k < j; k++)
                {
                    sum += A.At(i, k) * A.At(k, j);

                    itersCounter.AddNew();
                }

                A.At(i, j) -= sum;

                itersCounter.AddNew();
            }

            if (i >= j + 1)
            {
                double sum = 0;

                for (std::size_t k = 0; k < j; k++)
                {
                    sum += A.At(j, k) * A.At(k, i);

                    itersCounter.AddNew();
                }

                A.At(j, i) -= sum;

                if (isCloseToZero(A.At(j, j)))
                {
                    return std::nullopt;
                }

                A.At(j, i) /= A.At(j, j);

                itersCounter.AddNew();
            }
        }
    }

    Matrix L(n, n);
    Matrix U(n, n);

    for (std::size_t y = 0; y < n; y++)
    {
        for (std::size_t x = 0; x < n; x++)
        {
            L.At(y, x) = 0;

            itersCounter.AddNew();
        }
        U.At(y, y) = 1;

        itersCounter.AddNew();
    }

    for (std::size_t j = 0; j < n; j++)
    {
        for (std::size_t i = 0; i < j + 1; i++)
        {
            L.At(j, i) = A.At(j, i);

            itersCounter.AddNew();
        }
    }

    for (std::size_t j = 0; j < n; j++)
    {
        for (std::size_t i = j + 1; i < n; i++)
        {
            U.At(j, i) = A.At(j, i);

            itersCounter.AddNew();
        }
    }

    return LUPDecResult {
            .L = L
        , .U = U
        , .P = P
    };
}

std::optional<Vector> LUPSolver::solveY(
        const Matrix& L
    , const std::vector<std::size_t>& P
    , const Vector& B
    , IterationsCounter& itersCounter
)
{
    auto n = B.Size();

    Vector Y(n);

    for (std::size_t i = 0; i < n; i++)
    {
        if (isCloseToZero(L.At(i, i)))
        {
            return std::nullopt;

            itersCounter.AddNew();
        }
    }

    for (std::size_t i = 0; i < n; i++)
    {
        double sum = 0;

        for (std::size_t k = 0; k < i; k++)
        {
            sum += L.At(i, k) * Y[k];

            itersCounter.AddNew();
        }

        Y[i] = (
            B[P[i]] - sum
        )
        / L.At(i, i);

        itersCounter.AddNew();
    }

    return Y;
}

Vector LUPSolver::solveX(const Matrix& U, const Vector& Y, IterationsCounter& itersCounter)
{
    auto n = Y.Size();

    Vector X(n);

    for (std::ptrdiff_t i = n - 1; i >= 0; i--)
    {
        double sum = 0;

        for (std::size_t k = i + 1; k < n; k++)
        {
            sum += U.At(i, k) * X[k];

            itersCounter.AddNew();
        }

        X[i] = Y[i] - sum;

        itersCounter.AddNew();
    }

    return X;
}

SolvingResult LUPSolver::SolveInternally(Matrix&& A, Vector&& B)
{
    IterationsCounter itersCounter{};

    auto mayLUPDecRes = lupDecompose(A, itersCounter);

    if (! mayLUPDecRes.has_value())
    {
        return SolvingResult::Error();
    }

    const auto& lup = mayLUPDecRes.value();

    auto mayY = solveY(lup.L, lup.P, B, itersCounter);

    if (! mayY.has_value())
    {
        return SolvingResult::Error();
    }

    auto X = solveX(lup.U, mayY.value(), itersCounter);

    return SolvingResult::Successful(std::move(X)).SetItersCountChainly(itersCounter.GetTotalCount());
}
