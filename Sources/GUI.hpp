#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"
#include "AllocArray2D.inc.hpp"

#include <gtkmm.h>

#include <cstdint>

#include <memory>

#define CLASS_CONST static constexpr

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

    void SetLSEInputData(std::weak_ptr<LSEInputData> lseInputData);

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

    void OutputSolve();
    void ClearSolve();

private:
    Gtk::Box boxLayout{};

    Gtk::Label outputStatus{"Невідомий статус"};
    Gtk::Entry outputFileName{};
    Gtk::Button outputButton{"Вивести в файл"};
    Gtk::Label varsDesc{"Рішення СЛАР:"};
    Gtk::Label varsValues{""};

    Gtk::DrawingArea outputGraph{};

    bool doRenderGraph = false;

    void saveSolve();

    std::weak_ptr<LSEInputData> lseInputData{};
    std::weak_ptr<LSESolveData> lseSolveData{};
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

class GUISession
{
public:
    GUISession();

    void Init(int argc, char *argv[]);

    Gtk::Window& GetWindow();

private:
    ApplicationWindow appWin{};
    std::shared_ptr<ApplicationData> appData = std::make_shared<ApplicationData>();
};
