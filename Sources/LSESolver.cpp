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

    solvingResult.isSuccessful = false;

    return solvingResult;
}

bool SolvingResult::GetSuccessfulness() const
{
    return isSuccessful;
}
Vector& SolvingResult::GetVarsValuesRef()
{
    return varsValues;
}

SolvingResult& SolvingResult::SetItersCountChainly(std::size_t itersCount)
{
    itersCount = itersCount;

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
    isLSESoledSuccessfully = solvingResult.GetSuccessfulness();

    if (isLSESoledSuccessfully)
    {
        variablesValues = std::move(solvingResult.GetVarsValuesRef());
        totalIterationsCount = solvingResult.GetItersCount();
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
