#pragma once

#include "Containers/Matrix.hpp"
#include "Containers/Vector.hpp"

#include <cstdint>

#include <optional>
#include <functional>

class IterationsCounter
{
public:
    IterationsCounter();

    void AddNew() noexcept;
    std::size_t GetTotalCount() const noexcept;

private:
    std::size_t itersCount = 0;
};

class SolvingResult
{
public:
    SolvingResult();

    static SolvingResult Error();
    static SolvingResult Successful(auto&& varsValues)
    {
        SolvingResult solvingResult;

        solvingResult.isSuccessful = true;
        solvingResult.varsValues =
            std::forward<decltype(varsValues)>(varsValues);

        return solvingResult;
    }
    bool GetSuccessfulness() const;
    Vector& GetVarsValuesRef();
    std::size_t GetItersCount() const
    {
        return itersCount;
    }

    SolvingResult& SetItersCountChainly(std::size_t itersCount);

private:
    bool isSuccessful = false;
    Vector varsValues{};

    std::size_t itersCount = 0;
};

class SLESolver
{
public:
    SLESolver();
    virtual ~SLESolver();

    void SetEquationsCount(std::size_t equationsCount);
    void SetVariablesCoefficients(auto&& varsCoeffsMatrix)
    {
        if (! varsCoeffsMatrix.IsSquare())
        {
            return;
        }
        if (! (varsCoeffsMatrix.TryGetEdgeSize() == equationsCount))
        {
            return;
        }

        this->varsCoeffsMatrix = std::forward<decltype(varsCoeffsMatrix)>(varsCoeffsMatrix);
    }

    void SetFreeCoefficients(auto&& freeCoeffsVector)
    {
        if (! (freeCoeffsVector.Size() == equationsCount))
        {
            return;
        }

        this->freeCoeffsVector = std::forward<decltype(freeCoeffsVector)>(freeCoeffsVector);
    }

    void Solve();

    std::optional<bool> IsSolvedSuccessfully() const;
    std::optional<Vector> GetSolveOnce();

    std::optional<std::size_t> GetAlgoItersCount();

protected:
    virtual SolvingResult SolveInternally(Matrix&& A, Vector&& B) = 0;

    std::size_t equationsCount = 0;

    Matrix varsCoeffsMatrix{};
    Vector freeCoeffsVector{};

    Vector variablesValues{};

    std::size_t totalIterationsCount = 0;

    bool isEquationsCountSetted = false;

    bool isSolvingApplied       = false;
    bool isLSESoledSuccessfully = false;

    bool isSolvesKeeped = true;
};