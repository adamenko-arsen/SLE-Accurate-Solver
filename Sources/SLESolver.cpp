#include "SLESolver.hpp"

#include <cstdio>

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
    this->itersCount = itersCount;

    return *this;
}

// class LSESolver

SLESolver::SLESolver() = default;
SLESolver::~SLESolver() = default;

void SLESolver::SetEquationsCount(std::size_t equationsCount)
{
    if (isEquationsCountSetted)
    {
        return;
    }

    if (! (equationsCount >= 1))
    {
        return;
    }

    this->equationsCount = equationsCount;
    isEquationsCountSetted = true;
}

void SLESolver::Solve()
{
    if (isSolvingApplied)
    {
        return;
    }
    auto solvingResult = SolveInternally
    (
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

std::optional<bool> SLESolver::IsSolvedSuccessfully() const
{
    if (! isSolvingApplied)
    {
        return std::nullopt;
    }
    return isLSESoledSuccessfully;
}

std::optional<Vector> SLESolver::GetSolveOnce()
{
    if (! (isLSESoledSuccessfully && isSolvesKeeped))
    {
        return std::nullopt;
    }
    isSolvesKeeped = false;

    return std::move(variablesValues);
}

std::optional<std::size_t> SLESolver::GetAlgoItersCount()
{
    if (! (isSolvingApplied && isLSESoledSuccessfully))
    {
        return std::nullopt;
    }
    return totalIterationsCount;
}
