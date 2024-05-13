#pragma once

#include "LSESolver.hpp"

#include <memory>
#include <optional>
#include <string>

enum LSESolvingMethodIndex
{
      LUP            = 0
    , GaussHoletskiy = 1
    , Rotation       = 2
};

std::unique_ptr<LSESolver> LSESolverFactoryProduce(LSESolvingMethodIndex solverIndex);

struct ComboBoxMethodRecord
{
    LSESolvingMethodIndex SolvingMethodIndex;
    std::string MethodName{"[не вказано]"};
    std::string MethodPracticalItersComplexity{"невідомо"};
};

extern std::vector<ComboBoxMethodRecord> comboBoxMethodRecords;
