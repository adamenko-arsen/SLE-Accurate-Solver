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

std::ptrdiff_t GetTimeSeconds()
{
    return time(NULL);
}

std::string GetCurrentFormalTime()
{
    auto secs = GetTimeSeconds();

    return std::format(
          "{}:{}:{} UTC+00:00"
        , secs / (60 * 60) % 24
        , secs / 60 % 60
        , secs % 60
    );
}

double PutInRange(double value, double from, double to)
{
    if (! (from <= value))
    {
        return from;
    }
    if (! (value <= to))
    {
        return to;
    }
    return value;
}

void LSEConfigurator::SetLSEInputData(std::weak_ptr<LSEInputData> lseInputData)
{
    this->lseData = lseInputData;
}

double Logarithm(double poweredValue, double base)
{
    return std::log(poweredValue) / std::log(base);
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
    outputGraph.signal_draw().connect
    (
        [this_ = this](const Cairo::RefPtr<Cairo::Context>& canvas)
        {
            auto& outputGraph = this_->outputGraph;

            Gtk::Allocation canvasAlloc = outputGraph.get_allocation();

            const std::ptrdiff_t width = canvasAlloc.get_width();
            const std::ptrdiff_t height = canvasAlloc.get_height();

            canvas->set_source_rgb(0, 0, 0);
            canvas->rectangle(0, 0, width, height);
            canvas->fill();

            if (! this_->doRenderGraph)
            {
                return true;
            }

            auto& lseInputData = *this_->lseInputData.lock();
            auto& lseSolveData = *this_->lseSolveData.lock();

            auto coeffA1 = PutInRange(lseInputData.A.At(0, 0), 0.01, 100);
            auto coeffB1 = PutInRange(lseInputData.A.At(0, 1), 0.01, 100);
            auto coeffC1 = lseInputData.B[0];

            auto coeffA2 = PutInRange(lseInputData.A.At(1, 0), 0.01, 100);
            auto coeffB2 = PutInRange(lseInputData.A.At(1, 1), 0.01, 100);
            auto coeffC2 = lseInputData.B[1];

            const auto& solves = lseSolveData.X;

            auto solveX = solves[0];
            auto solveY = solves[1];

            if (! (lseInputData.EqsCount == 2))
            {
                return true;
            }

            double centerX = width / 2.0;
            double centerY = height / 2.0;

            double maxSolvedVarValue = std::max(
                std::fabs(solveX), std::fabs(solveY)
            );

            double pixelsToValueScale = (std::min(width, height) / 2) / maxSolvedVarValue / 2;

            // Draw major and minor lines:
            double minorLinesPower = std::floor(Logarithm(maxSolvedVarValue, 10));
            double majorLinesPower = std::floor(Logarithm(10 * maxSolvedVarValue, 10));

            double minorLinesValueStep = std::pow(10, minorLinesPower);
            double majorLinesValueStep = std::pow(10, majorLinesPower);

            double minorLinesPixelsStep = minorLinesValueStep * pixelsToValueScale;
            double majorLinesPixelsStep = majorLinesValueStep * pixelsToValueScale;

            double minorLinesCountByX = std::ceil((width / 2) / minorLinesPixelsStep) * minorLinesPixelsStep;
            double minorLinesCountByY = std::ceil((height / 2) / minorLinesPixelsStep) * minorLinesPixelsStep;

            double majorLinesCountByX = std::ceil((width / 2) / majorLinesPixelsStep) * majorLinesPixelsStep;
            double majorLinesCountByY = std::ceil((height / 2) / majorLinesPixelsStep) * majorLinesPixelsStep;

            canvas->set_source_rgb(0.25, 0.25, 0.25);
            for (std::ptrdiff_t i = -minorLinesCountByX; i <= minorLinesCountByX; i++)
            {
                canvas->move_to(width / 2 + minorLinesPixelsStep * i, 0);
                canvas->line_to(width / 2 + minorLinesPixelsStep * i, height);
                canvas->stroke();
            }

            for (std::ptrdiff_t i = -minorLinesCountByY; i <= minorLinesCountByY; i++)
            {
                canvas->move_to(0,     height / 2 + minorLinesPixelsStep * i);
                canvas->line_to(width, height / 2 + minorLinesPixelsStep * i);
                canvas->stroke();
            }

            canvas->set_source_rgb(0.6, 0.6, 0.6);
            for (std::ptrdiff_t i = -majorLinesCountByX; i <= majorLinesCountByX; i++)
            {
                canvas->move_to(width / 2 + majorLinesPixelsStep * i, 0);
                canvas->line_to(width / 2 + majorLinesPixelsStep * i, height);
                canvas->stroke();
            }

            for (std::ptrdiff_t i = -majorLinesCountByY; i <= majorLinesCountByY; i++)
            {
                canvas->move_to(0,     height / 2 + majorLinesPixelsStep * i);
                canvas->line_to(width, height / 2 + majorLinesPixelsStep * i);
                canvas->stroke();
            }

            // Draw X and Y axes
            canvas->set_line_width(2.0);
            canvas->set_source_rgb(0.0, 1.0, 0.0);

            canvas->move_to(0.0, centerY);
            canvas->line_to(width, centerY);

            canvas->move_to(centerX, 0.0);
            canvas->line_to(centerX, height);
            canvas->stroke();

            double coeffK1 = -coeffA1 / coeffB1;
            double coeffBase1 = coeffC1 / coeffB1;

            double coeffK2 = -coeffA2 / coeffB2;
            double coeffBase2 = coeffC2 / coeffB2;

            // Draw the first line
            canvas->set_source_rgb(1.0, 0.0, 0.0);
            canvas->move_to(0,     centerY - (-centerX * coeffK1 + coeffBase1 * pixelsToValueScale));
            canvas->line_to(width, centerY - (centerX * coeffK1 + coeffBase1 * pixelsToValueScale));
            canvas->stroke();

            // Draw the second line
            canvas->set_source_rgb(0.0, 0.0, 1.0);
            canvas->move_to(0,     centerY - (-centerX * coeffK2 + coeffBase2 * pixelsToValueScale));
            canvas->line_to(width, centerY - (centerX * coeffK2 + coeffBase2 * pixelsToValueScale));
            canvas->stroke();

            // Draw X and Y axises text.
            canvas->set_font_size(12);
            canvas->move_to(centerX + 12 / 2 * 1.5, 12);

            canvas->set_source_rgb(1, 1, 1);
            canvas->show_text("Y (X2)");

            canvas->move_to(width - 36, centerY + 12 * 1.5);
            canvas->show_text("X (X1)");

            // Draw coordinate
            canvas->set_font_size(12);
            canvas->move_to(centerX + solveX * pixelsToValueScale, centerY - solveY * pixelsToValueScale);
            canvas->show_text("(x=" + std::to_string(solveX) + " y=" + std::to_string(solveY) + ")");

            // Draw minor and major lines scale
            canvas->set_font_size(12);
            canvas->move_to(18, 18);
            canvas->show_text(std::format("Масштаб ліній відліку змінних: 10^{} = {}", minorLinesPower, std::pow(10, minorLinesPower)));

            return true;
        }
    );

    boxLayout.show_all_children();
    boxLayout.show();
}

void LSESolveOutput::OutputSolve()
{
    outputStatus.set_text(std::format("Рішення виведено {}", GetCurrentFormalTime()));

    std::string varsValuesStr{};

    auto& lseSolveDataV = *lseSolveData.lock();
    const auto& solves = lseSolveDataV.X;

    for (std::size_t solveIndex = 0; solveIndex < solves.Size(); solveIndex++)
    {
        if (solveIndex != 0)
        {
            varsValuesStr += "\n";
        }
        varsValuesStr += std::format("X{}: {}", solveIndex + 1, solves[solveIndex]);
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

    auto& solves = lseSolveDataV.X;

    for (std::size_t solveIndex = 0; solveIndex < solves.Size(); solveIndex++)
    {
        if (solveIndex != 0)
        {
            formattedSolves += " ";
        }
        formattedSolves += std::to_string(solves[solveIndex]);
    }

    WriteToFile(outputFileName.get_text(), formattedSolves);
}

std::string FormatExecTime(double execTime)
{
    struct functions
    {
        static std::ptrdiff_t toInt(double x)
        {
            return (std::ptrdiff_t)(x);
        }
    };

    constexpr double baseMax = 10;

    if (execTime < 0)
    {
        return "(негативне значення)";
    }
    
    if (execTime >= 10)
    {
        return std::to_string(functions::toInt(execTime)) + " с";
    }
    if (execTime >= (baseMax / 1'000))
    {
        return std::to_string(functions::toInt(execTime * 1'000)) + " мс";
    }
    if (execTime >= (baseMax / 1'000'000))
    {
        return std::to_string(functions::toInt(execTime * 1'000'000)) + " μс";
    }
    if (execTime >= (baseMax / 1'000'000'000))
    {
        return std::to_string(functions::toInt(execTime * 1'000'000'000)) + " нс";
    }
    return std::to_string(functions::toInt(execTime * 1'000'000'000'000)) + " пс";
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

    fixedLayout.put(lseConfigurator, 0, 0);
    fixedLayout.put(lseSolver, 650, 0);
    fixedLayout.put(*lseSolveOutput, 650, 310);

    fixedLayout.show();

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
    auto eqsCount = lseInputData->EqsCount;

    for (std::size_t cellY = 0; cellY < eqsCount; cellY++)
    {
        for (std::size_t cellX = 0; cellX < eqsCount; cellX++)
        {
            auto mayCoeffA = ToNumber(varsCoeffsEntries.At(cellY, cellX).get_text());

            if (! mayCoeffA.has_value())
            {
                eqsConfStatus.set_text(
                    std::format("Комірка {} не є числом", GetCoeffAFancyLabel(cellY, cellX))
                );
                return;
            }

            auto coeffA = mayCoeffA.value();

            if (! (-1'000 <= coeffA && coeffA <= 1'000))
            {
                eqsConfStatus.set_text(
                    std::format("Комірка {} не є в діапазоні [-1'000; 1'000]", GetCoeffAFancyLabel(cellY, cellX))
                );
                return;
            }

            lseInputData->A.At(cellY, cellX) = coeffA;
        }

        auto mayCoeffB = ToNumber(freeCoeffsEntries[cellY].get_text());

        if (! mayCoeffB.has_value())
        {
            eqsConfStatus.set_text(
                std::format("Комірка {} не є числом", GetCoeffBFancyLabel(cellY))
            );
            return;
        }

        auto coeffB = mayCoeffB.value();

        if (! (-10'000 <= coeffB && coeffB <= 10'000))
        {
            eqsConfStatus.set_text(
                std::format("Комірка {} не є в діапазоні [-10'000; 10'000]", GetCoeffBFancyLabel(cellY))
            );
            return;
        }

        lseInputData->B[cellY] = coeffB;
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

    lseDataP->EqsCount = eqsCount;
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
    std::size_t eqsCount = lseData.lock()->EqsCount;

    varsCoeffsGrid.hide();
    freeCoeffsGrid.hide();

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        varsCoeffsGrid.remove_column(0);
        varsCoeffsGrid.remove_row(0);

        freeCoeffsGrid.remove_column(0);
    }

    auto lseDataP = lseData.lock();

    lseDataP->EqsCount = 0;
    lseDataP->A = Matrix();
    lseDataP->B = Vector();
    lseDataP->IsSetted = false;

    varsCoeffsEntries = AllocArray2D<Gtk::Entry>();
    freeCoeffsEntries = std::vector<Gtk::Entry>();
}

void LSEConfigurator::fillEmptyEntriesWithZeroes()
{
    auto eqsCount = lseData.lock()->EqsCount;

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

    auto eqsCount = lseInput->EqsCount;
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
