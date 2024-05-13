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

struct SolvingResult
{
    SolvingResult();

    static SolvingResult Error();
    static SolvingResult Successful(auto&& varsValues)
    {
        SolvingResult solvingResult;

        solvingResult.IsSuccessful = true;
        solvingResult.VarsValues =
            std::forward<decltype(varsValues)>(varsValues);

        return solvingResult;
    }

    SolvingResult& SetItersCount(std::size_t itersCount);

    bool IsSuccessful = false;
    Vector VarsValues{};

    std::size_t ItersCount = 0;
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
    BaseStatus SetVariablesCoefficients(const Matrix& varsCoeffsMatrix);
    BaseStatus SetFreeCoefficients(const Vector& freeCoeffsVector);

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