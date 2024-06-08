#include "RotationSolver.hpp"

#include "../Containers/AllocArray2D.inc.hpp"

#include <cmath>

bool RotationSolver::isCloseToZero(double x)
{
    return std::fabs(x) < 1e-9;
}

std::size_t RotationSolver::suitableDiagLine(const Matrix& A, std::size_t firstLine, IterationsCounter& itersCounter)
{
    double maxValue = std::fabs(A.At(firstLine, firstLine));
    std::size_t maxIndex = firstLine;

    for (std::size_t i = firstLine + 1; i < A.TryGetEdgeSize(); i++)
    {
        auto mayNewMax = std::fabs(A.At(i, firstLine));

        if (mayNewMax > maxValue)
        {
            maxValue = mayNewMax;
            maxIndex = i;
        }

        itersCounter.AddNew();
    }

    return maxIndex;
}

SolvingResult RotationSolver::SolveInternally(Matrix&& A, Vector&& B)
{
    IterationsCounter itersCounter{};

    auto n = B.Size();

    Matrix AB(n, n + 1);

    for (std::size_t y = 0; y < n; y++)
    {
        for (std::size_t x = 0; x < n; x++)
        {
            AB.At(y, x) = A.At(y, x);

            itersCounter.AddNew();
        }

        AB.At(y, n) = B[y];
    }

    for (std::size_t i = 0; i < n - 1; i++)
    {
        auto maxDiagIndex = suitableDiagLine(AB, i, itersCounter);

        if (isCloseToZero(AB.At(maxDiagIndex, i)))
        {
            return SolvingResult::Error();
        }

        for (std::size_t r = 0; r < n + 1; r++)
        {
            std::swap(AB.At(i, r), AB.At(maxDiagIndex, r));

            itersCounter.AddNew();
        }

        for (std::size_t j = i + 1; j < n; j++)
        {
            auto b = AB.At(j, i);
            auto a = AB.At(i, i);
            
            auto squaresSum = a*a + b*b;

            if (! (squaresSum > 0))
            {
                return SolvingResult::Error();
            }

            auto sqrtedSquaresSum = std::sqrt(squaresSum);

            if (isCloseToZero(sqrtedSquaresSum))
            {
                return SolvingResult::Error();
            }

            auto c = a / sqrtedSquaresSum;
            auto s = b / sqrtedSquaresSum;

            for (std::size_t k = i; k < n + 1; k++)
            {
                auto t = AB.At(i, k);

                AB.At(i, k) = c * AB.At(i, k) + s * AB.At(j, k);
                AB.At(j, k) = -s * t + c * AB.At(j, k);

                itersCounter.AddNew();
            }
        }
    }

    Vector X(n);

    for (std::size_t i = 0; i < n; i++)
    {
        itersCounter.AddNew();

        if (isCloseToZero(AB.At(i, i)))
        {
            return SolvingResult::Error();
        }
    }

    for (std::ptrdiff_t i = n - 1; i >= 0; i--)
    {
        double membersSum = 0;

        for (std::size_t j = i + 1; j < n; j++)
        {
            membersSum += AB.At(i, j) * X[j];

            itersCounter.AddNew();
        }

        X[i] = (AB.At(i, n) - membersSum) / AB.At(i, i);
    }

    return SolvingResult::Successful(std::move(X)).SetItersCountChainly(itersCounter.GetTotalCount());
}
