#include "GUI.hpp"

#include "Convert.hpp"
#include "Filesystem.hpp"
#include "LinAlgUtility.hpp"
#include "Math.hpp"
#include "SLESolver.hpp"
#include "SLESolversData.hpp"
#include "Time.hpp"

#include <cmath>

#include <format>
#include <fstream>
#include <iomanip>
#include <string>
#include <utility>

#include <vector>

std::string GUIUtilityFuncs::GetCoeffAShortLabel(std::size_t eqIndex, std::size_t varIndex)
{
    return std::format("a{},{}", eqIndex + 1, varIndex + 1);
}

std::string GUIUtilityFuncs::GetCoeffAFancyLabel(std::size_t eqIndex, std::size_t varIndex)
{
    return std::format("A[рівняння №{}, змінна №{}]", eqIndex + 1, varIndex + 1);
}

std::string GUIUtilityFuncs::GetCoeffBShortLabel(std::size_t eqIndex)
{
    return std::format("b{}", eqIndex + 1);
}

std::string GUIUtilityFuncs::GetCoeffBFancyLabel(std::size_t eqIndex)
{
    return std::format("B[рівняння №{}]", eqIndex + 1);
}

std::string GUIUtilityFuncs::ToShortScientificForm(double number)
{
    std::stringstream ss{};

    ss << std::scientific << std::setprecision(3) << number;

    return ss.str();
}

double GUIUtilityFuncs::UniformRandom()
{
    return std::fmod((double)std::rand() / 65536, 1);
}

bool GUIUtilityFuncs::IsDetCloseToZero(double det)
{
    return std::fabs(det) < 10e-12;
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

// GUI modules classes methods implementations

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

SLESolveShower::SLESolveShower()
{
    set_label("Виведення рішень СЛАР");
    set_size_request(480, 400);

    add(boxLayout);

    // set the rendering parameters

    boxLayout.set_border_width(10);

    boxLayout.set_orientation(Gtk::ORIENTATION_VERTICAL);
    boxLayout.set_spacing(10);

    // render the main widgets here

    boxLayout.pack_start(outputStatus);

    boxLayout.pack_start(outputSolveLabel);
    boxLayout.pack_start(outputFileName);
    boxLayout.pack_start(outputButton);

    boxLayout.pack_start(varsDesc);
    boxLayout.pack_start(unlimitedVarsValuesScroller);

    unlimitedVarsValuesScrollerBox.pack_start(varsValues);
    unlimitedVarsValuesScrollerBox.set_vexpand(true);

    unlimitedVarsValuesScroller.add(unlimitedVarsValuesScrollerBox);

    unlimitedVarsValuesScroller.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    unlimitedVarsValuesScroller.set_min_content_height(10);
    unlimitedVarsValuesScroller.set_max_content_height(80);

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

    outputGraph.set_size_request(300, 240);
    outputGraph.signal_draw().connect
    (
        [this_ = this](const Cairo::RefPtr<Cairo::Context>& canvas)
        {
            // get width and height here
            auto& outputGraph = this_->outputGraph;

            const Gtk::Allocation canvasAlloc = outputGraph.get_allocation();

            const std::ptrdiff_t width = canvasAlloc.get_width();
            const std::ptrdiff_t height = canvasAlloc.get_height();

            // fill the background by default
            canvas->set_source_rgb(0, 0, 0);
            canvas->rectangle(0, 0, width, height);
            canvas->fill();

            // if it is a mistake to draw the graph
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

            // obtaining the input and solve data of SLE
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

            // there is a start of rendering the graph
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

            // draw major and minor lines
            const double maxSolvedVarValueMinLimited = std::max(10e-9, maxSolvedVarValue);

            const double minorLinesPower = std::floor(Math::Logarithm(maxSolvedVarValueMinLimited, 10));
            const double majorLinesPower = std::floor(Math::Logarithm(10 * maxSolvedVarValueMinLimited, 10));

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

            // draw X and Y axes
            canvas->set_line_width(2.0);
            canvas->set_source_rgb(0.0, 1.0, 0.0);

            canvas->move_to(0.0, centerY);
            canvas->line_to(width, centerY);

            canvas->move_to(centerX, 0.0);
            canvas->line_to(centerX, height);
            canvas->stroke();

            //
            // the code below will choose one of most optimal formula below for each line:
            //
            // 1. y = kx + b
            // 2. x = ly + d
            //
            // else gtkmm drawing area won't draw line because
            // at least one of coordinates of one of points become too large
            // for rendering
            //

            // get lines' alternative coefficients
            const double coeffK1X = -coeffA1 / coeffB1;
            const double coeffK1Y = -coeffB1 / coeffA1;

            const double coeffBase1Y = coeffC1 / coeffB1;
            const double coeffBase1X = coeffC1 / coeffA1;

            const double coeffK2X = -coeffA2 / coeffB2;
            const double coeffK2Y = -coeffB2 / coeffA2;

            const double coeffBase2Y = coeffC2 / coeffB2;
            const double coeffBase2X = coeffC2 / coeffA2;

            // Draw the first line
            canvas->set_source_rgb(1.0, 0.0, 0.0);

            if (Math::IsNumberInRange(coeffK1X, -1, 1))
            {
                canvas->move_to(0     , centerY - (-centerX * coeffK1X + coeffBase1Y * valueToPixelsScale));
                canvas->line_to(width , centerY - (centerX * coeffK1X + coeffBase1Y * valueToPixelsScale));
            }
            else
            {
                canvas->move_to(centerX + (centerY * coeffK1Y + coeffBase1X * valueToPixelsScale)  , 0);
                canvas->line_to(centerX + (-centerY * coeffK1Y + coeffBase1X * valueToPixelsScale) , height);
            }
            canvas->stroke();

            // draw the second line
            canvas->set_source_rgb(0.0, 0.0, 1.0);

            if (Math::IsNumberInRange(coeffK2X, -1, 1))
            {
                canvas->move_to(0,     centerY - (-centerX * coeffK2X + coeffBase2Y * valueToPixelsScale));
                canvas->line_to(width, centerY - (centerX * coeffK2X + coeffBase2Y * valueToPixelsScale));
            }
            else
            {
                canvas->move_to(centerX + (centerY * coeffK2Y + coeffBase2X * valueToPixelsScale)  , 0);
                canvas->line_to(centerX + (-centerY * coeffK2Y + coeffBase2X * valueToPixelsScale) , height);
            }
            canvas->stroke();

            // draw X and Y axises text.
            canvas->set_font_size(12);
            canvas->move_to(centerX + 12 / 2 * 1.5, 12);

            canvas->set_source_rgb(1, 1, 1);
            canvas->show_text("Y (X2)");

            canvas->move_to(width - 42, centerY + 12 * 1.5);
            canvas->show_text("X (X1)");

            auto intersectionDrawX = centerX + solveX * valueToPixelsScale;
            auto intersectionDrawY = centerY - solveY * valueToPixelsScale;

            // draw the intersection
            canvas->set_source_rgb(1, 1, 0);
            canvas->rectangle(intersectionDrawX - 2, intersectionDrawY - 2, 4, 4);
            canvas->stroke();

            // draw coordinate
            canvas->set_source_rgb(1, 1, 1);
            canvas->set_font_size(12);
            canvas->move_to(intersectionDrawX + 3 - 100, intersectionDrawY - 3);
            canvas->show_text(
                std::format(
                      "(x={} y={})"
                    , GUIUtilityFuncs::ToShortScientificForm(solveX)
                    , GUIUtilityFuncs::ToShortScientificForm(solveY)
                )
            );

            // draw minor and major lines scale
            canvas->set_source_rgb(197.0/255, 172.0/255, 255.0/255);
            canvas->set_font_size(12);

            canvas->move_to(18, 36);
            canvas->show_text("Масштаб ліній відліку змінних");
    
            canvas->move_to(18, 36 + 12 + 2);
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
    outputStatus.set_text(
        std::format(
              "Рішення виведено {}"
            , Time::GetCurrentFormalTime()
        )
    );


    const auto sleSolveDataSP = sleSolveData.lock();

    const auto& solves = (*sleSolveDataSP).GetSolveRef().value().get();
    auto eqsCount = solves.Size();

    std::string varsValuesStr{};

    if (eqsCount != 2)
    {
        for (std::size_t solveIndex = 0; solveIndex < eqsCount; solveIndex++)
        {
            if (solveIndex != 0)
            {
                varsValuesStr += "\n";
            }
            varsValuesStr += std::format("X{}: {:.9f}", solveIndex + 1, solves[solveIndex]);
        }

        doRenderGraph = false;
        outputGraph.queue_draw();
    }
    else
    {
        varsValuesStr = std::format("X (X1, червона):\t{}\nY (X2, синя):\t\t{}", solves[0], solves[1]);

        doRenderGraph = true;
        outputGraph.queue_draw();
    }

    varsValues.set_text(varsValuesStr);
}

void SLESolveShower::ShowInvalidSolve()
{
    outputStatus.set_text("Рішення неможливо отримати");
    varsValues.set_text("(розв'язків немає)");

    doRenderGraph = false;
    outputGraph.queue_draw();
}

void SLESolveShower::saveSolve()
{
    auto sleSolveDataSP = sleSolveData.lock();
    auto& sleSolveData = *sleSolveDataSP;

    if (sleSolveData.GetSolvingStatus() != SLESolvingStatus::SolvedSuccessfully)
    {
        return;
    }

    // the data for outputing into file
    std::string formattedSolves = "";

    const auto& solves = sleSolveData.GetSolveRef().value().get();

    for (std::size_t solveIndex = 0; solveIndex < solves.Size(); solveIndex++)
    {
        if (solveIndex != 0)
        {
            formattedSolves += " ";
        }
        formattedSolves += Convert::NumberToString(solves[solveIndex]);
    }

    Filesystem::WriteToFile(outputFileName.get_text(), formattedSolves);
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
    // configure the relations between GUI modules
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

    // put GUI modules to absolute positions
    fixedLayout.put(sleConfigurator, 0, 0);
    fixedLayout.put(sleSolver, 640 + 10, 0);
    fixedLayout.put(*sleSolveOutput, 640 + 10, 260 + 10);

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
    set_size_request(640, 864 - 20 - 32);

    add(boxLayout);

    boxLayout.set_orientation(Gtk::ORIENTATION_VERTICAL);
    boxLayout.set_border_width(10);

    initializeEqsCount();
    initializeEqsForm();

    boxLayout.pack_start(eqsPropConfGrid);
    boxLayout.pack_start(unlimitedSLEFormScroller);

    boxLayout.show_all_children();
    boxLayout.show();
}

void SLEConfigurator::initializeEqsCount()
{
    // attach a lot of children widgets to a cute grid of items
    eqsPropConfGrid.attach(eqsConfStatus, 0, 0, 5, 1);

    eqsPropConfGrid.attach(eqsCountProp, 0, 2);
    eqsPropConfGrid.attach(eqsCountValue, 2, 2);

    eqsCountHorPadding1.set_margin_right(12);
    eqsCountHorPadding2.set_margin_right(12);

    eqsCountVerPadding1.set_margin_bottom(18);
    eqsCountVerPadding2.set_margin_bottom(18);
    eqsCountVerPadding3.set_margin_bottom(18);
    eqsCountVerPadding4.set_margin_bottom(18);
    eqsCountVerPadding5.set_margin_bottom(36);

    eqsPropConfGrid.attach(eqsCountHorPadding1, 1, 0, 1, 5);
    eqsPropConfGrid.attach(eqsCountHorPadding2, 3, 0, 1, 5);

    eqsPropConfGrid.attach(eqsCountVerPadding1, 0,  1, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding2, 0,  3, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding3, 0,  5, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding4, 0,  7, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding5, 0,  9, 5, 1);
    eqsPropConfGrid.attach(eqsCountVerPadding6, 0, 11, 5, 1);

    eqsPropConfGrid.attach(eqsSetterProp, 0, 4);
    eqsPropConfGrid.attach(eqsSetterEntry, 2, 4);
    eqsPropConfGrid.attach(eqsSetterButton, 4, 4);

    eqsPropConfGrid.attach(eqsZeroFillerButton, 0, 6);
    eqsPropConfGrid.attach(fillUpSLEFormRandomly, 2, 6);
    eqsPropConfGrid.attach(clearSLEForm, 4, 6);

    eqsPropConfGrid.attach(setSLEFormButton, 0, 8, 3, 1);

    eqsPropConfGrid.attach(sleFormLabel, 0, 10);

    eqsSetterEntry.set_placeholder_text("1..10");

    eqsCountProp.set_width_chars(18);
    eqsSetterProp.set_width_chars(18);

    eqsSetterButton.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLEConfigurator::setEqsCount
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

    fillUpSLEFormRandomly.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLEConfigurator::fillEmptyEntriesWithRandomNumbers
        )
    );

    clearSLEForm.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLEConfigurator::doClearSLEForm
        )
    );

    setSLEFormButton.signal_clicked().connect
    (
        sigc::mem_fun
        (
              *this
            , &SLEConfigurator::setSLEForm
        )
    );

    eqsPropConfGrid.show_all_children();
}

void SLEConfigurator::setSLEForm()
{
    auto sleInputDataSP = sleData.lock();
    auto& sleInputData = *sleInputDataSP;

    if (! sleInputData.IsEquationsCountSetted())
    {
        return;
    }

    auto eqsCount = sleInputData.GetEquationsCount().value();

    auto& A = sleInputData.GetVariablesCoefficientsRef().value().get();
    auto& B = sleInputData.GetFreeCoefficientsRef().value().get();

    for (std::size_t cellY = 0; cellY < eqsCount; cellY++)
    {
        for (std::size_t cellX = 0; cellX < eqsCount; cellX++)
        {
            auto mayCoeffA = Convert::ToNumber
            (
                varsCoeffsEntries.At(cellY, cellX).get_text()
            );

            if (! mayCoeffA.has_value())
            {
                eqsConfStatus.set_text
                (
                    std::format
                    (
                          "Комірка {} не є числом"
                        , GUIUtilityFuncs::GetCoeffAFancyLabel(cellY, cellX)
                    )
                );
                return;
            }

            auto coeffA = mayCoeffA.value();

            if (! (-1'000 <= coeffA && coeffA <= 1'000))
            {
                eqsConfStatus.set_text
                (
                    std::format
                    (
                          "Комірка {} не є в діапазоні [-1'000; 1'000]"
                        , GUIUtilityFuncs::GetCoeffAFancyLabel(cellY, cellX)
                    )
                );
                return;
            }

            A.At(cellY, cellX) = Math::FloorWithPrecision(coeffA, 6);
        }

        auto mayCoeffB = Convert::ToNumber(freeCoeffsEntries[cellY].get_text());

        if (! mayCoeffB.has_value())
        {
            eqsConfStatus.set_text
            (
                std::format
                (
                      "Комірка {} не є числом"
                    , GUIUtilityFuncs::GetCoeffBFancyLabel(cellY)
                )
            );
            return;
        }

        auto coeffB = mayCoeffB.value();

        if (! (-10'000 <= coeffB && coeffB <= 10'000))
        {
            eqsConfStatus.set_text
            (
                std::format
                (
                      "Комірка {} не є в діапазоні [-10'000; 10'000]"
                    , GUIUtilityFuncs::GetCoeffBFancyLabel(cellY)
                )
            );
            return;
        }

        B[cellY] = Math::FloorWithPrecision(coeffB, 6);
    }

    eqsConfStatus.set_text
    (
        std::format
        (
              "Дана СЛАР встановлена {}"
            , Time::GetCurrentFormalTime()
        )
    );

    sleInputData.ConfirmData();
}

void SLEConfigurator::initializeEqsForm()
{
    unlimitedSLEFormScroller.add(eqsFormBox);
    
    unlimitedSLEFormScroller.set_max_content_width(1170);
    unlimitedSLEFormScroller.set_max_content_height(478);
    unlimitedSLEFormScroller.set_min_content_height(478);

    eqsFormBox.pack_start(varsCoeffsGrid, Gtk::PACK_SHRINK);
    eqsFormBox.pack_start(eqMarksGrid, Gtk::PACK_SHRINK);
    eqsFormBox.pack_start(freeCoeffsGrid, Gtk::PACK_SHRINK);

    unlimitedSLEFormScroller.show_all_children();
    eqsFormBox.show_all_children();
}

void SLEConfigurator::setEqsCount()
{
    auto mayEqsCount = Convert::ToInteger
    (
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
    // initialization of new widgets for SLE form
    varsCoeffsEntries = RTArray2D<Gtk::Entry>(eqsCount, eqsCount);
    varsCoeffsLabels  = RTArray2D<Gtk::Label>(eqsCount * 2, eqsCount);

    leEquationMarkLabels = std::vector<Gtk::Label>(eqsCount);

    freeCoeffsEntries = std::vector<Gtk::Entry>(eqsCount);

    // Getting the input data
    auto sleInputDataSP = sleData.lock();
    auto& sleInputData = *sleInputDataSP;

    // Initialization of SLE form
    sleInputData.SetEquationsCount(eqsCount);

    sleInputData.SetVariablesCoefficients(Matrix(eqsCount, eqsCount));
    sleInputData.SetFreeCoefficients(Vector(eqsCount));

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            // add a new variable coefficient entry
            auto& newVarCoeff = varsCoeffsEntries.At(y, x);
            
            newVarCoeff = Gtk::Entry();
            newVarCoeff.set_width_chars(4);
            newVarCoeff.set_max_length(16);

            newVarCoeff.set_placeholder_text
            (
                GUIUtilityFuncs::GetCoeffAShortLabel(y, x)
            );
            newVarCoeff.set_tooltip_text
            (
                GUIUtilityFuncs::GetCoeffAFancyLabel(y, x)
            );

            // add a new plus and var descriptor labels
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

        // add a new equality mark label
        auto& newLEEquationMark = leEquationMarkLabels[y];
        newLEEquationMark = Gtk::Label();
        newLEEquationMark.set_text("=");
        newLEEquationMark.set_size_request(32, 42);
        eqMarksGrid.attach(newLEEquationMark, 0, y);

        // add a new free coefficient entry
        auto& newFreeCoeff = freeCoeffsEntries[y];
        newFreeCoeff = Gtk::Entry();
        newFreeCoeff.set_width_chars(6);
        newFreeCoeff.set_size_request(24, 42);
        newFreeCoeff.set_max_length(16);

        newFreeCoeff.set_placeholder_text
        (
            GUIUtilityFuncs::GetCoeffBShortLabel(y)
        );
        newFreeCoeff.set_tooltip_text
        (
            GUIUtilityFuncs::GetCoeffBFancyLabel(y)
        );

        freeCoeffsGrid.attach(newFreeCoeff, 0, y);
    }

    // Show SLE nice form above
    varsCoeffsGrid.show_all_children();
    eqMarksGrid.show_all_children();
    freeCoeffsGrid.show_all_children();

    varsCoeffsGrid.show();
    eqMarksGrid.show();
    freeCoeffsGrid.show();
}

void SLEConfigurator::removeSLEForm()
{
    auto sleInputDataSP = sleData.lock();
    auto& sleInputData = *sleInputDataSP;

    // don't do anything if there is no sle form
    if (! sleInputData.IsEquationsCountSetted())
    {
        return;
    }

    std::size_t eqsCount = sleInputData.GetEquationsCount().value();
    sleInputData.ClearData();

    varsCoeffsGrid.hide();
    eqMarksGrid.hide();
    freeCoeffsGrid.hide();

    for (std::ptrdiff_t y = eqsCount - 1; y >= 0; y--)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            varsCoeffsGrid.remove(varsCoeffsEntries.At(y, x));
            varsCoeffsGrid.remove(varsCoeffsLabels.At(y, x * 2));

            if (x != eqsCount - 1)
            {
                varsCoeffsGrid.remove(varsCoeffsLabels.At(y, x * 2 + 1));
            }
        }

        eqMarksGrid.remove_column(y);
        freeCoeffsGrid.remove_column(y);
    }

    varsCoeffsEntries = RTArray2D<Gtk::Entry>();
    varsCoeffsLabels  = RTArray2D<Gtk::Label>();

    leEquationMarkLabels = std::vector<Gtk::Label>{};

    freeCoeffsEntries = std::vector<Gtk::Entry>();
}

void SLEConfigurator::fillEmptyEntriesWithZeroes()
{
    constexpr auto formattedZero = "0";

    const auto sleInputDataSP = sleData.lock();
    const auto& sleInputData = *sleInputDataSP;

    if (! sleInputData.IsEquationsCountSetted())
    {
        return;
    }

    auto eqsCount = sleInputData.GetEquationsCount().value();

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

void SLEConfigurator::fillEmptyEntriesWithRandomNumbers()
{
    const auto sleInputDataSP = sleData.lock();
    const auto& sleInputData = *sleInputDataSP;

    if (! sleInputData.IsEquationsCountSetted())
    {
        return;
    }

    auto eqsCount = sleInputData.GetEquationsCount().value();

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto& currentEntryOfMatrixA = varsCoeffsEntries.At(y, x);
            const auto& origEntryInput = currentEntryOfMatrixA.get_text();

            if (origEntryInput == "")
            {
                currentEntryOfMatrixA.set_text(Convert::NumberToString(Math::FloorWithPrecision
                (
                    100 * GUIUtilityFuncs::UniformRandom(), 2
                )));
            }
        }

        auto& currentEntryOfVectorB = freeCoeffsEntries[y];
        const auto& origEntryInput = currentEntryOfVectorB.get_text();

        if (origEntryInput == "")
        {
            currentEntryOfVectorB.set_text(Convert::NumberToString(Math::FloorWithPrecision
            (
                100 * GUIUtilityFuncs::UniformRandom(), 2
            )));
        }
    }
}

void SLEConfigurator::doClearSLEForm()
{
const auto sleInputDataSP = sleData.lock();
    const auto& sleInputData = *sleInputDataSP;

    if (! sleInputData.IsEquationsCountSetted())
    {
        return;
    }

    auto eqsCount = sleInputData.GetEquationsCount().value();

    for (std::size_t y = 0; y < eqsCount; y++)
    {
        for (std::size_t x = 0; x < eqsCount; x++)
        {
            auto& currentEntryOfMatrixA = varsCoeffsEntries.At(y, x);

            currentEntryOfMatrixA.set_text("");
        }

        auto& currentEntryOfVectorB = freeCoeffsEntries[y];

        currentEntryOfVectorB.set_text("");
    }
}

SLESolvePanel::SLESolvePanel()
{
    set_label("Розв'язання СЛАР");
    set_size_request(480, 260);

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

    auto& comboBoxMethodRecords = ComboBoxMethodRecords::ComboBoxMethodRecordsField;

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
    // get the selected method
    auto comboBoxMethodIndex = comboBoxMethodsNames.get_active_id();

    if (comboBoxMethodIndex == "")
    {
        solvingStatus.set_text("Метод не встановлено");
        return;
    }

    // check whether the SLE is confirmed
    auto sleInputDataSP = sleInputData.lock();
    auto& sleInputData = *sleInputDataSP;

    if (! sleInputData.IsDataConfirmed())
    {
        solvingStatus.set_text("СЛАР ще не встановлена");
        return;
    }

    // get the essential information for the solving process
    auto eqsCount = sleInputData.GetEquationsCount().value();

    const auto& A = sleInputData.GetVariablesCoefficientsRef().value().get();
    const auto& B = sleInputData.GetFreeCoefficientsRef().value().get();

    auto sleSolveDataSP = sleSolveData.lock();
    auto& sleSolveData = *sleSolveDataSP;

    // zero determinant check
    if (GUIUtilityFuncs::IsDetCloseToZero(LinAlgUtility::Determinant(A)))
    {
        sleSolveOutput.lock()->ShowInvalidSolve();

        sleSolveData.SetSolvingStatus(SLESolvingStatus::SolvedFailful);
        solvingStatus.set_text("Детермінант матриці коеф. рівен 0");
        practicalTimeComplexity.set_text("");
    
        return;
    }

    // create a new chose solver
    auto solvingMethodP = SLESolverFactory::CreateNew
    (
        ComboBoxMethodRecords::ComboBoxMethodRecordsField
        [
           Convert::ToInteger(comboBoxMethodIndex).value()
        ]
        .GetSolvingMethodIndex()
    );

    auto& solvingMethod = *solvingMethodP;

    // start the solving process
    solvingMethod.SetEquationsCount(eqsCount);
    solvingMethod.SetVariablesCoefficients(A);
    solvingMethod.SetFreeCoefficients(B);

    solvingMethod.Solve();

    // if the solving is not successful
    if (! solvingMethod.IsSolvedSuccessfully().value())
    {
        sleSolveOutput.lock()->ShowInvalidSolve();

        sleSolveData.SetSolvingStatus(SLESolvingStatus::SolvedFailful);
        solvingStatus.set_text("СЛАР не можливо вирішити цим методом");
        practicalTimeComplexity.set_text("");
    
        return;
    }

    // set the solves to other widget
    auto X = solvingMethod.GetSolveOnce().value();

    sleSolveData.SetVarsSolve(std::move(X));
    sleSolveData.SetSolvingStatus(SLESolvingStatus::SolvedSuccessfully);

    solvingStatus.set_text
    (
        std::format
        (
              "СЛАР вирішено {}"
            , Time::GetCurrentFormalTime()
        )
    );
    practicalTimeComplexity.set_text
    (
        std::format
        (
              "СЛАР вирішено за {} ітерацій"
            , solvingMethod.GetAlgoItersCount().value()
        )
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

ApplicationWindow& GUISession::GetWindowRef()
{
    return appWin;
}
