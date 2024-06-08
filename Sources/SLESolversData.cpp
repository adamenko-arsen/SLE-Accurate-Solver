#include "SLESolversData.hpp"

#include "SLESolvers/LUPSolver.hpp"
#include "SLESolvers/GaussHoletskiySolver.hpp"
#include "SLESolvers/RotationSolver.hpp"

std::unique_ptr<SLESolver> SLESolverFactory::CreateNew(SLESolvingMethodIndex solverIndex)
{
    using enum SLESolvingMethodIndex;

    std::unique_ptr<SLESolver> abstractSolver{};

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

SLESolvingMethodIndex ComboBoxMethodRecord::GetSolvingMethodIndex() const noexcept
{
    return solvingMethodIndex;
}

const std::string& ComboBoxMethodRecord::GetMethodName() const
{
    return methodName;
}
const std::string& ComboBoxMethodRecord::GetMethodPracticalItersComplexity() const
{
    return methodPracticalItersComplexity;
}

std::vector<ComboBoxMethodRecord> ComboBoxMethodRecords::ComboBoxMethodRecordsField =
{
      ComboBoxMethodRecord(SLESolvingMethodIndex::LUP            , "LUP-метод"              , "1/3*n^3 + 7/2*n^2 + 7/6*n")
    , ComboBoxMethodRecord(SLESolvingMethodIndex::Rotation       , "Метод обертання"        , "1/3*n^3 + 7/2*n^2 + 1/6*n - 2")
    , ComboBoxMethodRecord(SLESolvingMethodIndex::GaussHoletskiy , "Метод Гауса-Холецького (квадратного кореня)" , "1/6*n^3 + 5/2*n^2 - 2/3*n")
};
