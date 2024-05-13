#include "LSESolver.hpp"

// class IterationsCounter

IterationsCounter::IterationsCounter() = default;

void IterationsCounter::AddNew() noexcept
{
    itersCount++;
}
std::size_t IterationsCounter::GetTotalCount() const noexcept
{
    return itersCount;
}

// class SolvingResult

SolvingResult::SolvingResult() = default;

SolvingResult SolvingResult::Error()
{
    SolvingResult solvingResult;

    solvingResult.IsSuccessful = false;

    return solvingResult;
}

SolvingResult& SolvingResult::SetItersCount(std::size_t itersCount)
{
    ItersCount = itersCount;

    return *this;
}

// class LSESolver

LSESolver::LSESolver() = default;
LSESolver::~LSESolver() = default;

BaseStatus LSESolver::SetEquationsCount(std::size_t equationsCount)
{
    using enum BaseStatus;

    if (isEquationsCountSetted)
    {
        return Error;
    }

    if (! (equationsCount >= 1))
    {
        return Error;
    }

    this->equationsCount = equationsCount;
    isEquationsCountSetted = true;

    return Success;
}

BaseStatus LSESolver::SetVariablesCoefficients(const Matrix& varsCoeffsMatrix)
{
    using enum BaseStatus;

    if (! varsCoeffsMatrix.IsSquare())
    {
        return Error;
    }
    if (! (varsCoeffsMatrix.TryGetEdgeSize() == equationsCount))
    {
        return Error;
    }

    this->varsCoeffsMatrix = varsCoeffsMatrix;

    return Success;
}

BaseStatus LSESolver::SetFreeCoefficients(const Vector& freeCoeffsVector)
{
    using enum BaseStatus;

    if (! (freeCoeffsVector.Size() == equationsCount))
    {
        return Error;
    }

    this->freeCoeffsVector = freeCoeffsVector;

    return Success;
}

void LSESolver::SolveLSE()
{
    if (isSolvingApplied)
    {
        return;
    }
    auto solvingResult = SolveLSEInternal(
            std::move(varsCoeffsMatrix)
        , std::move(freeCoeffsVector)
    );

    isSolvingApplied = true;
    isLSESoledSuccessfully = solvingResult.IsSuccessful;

    if (isLSESoledSuccessfully)
    {
        variablesValues = std::move(solvingResult.VarsValues);
        totalIterationsCount = solvingResult.ItersCount;
    }
}

std::optional<bool> LSESolver::IsLSESoledSuccessfully() const
{
    if (! isSolvingApplied)
    {
        return std::nullopt;
    }
    return isLSESoledSuccessfully;
}

std::optional<Vector> LSESolver::GetVariablesSolvesOnce()
{
    if (! (isLSESoledSuccessfully && isSolvesKeeped))
    {
        return std::nullopt;
    }
    isSolvesKeeped = false;

    return std::move(variablesValues);
}

std::optional<std::size_t> LSESolver::GetTotalIterationsCount()
{
    if (! (isSolvingApplied && isLSESoledSuccessfully))
    {
        return std::nullopt;
    }
    return totalIterationsCount;
}
