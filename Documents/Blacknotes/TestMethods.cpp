// standard C libraries
#include <cmath>
#include <cstdint>

// standard C++ libraries
#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

// temporarily included libraries
#include <iostream>

#define private public
#define protected public

// ================================
// -------------------------------- UTILITY FUNCTIONS --------------------------------
// ================================

std::optional<std::ptrdiff_t> ToInteger(const std::string& mayFormattedInt)
{
    try
    {
        return std::stoi(mayFormattedInt);
    }
    catch (const std::exception& _)
    {
        return std::nullopt;
    }
}

// ================================
// -------------------------------- CORE DATA TYPES --------------------------------
// ================================

class Matrix
{
public:
    Matrix() = default;

    explicit Matrix(std::size_t height, std::size_t width)
    {
        this->width = width;
        this->height = height;

        flattenMatrixVH = std::vector<double>(width * height);
    }

    double at(std::size_t y, std::size_t x) const
    {
        return flatMtxElemRef(y, x);
    }
    double& at(std::size_t y, std::size_t x)
    {
        return flatMtxElemRef(y, x);
    }

    std::size_t Width() const noexcept
    {
        return width;
    }
    std::size_t Height() const noexcept
    {
        return height;
    }

    std::size_t TryGetEdgeSize() const noexcept
    {
        return width;
    }

    bool IsSquare() const noexcept
    {
        return width == height;
    }

private:
    std::size_t width = 0, height = 0;

    std::vector<double> flattenMatrixVH{};

    double flatMtxElemRef(std::size_t y, std::size_t x) const
    {
        return flattenMatrixVH[width * y + x];
    }
    double& flatMtxElemRef(std::size_t y, std::size_t x)
    {
        return flattenMatrixVH[width * y + x];
    }
};

template<typename T>
class RTArray2D
{
public:
    RTArray2D() = default;
    explicit RTArray2D(std::size_t width, std::size_t height)
    {
        this->width = width;
        this->height = height;

        flattenArray2DVH = std::vector<T>(width * height);
    }

    const T& at(std::size_t y, std::size_t x) const
    {
        return flatArray2DElemRef(y, x);
    }
    T& at(std::size_t y, std::size_t x)
    {
        return flatArray2DElemRef(y, x);
    }

    std::size_t Width() const noexcept
    {
        return width;
    }
    std::size_t Height() const noexcept
    {
        return height;
    }

private:
    std::size_t width = 0, height = 0;

    std::vector<T> flattenArray2DVH{};

    const T& flatArray2DElemRef(std::size_t y, std::size_t x) const
    {
        return flattenArray2DVH[width * y + x];
    }
    T& flatArray2DElemRef(std::size_t y, std::size_t x)
    {
        return flattenArray2DVH[width * y + x];
    }
};

class Vector
{
public:
    Vector() = default;
    explicit Vector(std::size_t vectorSize)
    {
        numbersVector.resize(vectorSize, 0);
    }
    double operator[](std::size_t index) const
    {
        return numbersVector[index];
    }
    double& operator[](std::size_t index)
    {
        return numbersVector[index];
    }
    std::size_t Size() const noexcept
    {
        return numbersVector.size();
    }

private:
    std::vector<double> numbersVector{};
};



struct SolvingResult
{
    SolvingResult() = default;

    static SolvingResult Error()
    {
        SolvingResult solvingResult;

        solvingResult.IsSuccessful = false;

        return solvingResult;
    }
    static SolvingResult Successful(auto&& varsValues)
    {
        SolvingResult solvingResult;

        solvingResult.IsSuccessful = true;
        solvingResult.VarsValues =
            std::forward<decltype(varsValues)>(varsValues);

        return solvingResult;
    }

    bool IsSuccessful = false;
    Vector VarsValues{};
};

// ================================
// -------------------------------- ABSTRACT SOLVING OBJECT --------------------------------
// ================================

enum class BaseStatus : bool
{
    Success, Error
};

class SLESolver
{
public:
    SLESolver() = default;

    BaseStatus SetEquationsCount(std::size_t equationsCount)
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

    BaseStatus SetVariablesCoefficients(const Matrix& varsCoeffsMatrix)
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

    BaseStatus SetFreeCoefficients(const Vector& freeCoeffsVector)
    {
        using enum BaseStatus;

        if (! (freeCoeffsVector.Size() == equationsCount))
        {
            return Error;
        }

        this->freeCoeffsVector = freeCoeffsVector;

        return Success;
    }

    void Solve()
    {
        if (isSolvingApplied)
        {
            return;
        }
        auto solvingResult = SolveInternally(
              std::move(varsCoeffsMatrix)
            , std::move(freeCoeffsVector)
        );

        isSolvingApplied = true;
        isLSESoledSuccessfully = solvingResult.IsSuccessful;

        if (isLSESoledSuccessfully)
        {
            variablesValues = std::move(solvingResult.VarsValues);
        }
    }

    std::optional<bool> IsSolvedSuccessfully() const
    {
        if (! isSolvingApplied)
        {
            return std::nullopt;
        }
        return isLSESoledSuccessfully;
    }

    Vector& GetVariablesValuesUnsafely()
    {
        return variablesValues;
    }

    virtual ~SLESolver() = default;

protected:
    virtual SolvingResult SolveInternally(Matrix&& A, Vector&& B) = 0;

    std::size_t equationsCount = 0;

    Matrix varsCoeffsMatrix{};
    Vector freeCoeffsVector{};

    Vector variablesValues{};

    bool isEquationsCountSetted = false;

    bool isSolvingApplied       = false;
    bool isLSESoledSuccessfully = false;
};

// ================================
// -------------------------------- SPECIFIC SOLVING METHODS --------------------------------
// ================================

#define CLASS_CONST static constexpr

struct LUPDecResult
{
    Matrix L, U;
    std::vector<std::size_t> P;
};

// TODO: implement...
class LUPSolver : public SLESolver
{
public:
    ~LUPSolver() override = default;

private:
    static bool isCloseToZero(double x)
    {
        return std::fabs(x) < 1e-9;
    }

    static std::size_t maxDiagLine(const Matrix& A, std::size_t baseColumn)
    {
        auto maxDiagValue = std::fabs(A.at(baseColumn, baseColumn));
        std::size_t indexOfMax = baseColumn;

        for (std::size_t curColumn = 0; curColumn < A.TryGetEdgeSize(); curColumn++)
        {
            auto newDiagValue = std::fabs(A.at(curColumn, baseColumn));

            if (newDiagValue > maxDiagValue)
            {
                maxDiagValue = newDiagValue;
                indexOfMax = curColumn;
            }
        }

        return indexOfMax;
    }

    static std::optional<LUPDecResult> lupDecompose(Matrix A)
    {
        auto n = A.TryGetEdgeSize();

        std::vector<std::size_t> P(n);

        for (std::size_t i = 0; i < n; i++)
        {
            P[i] = i;
        }

        for (std::size_t j = 0; j < n; j++)
        {
            auto maxDiagColumn = maxDiagLine(A, j);

            if (isCloseToZero(A.at(maxDiagColumn, j)))
            {
                return std::nullopt;
            }

            for (std::size_t r = 0; r < n; r++)
            {
                std::swap(A.at(j, r), A.at(maxDiagColumn, r));
            }

            std::swap(P[j], P[maxDiagColumn]);

            for (std::size_t i = j; i < n; i++)
            {
                {
                    double sum = 0;

                    for (std::size_t k = 0; k < j; k++)
                    {
                        sum += A.at(i, k) * A.at(k, j);
                    }

                    A.at(i, j) -= sum;
                }

                if (i >= j + 1)
                {
                    double sum = 0;

                    for (std::size_t k = 0; k < j; k++)
                    {
                        sum += A.at(j, k) * A.at(k, i);
                    }

                    A.at(j, i) -= sum;

                    if (isCloseToZero(A.at(j, j)))
                    {
                        return std::nullopt;
                    }

                    A.at(j, i) /= A.at(j, j);
                }
            }
        }

        Matrix L(n, n);
        Matrix U(n, n);

        for (std::size_t y = 0; y < n; y++)
        {
            for (std::size_t x = 0; x < n; x++)
            {
                L.at(y, x) = 0;
            }
            U.at(y, y) = 1;
        }

        for (std::size_t j = 0; j < n; j++)
        {
            for (std::size_t i = 0; i < j + 1; i++)
            {
                L.at(j, i) = A.at(j, i);
            }
        }

        for (std::size_t j = 0; j < n; j++)
        {
            for (std::size_t i = j + 1; i < n; i++)
            {
                U.at(j, i) = A.at(j, i);
            }
        }

        return LUPDecResult {
              .L = L
            , .U = U
            , .P = P
        };
    }

    static std::optional<Vector> solveY(
          const Matrix& L
        , const std::vector<std::size_t>& P
        , const Vector& B
    )
    {
        auto n = B.Size();

        Vector Y(n);

        for (std::size_t i = 0; i < n; i++)
        {
            if (isCloseToZero(L.at(i, i)))
            {
                return std::nullopt;
            }
        }

        for (std::size_t i = 0; i < n; i++)
        {
            double sum = 0;

            for (std::size_t k = 0; k < i; k++)
            {
                sum += L.at(i, k) * Y[k];
            }

            Y[i] = (
                B[P[i]] - sum
            )
            / L.at(i, i);
        }

        return Y;
    }

    static Vector solveX(const Matrix& U, const Vector& Y)
    {
        auto n = Y.Size();

        Vector X(n);

        for (std::ptrdiff_t i = n - 1; i >= 0; i--)
        {
            double sum = 0;

            for (std::size_t k = i + 1; k < n; k++)
            {
                sum += U.at(i, k) * X[k];
            }

            X[i] = Y[i] - sum;
        }

        return X;
    }

protected:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B) override
    {
        auto mayLUPDecRes = lupDecompose(A);

        if (! mayLUPDecRes.has_value())
        {
            return SolvingResult::Error();
        }

        const auto& lup = mayLUPDecRes.value();

        auto mayY = solveY(lup.L, lup.P, B);

        if (! mayY.has_value())
        {
            return SolvingResult::Error();
        }

        auto X = solveX(lup.U, mayY.value());

        return SolvingResult::Successful(std::move(X));
    }
};

struct LDLDecResult
{
    Matrix L, D;
};

// TODO: implement...
class GaussHoletskiySolver : public SLESolver
{
public:
    ~GaussHoletskiySolver() override = default;

private:
    static bool isCloseToZero(double x)
    {
        return std::fabs(x) < 1e-9;
    }

    static bool isCloseToZeroForSolves(double x)
    {
        return std::fabs(x) < 1e-9;
    }

    static bool isSolveSuitable(const Matrix& A, const Vector& B, const Vector& X)
    {
        auto n = B.Size();

        Vector NewB(n);

        for (std::size_t y = 0; y < n; y++)
        {
            NewB[y] = 0;

            for (std::size_t x = 0; x < n; x++)
            {
                NewB[y] += A.at(y, x) * X[x];
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

    static std::optional<LDLDecResult> ldlDecompose(const Matrix& A)
    {
        auto n = A.TryGetEdgeSize();

        Matrix L(n, n);
        Matrix D(n, n);

        for (std::size_t y = 0; y < n; y++)
        {
            for (std::size_t x = 0; x < n; x++)
            {
                L.at(y, x) = 0;
                D.at(y, x) = 0;
            }
        }

        for (std::size_t j = 0; j < n; j++)
        {
            L.at(j, j) = 1;

            {
                double sum = 0;

                for (std::size_t k = 0; k < j; k++)
                {
                    sum += std::pow(L.at(j, k), 2) * D.at(k, k);
                }

                D.at(j, j) = A.at(j, j) - sum;
            }

            for (std::size_t i = j + 1; i < n; i++)
            {
                auto diagElem = D.at(j, j);

                if (isCloseToZero(diagElem))
                {
                    return std::nullopt;
                }

                double sum = 0;

                for (std::size_t k = 0; k < j; k++)
                {
                    sum += L.at(i, k) * D.at(k, k) * L.at(j, k);
                }

                L.at(i, j) = (A.at(i, j) - sum) / D.at(j, j);
            }
        }

        return LDLDecResult
        {
              .L = L
            , .D = D
        };
    }

    static Vector solveY(const Matrix& L, const Vector& B)
    {
        auto n = B.Size();

        Vector Y(n);

        for (std::size_t i = 0; i < n; i++)
        {
            double sum = 0;

            for (std::size_t j = 0; j < i; j++)
            {
                sum += L.at(i, j) * Y[j];
            }

            Y[i] = B[i] - sum;
        }

        return Y;
    }

    static std::optional<Vector> solveZ(const Matrix& D, const Vector& Y)
    {
        auto n = Y.Size();

        Vector Z(n);

        for (std::size_t i = 0; i < n; i++)
        {
            auto diagElem = D.at(i, i);

            if (isCloseToZero(diagElem))
            {
                return std::nullopt;
            }

            Z[i] = Y[i] / diagElem;
        }

        return Z;
    }

    static Vector solveX(const Matrix& L, const Vector& Z)
    {
        auto n = Z.Size();

        Vector X(n);

        for (std::ptrdiff_t i = n - 1; i >= 0; i--)
        {
            double sum = 0;

            for (std::size_t j = i + 1; j < n; j++)
            {
                sum += L.at(j, i) * X[j];
            }

            X[i] = Z[i] - sum;
        }

        return X;
    }

protected:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B) override
    {
        auto mayLDL = ldlDecompose(A);

        if (! mayLDL.has_value())
        {
            return SolvingResult::Error();
        }

        const auto& ldl = mayLDL.value();

        auto Y = solveY(ldl.L, B);
        auto mayZ = solveZ(ldl.D, Y);

        if (! mayZ.has_value())
        {
            return SolvingResult::Error();
        }

        auto X = solveX(ldl.L, mayZ.value());

        if (! isSolveSuitable(A, B, X))
        {
            return SolvingResult::Error();
        }

        return SolvingResult::Successful(X);
    }
};

// TODO: implement...
class RotationSolver : public SLESolver
{
public:
    ~RotationSolver() override = default;

private:
    static bool isCloseToZero(double x)
    {
        return std::fabs(x) < 1e-9;
    }

protected:
    SolvingResult SolveInternally(Matrix&& A, Vector&& B) override
    {
        auto n = B.Size();

        RTArray2D<double> AB(n + 1, n);

        for (std::size_t y = 0; y < n; y++)
        {
            for (std::size_t x = 0; x < n; x++)
            {
                AB.at(y, x) = A.at(y, x);
            }

            AB.at(y, n) = B[y];
        }

        for (std::size_t i = 0; i < n - 1; i++)
        {
            for (std::size_t j = i + 1; j < n; j++)
            {
                auto b = AB.at(j, i);
                auto a = AB.at(i, i);
                
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
                    auto t = AB.at(i, k);

                    AB.at(i, k) = c * AB.at(i, k) + s * AB.at(j, k);
                    AB.at(j, k) = -s * t + c * AB.at(j, k);
                }
            }
        }

        Vector X(n);

        for (std::size_t i = 0; i < n; i++)
        {
            if (isCloseToZero(AB.at(i, i)))
            {
                return SolvingResult::Error();
            }
        }

        for (std::ptrdiff_t i = n - 1; i >= 0; i--)
        {
            double membersSum = 0;

            for (std::size_t j = i + 1; j < n; j++)
            {
                membersSum += AB.at(i, j) * X[j];
            }

            X[i] = (AB.at(i, n) - membersSum) / AB.at(i, i);
        }

        return SolvingResult::Successful(std::move(X));
    }
};

int main()
{
    std::unique_ptr<SLESolver> solverUP{new GaussHoletskiySolver()};

    auto& solver = *solverUP;

    Matrix A(3, 3);
    A.at(0, 0) =   9;
    A.at(0, 1) =  -2;
    A.at(0, 2) =  -6;
    A.at(1, 0) =  -2;
    A.at(1, 1) =  16;
    A.at(1, 2) = -12;
    A.at(2, 0) =  -6;
    A.at(2, 1) = -12;
    A.at(2, 2) =  18;

    Vector B(3);
    B[0] = -13;
    B[1] =  -6;
    B[2] =  24;

    solver.SetEquationsCount(3);
    solver.SetVariablesCoefficients(A);
    solver.SetFreeCoefficients(B);

    solver.Solve();

    auto X = solver.GetVariablesValuesUnsafely();

    for (int i = 0; i < X.Size(); i++)
    {
        std::cout << X[i] << std::endl;
    }
}
