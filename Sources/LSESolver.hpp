#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"

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

enum class BaseStatus : bool
{
    Success, Error
};

class LSESolver
{
public:
    LSESolver();
    virtual ~LSESolver();

    BaseStatus SetEquationsCount(std::size_t equationsCount);
    BaseStatus SetVariablesCoefficients(auto&& varsCoeffsMatrix)
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

        this->varsCoeffsMatrix = std::forward<decltype(varsCoeffsMatrix)>(varsCoeffsMatrix);

        return Success;
    }

    BaseStatus SetFreeCoefficients(auto&& freeCoeffsVector)
    {
        using enum BaseStatus;

        if (! (freeCoeffsVector.Size() == equationsCount))
        {
            return Error;
        }

        this->freeCoeffsVector = std::forward<decltype(freeCoeffsVector)>(freeCoeffsVector);

        return Success;
    }

    void SolveLSE();

    std::optional<bool> IsLSESoledSuccessfully() const;
    std::optional<Vector> GetVariablesSolvesOnce();

    std::optional<std::size_t> GetTotalIterationsCount();

protected:
    virtual SolvingResult SolveLSEInternal(Matrix&& A, Vector&& B) = 0;

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