#include "LSESolversData.hpp"

#include "LUPSolver.hpp"
#include "GaussHoletskiySolver.hpp"
#include "RotationSolver.hpp"

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

std::vector<ComboBoxMethodRecord> comboBoxMethodRecords =
{
      {LSESolvingMethodIndex::LUP            , "LUP-метод"              , "0.48n^3 + 0.9n + 6n"}
    , {LSESolvingMethodIndex::GaussHoletskiy , "Метод Гауса-Холецького" , "0.26n^3 + 1.6n^2 + 4n"}
    , {LSESolvingMethodIndex::Rotation       , "Метод обертання"        , "0.42n^3 + 1.3n^2 + 4n"}
};
