#include "GUI.hpp"

#include "Convert.hpp"
#include "Filesystem.hpp"

#include <cmath>

#include <format>
#include <fstream>
#include <string>
#include <utility>

#include "SLESolver.hpp"
#include "SLESolversData.hpp"

#include <vector>

std::ptrdiff_t GetTimeSeconds()
{
    return time(NULL);
}

std::string GetCurrentFormalTime()
{
    auto secs = GetTimeSeconds();

    return std::format
    (
          "{}:{}:{} UTC+00:00"
        , secs / (60 * 60) % 24
        , secs / 60 % 60
        , secs % 60
    );
}

double PutNumberToBounds(double value, double from, double to)
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

double Logarithm(double poweredValue, double base)
{
    return std::log(poweredValue) / std::log(base);
}

double FloorToZeroWithPrecision(double number, std::size_t maxDigits)
{
    bool isPositive = number >= 0;

    auto multiplier = std::pow(10, maxDigits);

    return (isPositive ? 1 : -1) * (std::floor(std::fabs(number) * multiplier) / multiplier);
}

// class SLEInputData

SLEInputData::SLEInputData() = default;

void SLEInputData::ClearData()
{
    A = Matrix();
    B = Vector();

    isEqsCountSetted = false;
    isConfirmed = false;
}
void SLEInputData::SetEquationsCount(std::size_t eqsCount)
{
    if (isEqsCountSetted)
    {
        return;
    }

    this->eqsCount = eqsCount;

    isEqsCountSetted = true;
}
std::optional<std::size_t> SLEInputData::GetEquationsCount() const noexcept
{
    if (! isEqsCountSetted)
    {
        return std::nullopt;
    }
    return eqsCount;
}
bool SLEInputData::IsEquationsCountSetted() const noexcept
{
    return isEqsCountSetted;
}

std::optional<std::reference_wrapper<Matrix>> SLEInputData::GetVariablesCoefficientsRef()
{
    if (! isEqsCountSetted)
    {
        return std::nullopt;
    }
    return A;
}
std::optional<std::reference_wrapper<const Matrix>> SLEInputData::GetVariablesCoefficientsRef() const
{
    if (! isEqsCountSetted)
    {
        return std::nullopt;
    }
    return A;
}

std::optional<std::reference_wrapper<Vector>> SLEInputData::GetFreeCoefficientsRef()
{
    if (! isEqsCountSetted)
    {
        return std::nullopt;
    }
    return B;
}
std::optional<std::reference_wrapper<const Vector>> SLEInputData::GetFreeCoefficientsRef() const
{
    if (! isEqsCountSetted)
    {
        return std::nullopt;
    }
    return B;
}

void SLEInputData::ConfirmData()
{
    isConfirmed = true;
}
bool SLEInputData::IsDataConfirmed() const noexcept
{
    return isConfirmed;
}

// class SLESolveData

SLESolveData::SLESolveData() = default;

void SLESolveData::SetSolvingStatus(SLESolvingStatus sleSolvingStatus)
{
    solvingStatus = sleSolvingStatus;
}

std::optional<std::reference_wrapper<Vector>> SLESolveData::GetSolveRef()
{
    if (! (solvingStatus == SLESolvingStatus::SolvedSuccessfully))
    {
        return std::nullopt;
    }

    return X;
}
std::optional<std::reference_wrapper<const Vector>> SLESolveData::GetSolveRef() const
{
    if (! (solvingStatus == SLESolvingStatus::SolvedSuccessfully))
    {
        return std::nullopt;
    }

    return X;
}

SLESolvingStatus SLESolveData::GetSolvingStatus() const noexcept
{
    return solvingStatus;
}

// class SLEConfigurator

void SLEConfigurator::SetSLEInputData(std::weak_ptr<SLEInputData> sleInputData)
{
    this->sleData = sleInputData;
}

void SLESolveShower::SetSLEInputData(std::weak_ptr<SLEInputData> sleInputData)
{
    this->sleInputData = sleInputData;
}

void SLESolveShower::SetSLESolveData(std::weak_ptr<SLESolveData> sleSolveData)
{
    this->sleSolveData = sleSolveData;
}

#include <iostream>

SLESolveShower::SLESolveShower()
{
    set_label("Виведення рішень СЛАР");
    set_size_request(400, 400);

    add(boxLayout);

    boxLayout.set_border_width(10);

    boxLayout.set_orientation(Gtk::ORIENTATION_VERTICAL);
    boxLayout.set_spacing(10);

    boxLayout.pack_start(outputStatus);

    boxLayout.pack_start(outputSolveLabel);
    boxLayout.pack_start(outputFileName);
    boxLayout.pack_start(outputButton);

    boxLayout.pack_start(varsDesc);
    boxLayout.pack_start(varsValues);

    boxLayout.pack_start(outputGraphLabel);
    boxLayout.pack_start(outputGraph);

    outputFileName.set_placeholder_text("Файл для виведення...");

    outputButton.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLESolveShower::saveSolve
        )
    );

    outputGraph.set_size_request(300, 300);
    outputGraph.signal_draw().connect
    (
        [this_ = this](const Cairo::RefPtr<Cairo::Context>& canvas)
        {
            auto& outputGraph = this_->outputGraph;

            const Gtk::Allocation canvasAlloc = outputGraph.get_allocation();

            const std::ptrdiff_t width = canvasAlloc.get_width();
            const std::ptrdiff_t height = canvasAlloc.get_height();

            canvas->set_source_rgb(0, 0, 0);
            canvas->rectangle(0, 0, width, height);
            canvas->fill();

            if (! this_->doRenderGraph)
            {
                // Draw scary border
                canvas->set_source_rgb(1, 0, 0);
                canvas->rectangle(0     , 0      , width , 1);
                canvas->rectangle(0     , 0      , 1     , height);
                canvas->rectangle(width , 0      , 1     , height);
                canvas->rectangle(0     , height , width , 1);
                canvas->stroke();

                return true;
            }

            const auto& sleInputData = *this_->sleInputData.lock();
            const auto& sleSolveData = *this_->sleSolveData.lock();

            const auto A = sleInputData.GetVariablesCoefficientsRef().value().get();
            const auto B = sleInputData.GetFreeCoefficientsRef().value().get();

            const auto coeffA1 = A.At(0, 0);
            const auto coeffB1 = A.At(0, 1);
            const auto coeffC1 = B[0];

            const auto coeffA2 = A.At(1, 0);
            const auto coeffB2 = A.At(1, 1);
            const auto coeffC2 = B[1];

            const auto solves = sleSolveData.GetSolveRef().value().get();

            const auto solveX = solves[0];
            const auto solveY = solves[1];

            if (! (sleInputData.GetEquationsCount().value() == 2))
            {
                return true;
            }

            const double centerX = width / 2.0;
            const double centerY = height / 2.0;

            const double maxSolvedVarValue = std::max(
                std::fabs(solveX), std::fabs(solveY)
            );

            const double valueToPixelsScale = std::min(10e10, (std::min(width, height) / 2) / maxSolvedVarValue / 3);

            // Draw major and minor lines:
            const double maxSolvedVarValueMinLimited = std::max(10e-9, maxSolvedVarValue);

            const double minorLinesPower = std::floor(Logarithm(maxSolvedVarValueMinLimited, 10));
            const double majorLinesPower = std::floor(Logarithm(10 * maxSolvedVarValueMinLimited, 10));

            const double minorLinesValueStep = std::pow(10, minorLinesPower);
            const double majorLinesValueStep = std::pow(10, majorLinesPower);

            const double minorLinesPixelsStep = minorLinesValueStep * valueToPixelsScale;
            const double majorLinesPixelsStep = majorLinesValueStep * valueToPixelsScale;

            const double minorLinesCountByX = std::ceil((width / 2) / minorLinesPixelsStep) * minorLinesPixelsStep;
            const double minorLinesCountByY = std::ceil((height / 2) / minorLinesPixelsStep) * minorLinesPixelsStep;

            const double majorLinesCountByX = std::ceil((width / 2) / majorLinesPixelsStep) * majorLinesPixelsStep;
            const double majorLinesCountByY = std::ceil((height / 2) / majorLinesPixelsStep) * majorLinesPixelsStep;

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

            const double coeffK1 = PutNumberToBounds(-coeffA1 / coeffB1, -100, 100);
            const double coeffBase1 = coeffC1 / coeffB1;

            const double coeffK2 = PutNumberToBounds(-coeffA2 / coeffB2, -100, 100);
            const double coeffBase2 = coeffC2 / coeffB2;

            // Draw the first line
            canvas->set_source_rgb(1.0, 0.0, 0.0);
            canvas->move_to(0,     centerY - (-centerX * coeffK1 + coeffBase1 * valueToPixelsScale));
            canvas->line_to(width, centerY - (centerX * coeffK1 + coeffBase1 * valueToPixelsScale));
            canvas->stroke();

            std::cout << "------------------" << std::endl;

            std::cout << centerY - (-centerX * coeffK1 + coeffBase1 * valueToPixelsScale) << std::endl;
            std::cout << centerY - (centerX * coeffK1 + coeffBase1 * valueToPixelsScale) << std::endl;

            // Draw the second line
            canvas->set_source_rgb(0.0, 0.0, 1.0);
            canvas->move_to(0,     centerY - (-centerX * coeffK2 + coeffBase2 * valueToPixelsScale));
            canvas->line_to(width, centerY - (centerX * coeffK2 + coeffBase2 * valueToPixelsScale));
            canvas->stroke();

            std::cout << centerY - (coeffBase2 * valueToPixelsScale) << std::endl;
            std::cout << centerY - (coeffBase2 * valueToPixelsScale) << std::endl;

            // Draw X and Y axises text.
            canvas->set_font_size(12);
            canvas->move_to(centerX + 12 / 2 * 1.5, 12);

            canvas->set_source_rgb(1, 1, 1);
            canvas->show_text("Y (X2)");

            canvas->move_to(width - 36, centerY + 12 * 1.5);
            canvas->show_text("X (X1)");

            // Draw coordinate
            canvas->set_font_size(12);
            canvas->move_to(centerX + solveX * valueToPixelsScale, centerY - solveY * valueToPixelsScale);
            canvas->show_text("(x=" + std::to_string(solveX) + " y=" + std::to_string(solveY) + ")");

            // Draw minor and major lines scale
            canvas->set_font_size(12);
            canvas->move_to(18, 24);
            canvas->show_text("Масштаб ліній відліку змінних");
            canvas->move_to(18, 24 + 12 + 2);
            canvas->show_text(std::format("(10^{} = {})", minorLinesPower, std::pow(10, minorLinesPower)));

            // Draw pretty border
            canvas->set_source_rgb(1, 1, 1);
            canvas->rectangle(0     , 0      , width , 1);
            canvas->rectangle(0     , 0      , 1     , height);
            canvas->rectangle(width , 0      , 1     , height);
            canvas->rectangle(0     , height , width , 1);
            canvas->stroke();

            return true;
        }
    );

    boxLayout.show_all_children();
    boxLayout.show();
}

void SLESolveShower::OutputSolve()
{
    outputStatus.set_text(std::format("Рішення виведено {}", GetCurrentFormalTime()));

    std::string varsValuesStr{};

    const auto& sleSolveDataV = *sleSolveData.lock();
    const auto& solves = (sleSolveDataV).GetSolveRef().value().get();

    for (std::size_t solveIndex = 0; solveIndex < solves.Size(); solveIndex++)
    {
        if (solveIndex != 0)
        {
            varsValuesStr += "\n";
        }
        varsValuesStr += std::format("X{}: {}", solveIndex + 1, solves[solveIndex]);
    }

    varsValues.set_text(varsValuesStr);

    if (solves.Size() == 2)
    {
        doRenderGraph = true;
        outputGraph.queue_draw();
    }
}

void SLESolveShower::ClearSolve()
{
    outputStatus.set_text("Рішення неможливо отримати");
    varsValues.set_text("(розв'язків немає)");

    outputGraph.queue_draw();
    doRenderGraph = false;
}

void SLESolveShower::saveSolve()
{
    auto& sleSolveDataV = *sleSolveData.lock();

    if (sleSolveDataV.GetSolvingStatus() != SLESolvingStatus::SolvedSuccessfully)
    {
        return;
    }

    std::string formattedSolves = "";

    const auto& solves = sleSolveDataV.GetSolveRef().value().get();

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

void ApplicationWindow::initializeWindowHead()
{
    set_title("Розв'язувач СЛАР точними методами");

    set_default_size(1000, 600);
    set_resizable(false);

    set_border_width(10);
}

void ApplicationWindow::SetApplicationData(std::weak_ptr<ApplicationData> appData)
{
    sleConfigurator.SetSLEInputData(appData.lock()->GetSLEInputData());

    sleSolver.SetSLEInputData(appData.lock()->GetSLEInputData());
    sleSolver.SetSLESolveData(appData.lock()->GetSLEOutputData());

    sleSolver.SetSLESolveOutput(sleSolveOutput);

    sleSolveOutput->SetSLEInputData(appData.lock()->GetSLEInputData());
    sleSolveOutput->SetSLESolveData(appData.lock()->GetSLEOutputData());
}

void ApplicationWindow::initializeWidgets()
{
    set_sensitive(false);

    add(fixedLayout);

    fixedLayout.put(sleConfigurator, 0, 0);
    fixedLayout.put(sleSolver, 1200 + 10, 0);
    fixedLayout.put(*sleSolveOutput, 1200 + 10, 260 + 10);

    fixedLayout.show();

    sleConfigurator.show();
    sleSolver.show();
    (*sleSolveOutput).show();
}

void ApplicationWindow::Ready()
{
    set_sensitive(true);
}

ApplicationWindow::ApplicationWindow()
{
    initializeWindowHead();

    initializeWidgets();
}

SLEConfigurator::SLEConfigurator()
{
    set_label("Конфігурація СЛАР");
    set_size_request(1200, -1);

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

void SLEConfigurator::initializeEqsCount()
{
    eqsPropConfGrid.attach(eqsConfStatus, 0, 0, 3, 1);

    eqsPropConfGrid.attach(eqsCountProp, 0, 2);
    eqsPropConfGrid.attach(eqsCountValue, 2, 2);

    eqsCountHorPadding1.set_margin_right(12);
    eqsCountHorPadding2.set_margin_right(12);

    eqsCountVerPadding1.set_margin_bottom(18);
    eqsCountVerPadding2.set_margin_bottom(18);
    eqsCountVerPadding3.set_margin_bottom(18);
    eqsCountVerPadding4.set_margin_bottom(36);
    eqsCountVerPadding5.set_margin_bottom(18);

    eqsPropConfGrid.attach(eqsCountHorPadding1, 1, 0, 1, 5);
    eqsPropConfGrid.attach(eqsCountHorPadding2, 3, 0, 1, 5);

    eqsPropConfGrid.attach(eqsCountVerPadding1, 0, 1, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding2, 0, 3, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding3, 0, 5, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding4, 0, 7, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding5, 0, 9, 5, 1);

    eqsPropConfGrid.attach(eqsSetterProp, 0, 4);
    eqsPropConfGrid.attach(eqsSetterEntry, 2, 4);
    eqsPropConfGrid.attach(eqsSetterButton, 4, 4);

    eqsPropConfGrid.attach(eqsZeroFillerButton, 0, 6);
    eqsPropConfGrid.attach(eqsSetAsInput, 2, 6);

    eqsPropConfGrid.attach(sleFormLabel, 0, 8);

    eqsSetterEntry.set_placeholder_text("1..10");

    eqsCountProp.set_width_chars(18);
    eqsSetterProp.set_width_chars(18);

    eqsSetterButton.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLEConfigurator::onEqsCountSetting
        )
    );

    eqsZeroFillerButton.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLEConfigurator::fillEmptyEntriesWithZeroes
        )
    );

    eqsSetAsInput.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLEConfigurator::setEqsAsInput
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
    return std::format("A[змінна №{}, рівняння №{}]", x + 1, y + 1);
}

std::string GetCoeffBShortLabel(double eqIndex)
{
    return std::format("b{}", eqIndex + 1);
}

std::string GetCoeffBFancyLabel(double eqIndex)
{
    return std::format("B[рівняння №{}]", eqIndex + 1);
}

void SLEConfigurator::setEqsAsInput()
{
    auto& sleDataV = *sleData.lock();

    if (! sleDataV.IsEquationsCountSetted())
    {
        return;
    }

    auto eqsCount = sleDataV.GetEquationsCount().value();

    auto& A = sleDataV.GetVariablesCoefficientsRef().value().get();
    auto& B = sleDataV.GetFreeCoefficientsRef().value().get();

    for (std::size_t cellY = 0; cellY < eqsCount; cellY++)
    {
        for (std::size_t cellX = 0; cellX < eqsCount; cellX++)
        {
            auto mayCoeffA = ToNumber(varsCoeffsEntries.At(cellY, cellX).get_text());

            if (! mayCoeffA.has_value())
            {
                eqsConfStatus.set_text
                (
                    std::format("Комірка {} не є числом", GetCoeffAFancyLabel(cellY, cellX))
                );
                return;
            }

            auto coeffA = mayCoeffA.value();

            if (! (-1'000 <= coeffA && coeffA <= 1'000))
            {
                eqsConfStatus.set_text
                (
                    std::format("Комірка {} не є в діапазоні [-1'000; 1'000]", GetCoeffAFancyLabel(cellY, cellX))
                );
                return;
            }

            A.At(cellY, cellX) = FloorToZeroWithPrecision(coeffA, 6);
        }

        auto mayCoeffB = ToNumber(freeCoeffsEntries[cellY].get_text());

        if (! mayCoeffB.has_value())
        {
            eqsConfStatus.set_text
            (
                std::format("Комірка {} не є числом", GetCoeffBFancyLabel(cellY))
            );
            return;
        }

        auto coeffB = mayCoeffB.value();

        if (! (-10'000 <= coeffB && coeffB <= 10'000))
        {
            eqsConfStatus.set_text
            (
                std::format("Комірка {} не є в діапазоні [-10'000; 10'000]", GetCoeffBFancyLabel(cellY))
            );
            return;
        }

        B[cellY] = FloorToZeroWithPrecision(coeffB, 6);
    }

    eqsConfStatus.set_text
    (
        std::format("Дана СЛАР встановлена {}", GetCurrentFormalTime())
    );

    sleDataV.ConfirmData();
}

void SLEConfigurator::initializeEqsForm()
{
    eqsFormBox.pack_start(varsCoeffsGrid, Gtk::PACK_SHRINK);
    eqsFormBox.pack_start(eqMarksGrid, Gtk::PACK_SHRINK);
    eqsFormBox.pack_start(freeCoeffsGrid, Gtk::PACK_SHRINK);

    eqsFormBox.show_all_children();
}

void SLEConfigurator::onEqsCountSetting()
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

    removeSLEForm();
    createSLEForm(eqsCount);
}

void SLEConfigurator::createSLEForm(std::size_t eqsCount)
{
    varsCoeffsEntries = RTArray2D<Gtk::Entry>(eqsCount, eqsCount);
    varsCoeffsLabels  = RTArray2D<Gtk::Label>(eqsCount * 2, eqsCount);
    leEquationMarkLabels = std::vector<Gtk::Label>(eqsCount);
    freeCoeffsEntries = std::vector<Gtk::Entry>(eqsCount);

    auto& sleDataV = *sleData.lock();

    sleDataV.SetEquationsCount(eqsCount);

    sleDataV.SetVariablesCoefficients(Matrix(eqsCount, eqsCount));
    sleDataV.SetFreeCoefficients(Vector(eqsCount));

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto& newVarCoeff = varsCoeffsEntries.At(y, x);
            
            newVarCoeff = Gtk::Entry();
            newVarCoeff.set_width_chars(4);
            newVarCoeff.set_max_length(16);

            newVarCoeff.set_placeholder_text
            (
                GetCoeffAShortLabel(y, x)
            );
            newVarCoeff.set_tooltip_text
            (
                GetCoeffAFancyLabel(y, x)
            );

            auto& newIndexedVar  = varsCoeffsLabels.At(y, 2 * x);
            auto& newOperatorAdd = varsCoeffsLabels.At(y, 2 * x + 1);

            newIndexedVar.set_text(std::format(" X{}", x + 1));
            newIndexedVar.set_size_request(-1, 42);

            newOperatorAdd.set_text("+");
            newOperatorAdd.set_size_request(24, 42);

            varsCoeffsGrid.attach(newVarCoeff   , 3 * x, y);
            varsCoeffsGrid.attach(newIndexedVar , 3 * x + 1, y);

            if (x != eqsCount - 1)
            {
                varsCoeffsGrid.attach(newOperatorAdd, 3 * x + 2, y);
            }
        }

        auto& newLEEquationMark = leEquationMarkLabels[y];
        newLEEquationMark = Gtk::Label();
        newLEEquationMark.set_text("=");
        newLEEquationMark.set_size_request(32, 42);
        eqMarksGrid.attach(newLEEquationMark, 0, y);

        auto& newFreeCoeff = freeCoeffsEntries[y];
        newFreeCoeff = Gtk::Entry();
        newFreeCoeff.set_width_chars(3);
        newFreeCoeff.set_size_request(24, 42);
        newFreeCoeff.set_max_length(16);

        newFreeCoeff.set_placeholder_text
        (
            GetCoeffBShortLabel(y)
        );
        newFreeCoeff.set_tooltip_text
        (
            GetCoeffBFancyLabel(y)
        );

        freeCoeffsGrid.attach(newFreeCoeff, 0, y);
    }

    varsCoeffsGrid.show_all_children();
    eqMarksGrid.show_all_children();
    freeCoeffsGrid.show_all_children();

    varsCoeffsGrid.show();
    eqMarksGrid.show();
    freeCoeffsGrid.show();
}

void SLEConfigurator::removeSLEForm()
{
    auto& sleDataV = *sleData.lock();

    if (! sleDataV.IsEquationsCountSetted())
    {
        return;
    }

    std::size_t eqsCount = sleDataV.GetEquationsCount().value();
    sleDataV.ClearData();

    varsCoeffsGrid.hide();
    eqMarksGrid.hide();
    freeCoeffsGrid.hide();

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        varsCoeffsGrid.remove_column(0);
        eqMarksGrid.remove_column(0);
        freeCoeffsGrid.remove_column(0);
    }

    varsCoeffsEntries = RTArray2D<Gtk::Entry>();
    varsCoeffsLabels  = RTArray2D<Gtk::Label>();

    leEquationMarkLabels = std::vector<Gtk::Label>{};

    freeCoeffsEntries = std::vector<Gtk::Entry>();
}

void SLEConfigurator::fillEmptyEntriesWithZeroes()
{
    constexpr auto formattedZero = "0";

    const auto& sleDataV = *sleData.lock();

    if (! sleDataV.IsEquationsCountSetted())
    {
        return;
    }

    auto eqsCount = sleDataV.GetEquationsCount().value();

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto& currentEntryOfMatrixA = varsCoeffsEntries.At(y, x);
            const auto& origEntryInput = currentEntryOfMatrixA.get_text();

            if (origEntryInput == "")
            {
                currentEntryOfMatrixA.set_text(formattedZero);
            }
        }

        auto& currentEntryOfVectorB = freeCoeffsEntries[y];
        const auto& origEntryInput = currentEntryOfVectorB.get_text();

        if (origEntryInput == "")
        {
            currentEntryOfVectorB.set_text(formattedZero);
        }
    }
}

SLESolvePanel::SLESolvePanel()
{
    set_label("Розв'язання СЛАР");
    set_size_request(400, 260);

    add(solverRootBox);

    solverRootBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    solverRootBox.set_border_width(10);

    solverRootBox.set_spacing(10);

    solverRootBox.pack_start(solverInstruction);
    solverRootBox.pack_start(comboBoxMethodsNames);
    solverRootBox.pack_start(solveButton);
    solverRootBox.pack_start(solvingStatus);
    solverRootBox.pack_start(practicalTimeComplexity);

    solveButton.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLESolvePanel::onSolvingProcess
        )
    );

    for (std::size_t methodIndex = 0; methodIndex < comboBoxMethodRecords.size(); methodIndex++)
    {
        auto currentOption = comboBoxMethodRecords[methodIndex];

        comboBoxMethodsNames.append
        (
              std::to_string(methodIndex)
            , std::format
            (
                  "{}\nСкладність: {}"
                , currentOption.GetMethodName()
                , currentOption.GetMethodPracticalItersComplexity()
            )
        );
    }

    solverRootBox.show_all_children();
    solverRootBox.show();
}

void SLESolvePanel::SetSLEInputData(std::weak_ptr<SLEInputData> sleInputData)
{
    this->sleInputData = sleInputData;
}
void SLESolvePanel::SetSLESolveData(std::weak_ptr<SLESolveData> sleSolveData)
{
    this->sleSolveData = sleSolveData;
}

void SLESolvePanel::SetSLESolveOutput(std::weak_ptr<SLESolveShower> sleSolveOutput)
{
    this->sleSolveOutput = sleSolveOutput;
}

void SLESolvePanel::onSolvingProcess()
{
    auto comboBoxMethodIndex = comboBoxMethodsNames.get_active_id();

    if (comboBoxMethodIndex == "")
    {
        solvingStatus.set_text("Метод не встановлено");
        return;
    }

    auto sleInput = *sleInputData.lock();

    if (! sleInput.IsDataConfirmed())
    {
        solvingStatus.set_text("СЛАР ще не встановлена");
        return;
    }

    auto eqsCount = sleInput.GetEquationsCount().value();

    auto& A = sleInput.GetVariablesCoefficientsRef().value().get();
    auto& B = sleInput.GetFreeCoefficientsRef().value().get();

    auto solvingMethodP = SLESolverFactoryProduce
    (
        comboBoxMethodRecords
        [
           ToInteger(comboBoxMethodIndex).value()
        ]
        .GetSolvingMethodIndex()
    );

    auto& solvingMethod = *solvingMethodP;

    solvingMethod.SetEquationsCount(eqsCount);
    solvingMethod.SetVariablesCoefficients(A);
    solvingMethod.SetFreeCoefficients(B);

    solvingMethod.Solve();

    auto& sleSolvesPlaceV = *sleSolveData.lock();

    if (! solvingMethod.IsSolvedSuccessfully().value())
    {
        sleSolveOutput.lock()->ClearSolve();

        sleSolvesPlaceV.SetSolvingStatus(SLESolvingStatus::SolvedFailful);
        solvingStatus.set_text("СЛАР не можливо вирішити цим методом");
        practicalTimeComplexity.set_text("");
        return;
    }

    auto X = solvingMethod.GetSolveOnce().value();

    sleSolvesPlaceV.SetVarsSolve(std::move(X));
    sleSolvesPlaceV.SetSolvingStatus(SLESolvingStatus::SolvedSuccessfully);

    sleInput.ClearData();

    solvingStatus.set_text
    (
        std::format("СЛАР вирішено {}", GetCurrentFormalTime())
    );
    practicalTimeComplexity.set_text
    (
        std::format("СЛАР вирішено за {} ітерацій", solvingMethod.GetAlgoItersCount().value())
    );

    sleSolveOutput.lock()->OutputSolve();
}

// class GUISession

GUISession::GUISession() = default;

void GUISession::Init()
{
    appWin.SetApplicationData(appData);
    appWin.Ready();
}

Gtk::Window& GUISession::GetWindowRef()
{
    return appWin;
}
