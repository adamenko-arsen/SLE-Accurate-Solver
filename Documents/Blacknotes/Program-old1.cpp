// third-party well-known external libraries
#include <gtkmm.h>

// standard C libraries
#include <cmath>
#include <cstdint>

// standard C++ libraries
#include <chrono>
#include <format>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vector>

// temporarily included libraries
#include <iostream>

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
class AllocArray2D
{
public:
    AllocArray2D() = default;
    explicit AllocArray2D(std::size_t width, std::size_t height)
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

class LSESolver
{
public:
    LSESolver() = default;

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

    void SolveLSE()
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
        }
    }

    std::optional<bool> IsLSESoledSuccessfully() const
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

    virtual ~LSESolver() = default;

protected:
    virtual SolvingResult SolveLSEInternal(Matrix&& A, Vector&& B) = 0;

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

class LUPSolver : public LSESolver
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
    SolvingResult SolveLSEInternal(Matrix&& A, Vector&& B) override
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

class GaussHoletskiySolver : public LSESolver
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
    SolvingResult SolveLSEInternal(Matrix&& A, Vector&& B) override
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

class RotationSolver : public LSESolver
{
public:
    ~RotationSolver() override = default;

private:
    static bool isCloseToZero(double x)
    {
        return std::fabs(x) < 1e-9;
    }

protected:
    SolvingResult SolveLSEInternal(Matrix&& A, Vector&& B) override
    {
       auto n = B.Size();

        AllocArray2D<double> AB(n + 1, n);

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

enum LSESolvingMethodIndex
{
      LUP            = 0
    , GaussHoletskiy = 1
    , Rotation       = 2
};

std::unique_ptr<LSESolver> LSESolverFactoryProduce(LSESolvingMethodIndex solverIndex)
{
    using enum LSESolvingMethodIndex;

    std::unique_ptr<LSESolver> abstractSolver{};

    if (solverIndex == LUP)
    {
        abstractSolver.reset(new LUPSolver());
    }
    else if (solverIndex == GaussHoletskiy)
    {
        abstractSolver.reset(new GaussHoletskiySolver());
    }
    else if (solverIndex == Rotation)
    {
        abstractSolver.reset(new RotationSolver());
    }
    else
    {
        throw std::runtime_error("cannot get the suitable solver method by its index");
    }

    return abstractSolver;
}

std::string GetCurrentFormalTime()
{
    auto t = time(NULL);

    return std::format("{}:{}:{} UTC+00:00"
        , t / 3600 % 24
        , t / 60 % 60
        , t % 60
    );
}

// ================================
// -------------------------------- APPLICATION GUI DECLARATION --------------------------------
// ================================

struct LSEInputData
{
    std::size_t eqsCount = 0;
    Matrix A{};
    Vector B{};

    bool IsSetted = false;
};

enum class LSESolvingStatus : uint8_t
{
    Unsolved, SolvedSuccessfuly, SolvedFailful
};

struct LSESolveData
{
    Vector X{};

    LSESolvingStatus SolvingStatus = LSESolvingStatus::Unsolved;
};

class ApplicationData
{
public:
    ApplicationData() = default;

    std::weak_ptr<LSEInputData> GetLSEInputDataRef()
    {
        return std::weak_ptr<LSEInputData>(lseInputData);
    }
    std::weak_ptr<LSESolveData> GetLSEOutputDataRef()
    {
        return std::weak_ptr<LSESolveData>(lseSolves);
    }

private:
    std::shared_ptr<LSEInputData> lseInputData
        = std::make_shared<LSEInputData>();

    std::shared_ptr<LSESolveData> lseSolves
        = std::make_shared<LSESolveData>();
};

class LSEConfigurator : public Gtk::Frame
{
public:
    LSEConfigurator();

    void SetLSEInputData(std::weak_ptr<LSEInputData> lseInputData)
    {
        this->lseData = lseInputData;
    }

    void FrozeForm()
    {
        set_sensitive(false);
    }

    void ResumeForm()
    {
        set_sensitive(true);
    }

private:
    Gtk::Box boxLayout{};

    Gtk::Grid eqsPropConfGrid{};

    Gtk::Alignment eqsCountFirstHorAlign{};

    Gtk::Label eqsConfStatus{"Невідомий статус"};

    Gtk::Label eqsCountProp{"Кількість рівнянь:"};
    Gtk::Label eqsCountValue{"не вказано"};

    Gtk::Label  eqsSetterProp{"Встановити кількість:"};
    Gtk::Entry  eqsSetterEntry{};
    Gtk::Button eqsSetterButton{"Нова СЛАР"};

    Gtk::Button eqsZeroFillerButton{"Заповнити пусті кліт. 0-ми"};
    Gtk::Button eqsSetAsInput{"Встановити дану СЛАР"};

    void setEqsAsInput();

    void initializeEqsCount();

    void onEqsCountSetting();
    void createEqsForm(std::size_t eqsCount);
    void removeEqsForm();

    void fillEmptyEntriesWithZeroes();

    Gtk::Box eqsFormBox{};

    Gtk::Grid varsCoeffsGrid{};
    Gtk::Grid freeCoeffsGrid{};

    AllocArray2D<Gtk::Entry> varsCoeffsEntries{};
    std::vector<Gtk::Entry>  freeCoeffsEntries{};

    void initializeEqsForm();

    std::weak_ptr<LSEInputData> lseData{};
};

class LSESolveOutput : public Gtk::Frame
{
public:
    LSESolveOutput();

    void SetLSEOutputDataRef(std::weak_ptr<LSESolveData> lseSolveData)
    {
        this->lseSolveData = lseSolveData;
    }
    void SetLSEInputDataRef(std::weak_ptr<LSEInputData> lseInputData)
    {
        this->lseInputData = lseInputData;
    }

    void OutputSolves()
    {
        outputStatus.set_text(std::format("Рішення виведено {}", GetCurrentFormalTime()));

        if (lseInputData.lock()->eqsCount == 2)
        {
            UpdateVarsValues();
        }
        else
        {
            ClearVarsValues();
        }
    }

    void UpdateVarsValues();
    void ClearVarsValues();

private:
    Gtk::Box boxLayout{};

    Gtk::Label outputStatus{"Невідомий статус"};
    Gtk::Entry outputFileName{};
    Gtk::Button outputButton{"Вивести в файл"};
    Gtk::Label varsDesc{"Рішення СЛАР:"};

    Gtk::DrawingArea graphicOutput{};

    std::size_t varsCount = 2;

    void saveSolve();

    std::weak_ptr<LSEInputData> lseInputData{};
    std::weak_ptr<LSESolveData> lseSolveData{};
};

struct ComboBoxMethodRecord
{
    LSESolvingMethodIndex SolvingMethodIndex;
    std::string MethodName{"[не вказано]"};
};

class LSESolverUI : public Gtk::Frame
{
public:
    LSESolverUI();

    void SetLSEInputData(std::weak_ptr<LSEInputData> lseInputData)
    {
        this->lseInputData = lseInputData;
    }
    void SetLSESolvesPlace(std::weak_ptr<LSESolveData> lseSolvesPlace)
    {
        this->lseSolvesPlace = lseSolvesPlace;
    }

    void SetLSESolveOutputUIRef(std::weak_ptr<LSESolveOutput> lseSolveOutput)
    {
        this->lseSolveOutput = lseSolveOutput;
    }

private:
    std::vector<ComboBoxMethodRecord> comboBoxMethodRecords =
    {
          {LSESolvingMethodIndex::LUP            , "LUP-метод"}
        , {LSESolvingMethodIndex::GaussHoletskiy , "метод Гауса-Холецького"}
        , {LSESolvingMethodIndex::Rotation       , "метод обертання"}
    };

    Gtk::Box solverRootBox{};

    Gtk::Label solverInstruction{"Виберіть один з методів:"};
    Gtk::ComboBoxText comboBoxMethodsNames{};
    Gtk::Button solveButton{"Вирішити"};
    Gtk::Label solvingStatus{"Невідомий статус вирішення"};
    Gtk::Label practicalTimeComplexity{"Практ. час. складн. невідома"};

    void onSolvingProcess();

    std::weak_ptr<LSEInputData> lseInputData;
    std::weak_ptr<LSESolveData> lseSolvesPlace;
    std::weak_ptr<LSESolveOutput> lseSolveOutput;
};

class ApplicationWindow : public Gtk::Window
{
public:
    ApplicationWindow();

    void SetApplicationData(std::weak_ptr<ApplicationData> appData);

    void ReadyWindow();

private:
    CLASS_CONST auto windowTitle = "Розв'язальник СЛАР";
    CLASS_CONST auto windowSize = std::pair(1000, 600);
    CLASS_CONST auto windowBorderWidth = 10;

    Gtk::Fixed fixedLayout;

    LSEConfigurator lseConfigurator;
    LSESolverUI     lseSolver;
    std::shared_ptr<LSESolveOutput> lseSolveOutput = std::make_shared<LSESolveOutput>();

    void initializeWindowHead();
    void initializeWidgets();
};

// ================================
// -------------------------------- APPLICATION GUI IMPLEMENTATION --------------------------------
// ================================

LSESolveOutput::LSESolveOutput()
{
    set_label("Виведення рішень СЛАР");
    set_size_request(320, 240);

    add(boxLayout);

    boxLayout.set_border_width(10);

    boxLayout.set_orientation(Gtk::ORIENTATION_VERTICAL);
    boxLayout.set_spacing(10);

    boxLayout.pack_start(outputStatus);
    boxLayout.pack_start(outputFileName);
    boxLayout.pack_start(outputButton);
    boxLayout.pack_start(varsDesc);
    boxLayout.pack_start(graphicOutput);

    graphicOutput.set_size_request(300, 300);

    outputFileName.set_placeholder_text("Файл для виведення");

    outputButton.signal_clicked().connect(
        sigc::mem_fun(
              *this
            , &LSESolveOutput::saveSolve
        )
    );

    boxLayout.show_all_children();
    boxLayout.show();
}

void WriteToFile(const std::string& fileName, const std::string& content)
{
    std::ofstream file(fileName);

    if (file.is_open())
    {
        file << content;
        file.close();
    }
    else
    {
        // TODO: implement...
    }
}

void LSESolveOutput::saveSolve()
{
    auto& lseSolveDataV = *lseSolveData.lock();

    if (lseSolveDataV.SolvingStatus != LSESolvingStatus::SolvedSuccessfuly)
    {
        return;
    }

    std::string formattedSolves = "";

    auto& X = lseSolveDataV.X;

    for (std::size_t i = 0; i < X.Size(); i++)
    {
        if (i >= 1)
        {
            formattedSolves += " ";
        }
        formattedSolves += std::to_string(X[i]);
    }

    WriteToFile(outputFileName.get_text(), formattedSolves);
}

void LSESolveOutput::UpdateVarsValues()
{
    auto graphicOutputP = &graphicOutput;
    auto lseInputDataD = lseInputData;
    auto lseSolveDataD = lseSolveData;

    if (graphicOutput.isDrawingConnected())
    {
        graphicOutput.signal_draw().disconnect();
    }

    graphicOutput.signal_draw().connect([graphicOutputP, lseInputDataD, lseSolveDataD](const Cairo::RefPtr<Cairo::Context>& cr)
    {
        Gtk::Allocation allocation = (*graphicOutputP).get_allocation();

        const int width = allocation.get_width();
        const int height = allocation.get_height();

        double center_x = width / 2.0;
        double center_y = height / 2.0;

        // Draw X and Y axes
        cr->set_line_width(2.0);
        cr->set_source_rgb(0.0, 1.0, 0.0);
        cr->move_to(0.0, center_y); // Move to the center of Y axis
        cr->line_to(width, center_y); // Draw X axis
        cr->move_to(center_x, 0.0); // Move to the center of X axis
        cr->line_to(center_x, height); // Draw Y axis
        cr->stroke();

        auto& out = *lseSolveDataD.lock();
        auto& in = *lseInputDataD.lock();

        auto A1 = in.A.at(0, 0);
        auto B1 = in.A.at(0, 1);
        auto C1 = in.B[0];

        auto A2 = in.A.at(1, 0);
        auto B2 = in.A.at(1, 1);
        auto C2 = in.B[1];

        auto X = out.X[0];
        auto Y = out.X[1];

        double m1 = -A1 / B1;
        double b1 = C1 / B1;

        double m2 = -A2 / B2;
        double b2 = C2 / B2;

        double max_y = std::max(
              std::max(std::fabs(-center_x * m1 + b1), std::fabs(center_x * m1 + b1))
            , std::max(std::fabs(-center_x * m1 + b2), std::fabs(center_x * m2 + b2))
        );

        double koeff = std::min(width, height) / std::max(std::fabs(X), std::fabs(Y)) / 4;

        // Draw the first line
        cr->set_source_rgb(1.0, 0.0, 0.0);
        cr->move_to(center_x - center_x * koeff, center_y - (-center_x * m1 + b1) * koeff);
        cr->line_to(center_x + center_x * koeff, center_y - (center_x * m1 + b1) * koeff);
        cr->stroke();

        // Draw the second line
        cr->set_source_rgb(0.0, 0.0, 1.0);
        cr->move_to(center_x - center_x * koeff, center_y - (-center_x * m2 + b2) * koeff);
        cr->line_to(center_x + center_x * koeff, center_y - (center_x * m2 + b2) * koeff);
        cr->stroke();

        // Draw X and Y axises text.
        cr->set_font_size(18);
        cr->move_to(center_x + 16, 18);

        cr->set_source_rgb(1, 1, 1);
        cr->show_text("X");

        cr->move_to(width - 18, center_y + 32);
        cr->show_text("Y");

        // Draw coordinate
        cr->set_font_size(12);
        cr->move_to(center_x - X * koeff, center_y - Y * koeff);
        cr->show_text("(x=" + std::to_string(X) + " y=" + std::to_string(Y) + ")");

        return true;
    });

    graphicOutput.queue_draw();
}

std::optional<std::pair<double, double>> IntersectionOfLines(
    double a1, double b1, double c1,
    double a2, double b2, double c2
)
{
    double determinant = a1 * b2 - a2 * b1;

    if (determinant == 0)
    {
        return std::nullopt;
    }

    double x = (c1 * b2 - c2 * b1) / determinant;
    double y = (a1 * c2 - a2 * c1) / determinant;

    return std::pair{x, y};
}

void LSESolveOutput::ClearVarsValues()
{
    auto graphicOutputP = &graphicOutput;

    if (graphicOutput.isDrawingConnected())
    {
        graphicOutput.signal_draw().disconnect();
    }

    graphicOutput.signal_draw().connect([graphicOutputP](const Cairo::RefPtr<Cairo::Context>& cr)
    {
        Gtk::Allocation allocation = (*graphicOutputP).get_allocation();

        const int width = allocation.get_width();
        const int height = allocation.get_height();

        cr->set_source_rgb(0, 0, 0);
        cr->rectangle(0, 0, width, height);
        cr->fill();

        return true;
    });

    graphicOutput.queue_draw();
}

std::string FormatExecTime(double x)
{
    struct functions
    {
        static std::ptrdiff_t toInt(double x)
        {
            return (std::ptrdiff_t)(x);
        }
    };

    constexpr double b = 10;

    if (x < 0)
    {
        return "(negative value)";
    }
    
    if (x >= 10)
    {
        return std::to_string(functions::toInt(x)) + " с";
    }
    if (x >= (b / 1'000))
    {
        return std::to_string(functions::toInt(x * 1'000)) + " мс";
    }
    if (x >= (b / 1'000'000))
    {
        return std::to_string(functions::toInt(x * 1'000'000)) + " μс";
    }
    if (x >= (b / 1'000'000'000))
    {
        return std::to_string(functions::toInt(x * 1'000'000'000)) + " нс";
    }
    return std::to_string(functions::toInt(x * 1'000'000'000'000)) + " пс";
}

void ApplicationWindow::initializeWindowHead()
{
    set_title(windowTitle);

    set_default_size(windowSize.first, windowSize.second);
    set_resizable(false);

    set_border_width(windowBorderWidth);
}

void ApplicationWindow::SetApplicationData(std::weak_ptr<ApplicationData> appData)
{
    lseConfigurator.SetLSEInputData(appData.lock()->GetLSEInputDataRef());

    lseSolver.SetLSEInputData(appData.lock()->GetLSEInputDataRef());
    lseSolver.SetLSESolvesPlace(appData.lock()->GetLSEOutputDataRef());

    lseSolver.SetLSESolveOutputUIRef(lseSolveOutput);

    lseSolveOutput->SetLSEOutputDataRef(appData.lock()->GetLSEOutputDataRef());
    lseSolveOutput->SetLSEInputDataRef(appData.lock()->GetLSEInputDataRef());
}

void ApplicationWindow::initializeWidgets()
{
    set_sensitive(false);

    add(fixedLayout);
    fixedLayout.show();
    fixedLayout.put(lseConfigurator, 0, 0);
    fixedLayout.put(lseSolver, 650, 0);
    fixedLayout.put(*lseSolveOutput, 650, 310);

    lseConfigurator.show();
    lseSolver.show();
    (*lseSolveOutput).show();
}

void ApplicationWindow::ReadyWindow()
{
    set_sensitive(true);
}

ApplicationWindow::ApplicationWindow()
{
    initializeWindowHead();

    initializeWidgets();
}

LSEConfigurator::LSEConfigurator()
{
    set_label("Конфігурація СЛАР");
    set_size_request(640, -1);

    add(boxLayout);

    boxLayout.set_orientation(Gtk::ORIENTATION_VERTICAL);
    boxLayout.set_border_width(10);

    initializeEqsCount();
    initializeEqsForm();

    boxLayout.pack_start(eqsPropConfGrid);
    boxLayout.pack_start(eqsFormBox);

    boxLayout.show_all_children();
    boxLayout.show();
}

void LSEConfigurator::initializeEqsCount()
{
    eqsPropConfGrid.attach(eqsConfStatus, 0, 0, 3, 1);

    eqsPropConfGrid.attach(eqsCountProp, 0, 1);
    eqsPropConfGrid.attach(eqsCountValue, 2, 1);

    eqsCountFirstHorAlign.set_margin_right(18);
    eqsPropConfGrid.attach(eqsCountFirstHorAlign, 1, 1, 1, 2);

    eqsPropConfGrid.attach(eqsSetterProp, 0, 2);
    eqsPropConfGrid.attach(eqsSetterEntry, 2, 2);
    eqsPropConfGrid.attach(eqsSetterButton, 4, 2);

    eqsPropConfGrid.attach(eqsZeroFillerButton, 0, 3);
    eqsPropConfGrid.attach(eqsSetAsInput, 1, 3, 2, 1);

    eqsSetterEntry.set_placeholder_text("1..10");

    eqsCountProp.set_width_chars(18);
    eqsSetterProp.set_width_chars(18);

    eqsSetterButton.signal_clicked().connect(
        sigc::mem_fun(
              *this
            , &LSEConfigurator::onEqsCountSetting
        )
    );

    eqsZeroFillerButton.signal_clicked().connect(
        sigc::mem_fun(
              *this
            , &LSEConfigurator::fillEmptyEntriesWithZeroes
        )
    );

    eqsSetAsInput.signal_clicked().connect(
        sigc::mem_fun(
              *this
            , &LSEConfigurator::setEqsAsInput
        )
    );

    eqsPropConfGrid.show_all_children();
}

std::string GetCoeffAShortLabel(double y, double x)
{
    return std::format("a{},{}", y + 1, x + 1);
}

std::string GetCoeffAFancyLabel(double y, double x)
{
    return std::format("A[змінна={} рівн.={}]", x + 1, y + 1);
}

std::string GetCoeffBShortLabel(double eqIndex)
{
    return std::format("b{}", eqIndex + 1);
}

std::string GetCoeffBFancyLabel(double eqIndex)
{
    return std::format("B[рівн.={}]", eqIndex + 1);
}

std::optional<double> ToNumber(const std::string& mayFormattedNumber)
{
    try
    {
        return std::stod(mayFormattedNumber);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

void LSEConfigurator::setEqsAsInput()
{
    auto lseInputData = lseData.lock();
    auto eqsCount = lseInputData->eqsCount;

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto mayCoeffA = ToNumber(varsCoeffsEntries.at(y, x).get_text());

            if (! mayCoeffA.has_value())
            {
                eqsConfStatus.set_text(
                    std::format("Комірка {} не є числом", GetCoeffAFancyLabel(y, x))
                );
                return;
            }

            lseInputData->A.at(y, x) = mayCoeffA.value();
        }

        auto mayCoeffB = ToNumber(freeCoeffsEntries[y].get_text());

        if (! mayCoeffB.has_value())
        {
            eqsConfStatus.set_text(
                std::format("Комірка {} не є числом", GetCoeffBFancyLabel(y))
            );
            return;
        }

        lseInputData->B[y] = mayCoeffB.value();
    }

    eqsConfStatus.set_text(
        std::format("Дана СЛАР встановлена {}", GetCurrentFormalTime())
    );

    lseInputData->IsSetted = true;
}

void LSEConfigurator::initializeEqsForm()
{
    eqsFormBox.pack_start(varsCoeffsGrid, Gtk::PACK_SHRINK);
    eqsFormBox.pack_start(freeCoeffsGrid, Gtk::PACK_SHRINK, 12);

    eqsFormBox.show_all_children();
}

void LSEConfigurator::onEqsCountSetting()
{
    auto mayEqsCount = ToInteger(
        eqsSetterEntry.get_text()
    );

    eqsSetterEntry.set_text("");

    if (! mayEqsCount.has_value())
    {
        eqsConfStatus.set_text("Ввід не є додатнім числом");
        return;
    }

    auto eqsCount = mayEqsCount.value();

    if (! (1 <= eqsCount && eqsCount <= 10))
    {
        eqsConfStatus.set_text("Кількість рівнянь не є в проміжку [1; 10]");
        return;
    }

    eqsConfStatus.set_text("Встановлено кількість рівнянь");
    eqsCountValue.set_text(std::to_string(eqsCount));

    removeEqsForm();
    createEqsForm(eqsCount);
}

void LSEConfigurator::createEqsForm(std::size_t eqsCount)
{
    varsCoeffsEntries = AllocArray2D<Gtk::Entry>(eqsCount, eqsCount);
    freeCoeffsEntries = std::vector<Gtk::Entry>(eqsCount);

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto& newVarCoeff = varsCoeffsEntries.at(y, x);
            
            newVarCoeff = Gtk::Entry();
            newVarCoeff.set_width_chars(3);

            newVarCoeff.set_placeholder_text(
                GetCoeffAShortLabel(y, x)
            );
            newVarCoeff.set_tooltip_text(
                GetCoeffAFancyLabel(y, x)
            );

            varsCoeffsGrid.attach(newVarCoeff, x, y);
        }

        auto& newFreeCoeff = freeCoeffsEntries[y];

        newFreeCoeff = Gtk::Entry();
        newFreeCoeff.set_width_chars(5);

        newFreeCoeff.set_placeholder_text(
            GetCoeffBShortLabel(y)
        );
        newFreeCoeff.set_tooltip_text(
            GetCoeffBFancyLabel(y)
        );

        freeCoeffsGrid.attach(newFreeCoeff, 0, y);
    }

    auto lseDataP = lseData.lock();

    lseDataP->eqsCount = eqsCount;
    lseDataP->A = Matrix(eqsCount, eqsCount);
    lseDataP->B = Vector(eqsCount);
    lseDataP->IsSetted = false;

    varsCoeffsGrid.show_all_children();
    freeCoeffsGrid.show_all_children();

    varsCoeffsGrid.show();
    freeCoeffsGrid.show();
}

void LSEConfigurator::removeEqsForm()
{
    std::size_t eqsCount = lseData.lock()->eqsCount;

    varsCoeffsGrid.hide();
    freeCoeffsGrid.hide();

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        varsCoeffsGrid.remove_column(0);
        varsCoeffsGrid.remove_row(0);

        freeCoeffsGrid.remove_column(0);
    }

    auto lseDataP = lseData.lock();

    lseDataP->eqsCount = 0;
    lseDataP->A = Matrix();
    lseDataP->B = Vector();
    lseDataP->IsSetted = false;

    varsCoeffsEntries = AllocArray2D<Gtk::Entry>();
    freeCoeffsEntries = std::vector<Gtk::Entry>();
}

void LSEConfigurator::fillEmptyEntriesWithZeroes()
{
    auto eqsCount = lseData.lock()->eqsCount;

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto& currentEntryOfMatrixA = varsCoeffsEntries.at(y, x);
            
            const auto& origEntryInput = currentEntryOfMatrixA.get_text();

            if (origEntryInput == "")
            {
                currentEntryOfMatrixA.set_text("0");
            }
        }

        auto& currentEntryOfVectorB = freeCoeffsEntries[y];

        const auto& origEntryInput = currentEntryOfVectorB.get_text();

        if (origEntryInput == "")
        {
            currentEntryOfVectorB.set_text("0");
        }
    }
}

LSESolverUI::LSESolverUI()
{
    set_label("Розв'язання СЛАР");
    set_size_request(340, 300);

    add(solverRootBox);

    solverRootBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    solverRootBox.set_border_width(10);

    solverRootBox.set_spacing(10);

    solverRootBox.pack_start(solverInstruction);
    solverRootBox.pack_start(comboBoxMethodsNames);
    solverRootBox.pack_start(solveButton);
    solverRootBox.pack_start(solvingStatus);
    solverRootBox.pack_start(practicalTimeComplexity);

    solveButton.signal_clicked().connect(
        sigc::mem_fun(
              *this
            , &LSESolverUI::onSolvingProcess
        )
    );

    for (std::size_t methodIndex = 0; methodIndex < comboBoxMethodRecords.size(); methodIndex++)
    {
        comboBoxMethodsNames.append(
              std::to_string(methodIndex)
            , comboBoxMethodRecords[methodIndex].MethodName
        );
    }

    solverRootBox.show_all_children();
    solverRootBox.show();
}

class TimeDifferenceRuler
{
public:
    TimeDifferenceRuler() = default;

    void SetBeginTimePoint()
    {
        beginTimePoint = std::chrono::high_resolution_clock::now();
    }
    void SetEndTimePoint()
    {
        endTimePoint = std::chrono::high_resolution_clock::now();
    }
    double GetTimeDifference()
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(endTimePoint - beginTimePoint).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> beginTimePoint{};
    std::chrono::time_point<std::chrono::high_resolution_clock> endTimePoint{};
};

void LSESolverUI::onSolvingProcess()
{
    auto comboBoxMethodIndex = comboBoxMethodsNames.get_active_id();

    if (comboBoxMethodIndex == "")
    {
        solvingStatus.set_text("Метод не встановлено");
        return;
    }

    auto lseInput = lseInputData.lock();

    if (! lseInput->IsSetted)
    {
        solvingStatus.set_text("СЛАР ще не встановлена");
        return;
    }

    auto eqsCount = lseInput->eqsCount;
    auto& A = lseInput->A;
    auto& B = lseInput->B;

    auto solvingMethodP = LSESolverFactoryProduce(
        comboBoxMethodRecords[
           std::stoi(comboBoxMethodIndex)
        ].SolvingMethodIndex
    );

    auto& solvingMethod = *solvingMethodP;

    solvingMethod.SetEquationsCount(eqsCount);
    solvingMethod.SetVariablesCoefficients(A);
    solvingMethod.SetFreeCoefficients(B);

    TimeDifferenceRuler timeRuler{};

    timeRuler.SetBeginTimePoint();

    solvingMethod.SolveLSE();

    timeRuler.SetEndTimePoint();

    auto& lseSolvesPlaceV = *lseSolvesPlace.lock();

    if (! solvingMethod.IsLSESoledSuccessfully().value())
    {
        lseSolvesPlaceV.SolvingStatus = LSESolvingStatus::SolvedFailful;
        solvingStatus.set_text("СЛАР не можливо вирішити");
        return;
    }

    auto X = solvingMethod.GetVariablesValuesUnsafely();

    lseSolvesPlaceV.X = std::move(X);
    lseSolvesPlaceV.SolvingStatus = LSESolvingStatus::SolvedSuccessfuly;

    lseInput->IsSetted = false;

    solvingStatus.set_text(
        std::format("СЛАР вирішено {}", GetCurrentFormalTime())
    );
    practicalTimeComplexity.set_text(
        std::format("Практ. час. складн. {}", FormatExecTime(
            timeRuler.GetTimeDifference()
        ))
    );

    lseSolveOutput.lock()->OutputSolves();
}

// ================================
// -------------------------------- ENTRY POINT --------------------------------
// ================================

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "ua.kpi.ip-35.course1-term2.lse-solver");

    ApplicationWindow appWin{};

    auto appData = std::make_shared<ApplicationData>();

    appWin.SetApplicationData(appData);
    appWin.ReadyWindow();

    return app->run(appWin);
}
