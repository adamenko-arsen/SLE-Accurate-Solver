#include "GUI.hpp"

#include "Convert.hpp"
#include "Filesystem.hpp"

#include <cmath>

#include <format>
#include <fstream>
#include <string>
#include <utility>

#include "LSESolver.hpp"
#include "LSESolversData.hpp"

#include <vector>

std::string GetCurrentFormalTime()
{
    auto t = time(NULL);

    return std::format(
          "{}:{}:{} UTC+00:00"
        , t / 3600 % 24
        , t / 60 % 60
        , t % 60
    );
}

double PutInRange(double x, double f, double t)
{
    if (! (f <= x))
    {
        return f;
    }
    if (! (x <= t))
    {
        return t;
    }
    return x;
}

void LSEConfigurator::SetLSEInputData(std::weak_ptr<LSEInputData> lseInputData)
{
    this->lseData = lseInputData;
}

double Logarithm(double powered, double base)
{
    return std::log(powered) / std::log(base);
}

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
    boxLayout.pack_start(outputGraph);
    boxLayout.pack_start(varsValues);

    outputFileName.set_placeholder_text("Файл для виведення");

    outputButton.signal_clicked().connect(
        sigc::mem_fun(
              *this
            , &LSESolveOutput::saveSolve
        )
    );

    outputGraph.set_size_request(300, 300);
    outputGraph.signal_draw().connect([this_ = this](const Cairo::RefPtr<Cairo::Context>& cr)
    {
        auto& outputGraph = this_->outputGraph;

        Gtk::Allocation allocation = outputGraph.get_allocation();

        const std::ptrdiff_t width = allocation.get_width();
        const std::ptrdiff_t height = allocation.get_height();

        cr->set_source_rgb(0, 0, 0);
        cr->rectangle(0, 0, width, height);
        cr->fill();

        if (! this_->doRenderGraph)
        {
            return true;
        }

        auto& lseInputData = *this_->lseInputData.lock();
        auto& lseSolveData = *this_->lseSolveData.lock();

        auto A1 = PutInRange(lseInputData.A.At(0, 0), 0.01, 100);
        auto B1 = PutInRange(lseInputData.A.At(0, 1), 0.01, 100);
        auto C1 = lseInputData.B[0];

        auto A2 = PutInRange(lseInputData.A.At(1, 0), 0.01, 100);
        auto B2 = PutInRange(lseInputData.A.At(1, 1), 0.01, 100);
        auto C2 = lseInputData.B[1];

        auto X = lseSolveData.X[0];
        auto Y = lseSolveData.X[1];

        if (! (lseInputData.eqsCount == 2))
        {
            return true;
        }

        double center_x = width / 2.0;
        double center_y = height / 2.0;

        double maxSolvedVarValue = std::max(
            std::fabs(X), std::fabs(Y)
        );

        double koeff = (std::min(width, height) / 2) / maxSolvedVarValue / 2;

        // Draw major and minor lines:
        double Cp = std::floor(Logarithm(maxSolvedVarValue, 10));
        double Dp = std::floor(Logarithm(10 * maxSolvedVarValue, 10));

        double C = std::pow(10, Cp);
        double D = std::pow(10, Dp);

        double MinStep = C * koeff;
        double MajStep = D * koeff;

        double MinXL = std::ceil((height / 2) / MinStep) * MinStep;
        double MinYL = std::ceil((width / 2) / MinStep) * MinStep;

        double MajXL = std::ceil((height / 2) / MajStep) * MajStep;
        double MajYL = std::ceil((width / 2) / MajStep) * MajStep;

        cr->set_source_rgb(0.25, 0.25, 0.25);
        for (std::ptrdiff_t i = -MinXL; i <= MinXL; i++)
        {
            cr->move_to(width / 2 + MinStep * i, 0);
            cr->line_to(width / 2 + MinStep * i, height);
            cr->stroke();
        }

        for (std::ptrdiff_t i = -MinYL; i <= MinYL; i++)
        {
            cr->move_to(0,     height / 2 + MinStep * i);
            cr->line_to(width, height / 2 + MinStep * i);
            cr->stroke();
        }

        cr->set_source_rgb(0.6, 0.6, 0.6);
        for (std::ptrdiff_t i = -MajXL; i <= MajXL; i++)
        {
            cr->move_to(width / 2 + MajStep * i, 0);
            cr->line_to(width / 2 + MajStep * i, height);
            cr->stroke();
        }

        for (std::ptrdiff_t i = -MajYL; i <= MajYL; i++)
        {
            cr->move_to(0,     height / 2 + MajStep * i);
            cr->line_to(width, height / 2 + MajStep * i);
            cr->stroke();
        }

        // Draw X and Y axes
        cr->set_line_width(2.0);
        cr->set_source_rgb(0.0, 1.0, 0.0);

        cr->move_to(0.0, center_y); // Move to the center of Y axis
        cr->line_to(width, center_y); // Draw X axis

        cr->move_to(center_x, 0.0); // Move to the center of X axis
        cr->line_to(center_x, height); // Draw Y axis
        cr->stroke();

        double m1 = -A1 / B1;
        double b1 = C1 / B1;

        double m2 = -A2 / B2;
        double b2 = C2 / B2;

        // Draw the first line
        cr->set_source_rgb(1.0, 0.0, 0.0);
        cr->move_to(0,     center_y - (-center_x * m1 + b1 * koeff));
        cr->line_to(width, center_y - (center_x * m1 + b1 * koeff));
        cr->stroke();

        // Draw the second line
        cr->set_source_rgb(0.0, 0.0, 1.0);
        cr->move_to(0,     center_y - (-center_x * m2 + b2 * koeff));
        cr->line_to(width, center_y - (center_x * m2 + b2 * koeff));
        cr->stroke();

        // Draw X and Y axises text.
        cr->set_font_size(12);
        cr->move_to(center_x + 12 / 2 * 1.5, 12);

        cr->set_source_rgb(1, 1, 1);
        cr->show_text("Y");

        cr->move_to(width - 12, center_y + 12 * 1.5);
        cr->show_text("X");

        // Draw coordinate
        cr->set_font_size(12);
        cr->move_to(center_x + X * koeff, center_y - Y * koeff);
        cr->show_text("(x=" + std::to_string(X) + " y=" + std::to_string(Y) + ")");

        // Draw minor and major lines scale
        cr->set_font_size(12);
        cr->move_to(18, 18);
        cr->show_text(std::format("Масштаб ліній відліку змінних: 10^{} = {}", Cp, std::pow(10, Cp)));

        return true;
    });

    boxLayout.show_all_children();
    boxLayout.show();
}

void LSESolveOutput::OutputSolve()
{
    outputStatus.set_text(std::format("Рішення виведено {}", GetCurrentFormalTime()));

    std::string varsValuesStr{};

    auto& lseSolveDataV = *lseSolveData.lock();

    for (std::size_t i = 0; i < lseSolveDataV.X.Size(); i++)
    {
        if (i != 0)
        {
            varsValuesStr += "\n";
        }
        varsValuesStr += std::format("X{}: {}", i + 1, lseSolveDataV.X[i]);
    }

    varsValues.set_text(varsValuesStr);

    doRenderGraph = true;
    outputGraph.queue_draw();
}

void LSESolveOutput::ClearSolve()
{
    outputStatus.set_text(std::format("Рішення невиведено {}", GetCurrentFormalTime()));
    varsValues.set_text("");

    doRenderGraph = false;
    outputGraph.queue_draw();
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

    lseSolveOutput->SetLSEInputDataRef(appData.lock()->GetLSEInputDataRef());
    lseSolveOutput->SetLSEOutputDataRef(appData.lock()->GetLSEOutputDataRef());
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

#include <iostream>

void LSEConfigurator::setEqsAsInput()
{
    auto lseInputData = lseData.lock();
    auto eqsCount = lseInputData->eqsCount;

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto mayCoeffA = ToNumber(varsCoeffsEntries.At(y, x).get_text());

            if (! mayCoeffA.has_value())
            {
                eqsConfStatus.set_text(
                    std::format("Комірка {} не є числом", GetCoeffAFancyLabel(y, x))
                );
                return;
            }

            auto coeffA = mayCoeffA.value();

            if (! (-1'000 <= coeffA && coeffA <= 1'000))
            {
                eqsConfStatus.set_text(
                    std::format("Комірка {} не є в діапазоні [-1'000; 1'000]", GetCoeffAFancyLabel(y, x))
                );
                return;
            }

            lseInputData->A.At(y, x) = coeffA;
        }

        auto mayCoeffB = ToNumber(freeCoeffsEntries[y].get_text());

        if (! mayCoeffB.has_value())
        {
            eqsConfStatus.set_text(
                std::format("Комірка {} не є числом", GetCoeffBFancyLabel(y))
            );
            return;
        }

        auto coeffB = mayCoeffB.value();

        if (! (-10'000 <= coeffB && coeffB <= 10'000))
        {
            eqsConfStatus.set_text(
                std::format("Комірка {} не є в діапазоні [-10'000; 10'000]", GetCoeffBFancyLabel(y))
            );
            return;
        }

        lseInputData->B[y] = coeffB;
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
            auto& newVarCoeff = varsCoeffsEntries.At(y, x);
            
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
            auto& currentEntryOfMatrixA = varsCoeffsEntries.At(y, x);
            
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
        auto currentOption = comboBoxMethodRecords[methodIndex];
        comboBoxMethodsNames.append(
              std::to_string(methodIndex)
            , std::format("{}\nСкладність:{}", currentOption.MethodName, currentOption.MethodPracticalItersComplexity)
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
           ToInteger(comboBoxMethodIndex).value()
        ].SolvingMethodIndex
    );

    auto& solvingMethod = *solvingMethodP;

    solvingMethod.SetEquationsCount(eqsCount);
    solvingMethod.SetVariablesCoefficients(A);
    solvingMethod.SetFreeCoefficients(B);

    solvingMethod.SolveLSE();

    auto& lseSolvesPlaceV = *lseSolvesPlace.lock();

    if (! solvingMethod.IsLSESoledSuccessfully().value())
    {
        lseSolveOutput.lock()->ClearSolve();

        lseSolvesPlaceV.SolvingStatus = LSESolvingStatus::SolvedFailful;
        solvingStatus.set_text("СЛАР не можливо вирішити цим методом");
        practicalTimeComplexity.set_text("");
        return;
    }

    auto X = solvingMethod.GetVariablesSolvesOnce().value();

    lseSolvesPlaceV.X = std::move(X);
    lseSolvesPlaceV.SolvingStatus = LSESolvingStatus::SolvedSuccessfuly;

    lseInput->IsSetted = false;

    solvingStatus.set_text(
        std::format("СЛАР вирішено {}", GetCurrentFormalTime())
    );
    practicalTimeComplexity.set_text(
        std::format("СЛАР вирішено за {} ітерацій", solvingMethod.GetTotalIterationsCount().value())
    );

    lseSolveOutput.lock()->OutputSolve();
}

GUISession::GUISession() = default;

void GUISession::Init(int argc, char *argv[])
{
    appWin.SetApplicationData(appData);
    appWin.ReadyWindow();
}

Gtk::Window& GUISession::GetWindow()
{
    return appWin;
}
