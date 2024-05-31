#pragma once

#include "Containers/AllocArray2D.inc.hpp"
#include "Containers/Matrix.hpp"
#include "Containers/Vector.hpp"

#include <gtkmm.h>

#include <cstdint>

#include <memory>

class SLEInputData
{
public:
    SLEInputData();

    void ClearData();

    void SetEquationsCount(std::size_t eqsCount);
    std::optional<std::size_t> GetEquationsCount() const noexcept;
    bool IsEquationsCountSetted() const noexcept;

    void SetVariablesCoefficients(auto&& a)
    {
        if (! isEqsCountSetted)
        {
            return;
        }
        if (! (a.TryGetEdgeSize() == eqsCount))
        {
            return;
        }

        A = a;
    }
    void SetFreeCoefficients(auto&& b)
    {
        if (! isEqsCountSetted)
        {
            return;
        }
        if (! (b.Size() == eqsCount))
        {
            return;
        }

        B = std::forward<decltype(b)>(b);
    }

    std::optional<std::reference_wrapper<Matrix>> GetVariablesCoefficientsRef();
    std::optional<std::reference_wrapper<const Matrix>> GetVariablesCoefficientsRef() const;

    std::optional<std::reference_wrapper<Vector>> GetFreeCoefficientsRef();
    std::optional<std::reference_wrapper<const Vector>> GetFreeCoefficientsRef() const;

    void ConfirmData();
    bool IsDataConfirmed() const noexcept;

private:
    std::size_t eqsCount = 0;

    Matrix A{};
    Vector B{};

    bool isConfirmed = false;

    bool isEqsCountSetted = false;
};

enum class SLESolvingStatus : uint8_t
{
    Unsolved, SolvedSuccessfully, SolvedFailful
};

class SLESolveData
{
public:
    SLESolveData();

    void SetVarsSolve(auto&& x)
    {
        X = std::forward<decltype(x)>(x);
    }
    std::optional<std::reference_wrapper<Vector>> GetSolveRef();
    std::optional<std::reference_wrapper<const Vector>> GetSolveRef() const;

    void SetSolvingStatus(SLESolvingStatus sleSolvingStatus);
    SLESolvingStatus GetSolvingStatus() const noexcept;

private:
    Vector X{};
    SLESolvingStatus solvingStatus = SLESolvingStatus::Unsolved;
};

class ApplicationData
{
public:
    ApplicationData() = default;

    std::weak_ptr<SLEInputData> GetSLEInputData()
    {
        return std::weak_ptr<SLEInputData>(sleInputData);
    }
    std::weak_ptr<SLESolveData> GetSLEOutputData()
    {
        return std::weak_ptr<SLESolveData>(sleSolveData);
    }

private:
    std::shared_ptr<SLEInputData> sleInputData
        = std::make_shared<SLEInputData>();

    std::shared_ptr<SLESolveData> sleSolveData
        = std::make_shared<SLESolveData>();
};

class SLEConfigurator : public Gtk::Frame
{
public:
    SLEConfigurator();

    void SetSLEInputData(std::weak_ptr<SLEInputData> sleInputData);

private:
    Gtk::Box boxLayout{};

    Gtk::Grid eqsPropConfGrid{};

    Gtk::Alignment eqsCountHorPadding1{};
    Gtk::Alignment eqsCountHorPadding2{};

    Gtk::Alignment eqsCountVerPadding1{};
    Gtk::Alignment eqsCountVerPadding2{};
    Gtk::Alignment eqsCountVerPadding3{};
    Gtk::Alignment eqsCountVerPadding4{};
    Gtk::Alignment eqsCountVerPadding5{};

    Gtk::Label eqsConfStatus{"Невідомий статус"};

    Gtk::Label eqsCountProp{"Кількість рівнянь:"};
    Gtk::Label eqsCountValue{"не вказано"};

    Gtk::Label  eqsSetterProp{"Встановити кількість:"};
    Gtk::Entry  eqsSetterEntry{};
    Gtk::Button eqsSetterButton{"Нова СЛАР"};

    Gtk::Button eqsZeroFillerButton{"Заповнити пусті кліт. 0-ми"};
    Gtk::Button eqsSetAsInput{"Встановити дану СЛАР"};

    Gtk::Label sleFormLabel{"Форма введення СЛАР:"};

    void setEqsAsInput();

    void initializeEqsCount();

    void onEqsCountSetting();
    void createSLEForm(std::size_t eqsCount);
    void removeSLEForm();

    void fillEmptyEntriesWithZeroes();

    Gtk::ScrolledWindow unlimitedSLEFormScroller{};

    Gtk::Box eqsFormBox{};

    Gtk::Grid varsCoeffsGrid{};
    Gtk::Grid eqMarksGrid{};
    Gtk::Grid freeCoeffsGrid{};

    RTArray2D<Gtk::Entry> varsCoeffsEntries{};
    RTArray2D<Gtk::Label> varsCoeffsLabels{};

    std::vector<Gtk::Label> leEquationMarkLabels{};

    std::vector<Gtk::Entry>  freeCoeffsEntries{};

    void initializeEqsForm();

    std::weak_ptr<SLEInputData> sleData{};
};

class SLESolveShower : public Gtk::Frame
{
public:
    SLESolveShower();

    void SetSLEInputData(std::weak_ptr<SLEInputData> sleInputData);
    void SetSLESolveData(std::weak_ptr<SLESolveData> sleSolveData);

    void OutputSolve();
    void ClearSolve();

private:
    Gtk::Box boxLayout{};

    Gtk::Label outputStatus{"Невідомий статус"};

    Gtk::Label outputSolveLabel{"Вивести в файл:"};
    Gtk::Entry outputFileName{};
    Gtk::Button outputButton{"Спробувати записати"};

    Gtk::Label varsDesc{"Рішення СЛАР:"};
    Gtk::Label varsValues{"(ще немає)"};

    Gtk::Label outputGraphLabel{"Графічне вирішення:"};
    Gtk::DrawingArea outputGraph{};

    bool doRenderGraph = false;

    void saveSolve();

    std::weak_ptr<SLEInputData> sleInputData{};
    std::weak_ptr<SLESolveData> sleSolveData{};
};

class SLESolvePanel : public Gtk::Frame
{
public:
    SLESolvePanel();

    void SetSLEInputData(std::weak_ptr<SLEInputData> sleInputData);
    void SetSLESolveData(std::weak_ptr<SLESolveData> sleSolveData);

    void SetSLESolveOutput(std::weak_ptr<SLESolveShower> sleSolveOutput);

private:
    Gtk::Box solverRootBox{};

    Gtk::Label solverInstruction{"Виберіть один з методів:"};
    Gtk::ComboBoxText comboBoxMethodsNames{};
    Gtk::Button solveButton{"Вирішити"};
    Gtk::Label solvingStatus{"Невідомий статус вирішення"};
    Gtk::Label practicalTimeComplexity{"Практ. час. складн. невідома"};

    void onSolvingProcess();

    std::weak_ptr<SLEInputData> sleInputData;
    std::weak_ptr<SLESolveData> sleSolveData;
    std::weak_ptr<SLESolveShower> sleSolveOutput;
};

class ApplicationWindow : public Gtk::Window
{
public:
    ApplicationWindow();

    void SetApplicationData(std::weak_ptr<ApplicationData> appData);

    void Ready();

private:
    Gtk::Fixed fixedLayout;

    SLEConfigurator sleConfigurator;
    SLESolvePanel     sleSolver;
    std::shared_ptr<SLESolveShower> sleSolveOutput = std::make_shared<SLESolveShower>();

    void initializeWindowHead();
    void initializeWidgets();
};

class GUISession
{
public:
    GUISession();

    void Init();
    ApplicationWindow& GetWindowRef();

private:
    ApplicationWindow appWin{};
    std::shared_ptr<ApplicationData> appData = std::make_shared<ApplicationData>();
};
