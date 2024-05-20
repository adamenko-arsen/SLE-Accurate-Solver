#pragma once

#include "SLESolver.hpp"

#include <memory>
#include <optional>
#include <string>

enum LSESolvingMethodIndex
{
      LUP            = 0
    , GaussHoletskiy = 1
    , Rotation       = 2
};

std::unique_ptr<SLESolver> SLESolverFactoryProduce(LSESolvingMethodIndex solverIndex);

class ComboBoxMethodRecord
{
public:
    ComboBoxMethodRecord(LSESolvingMethodIndex solvingMethodIndex, auto&& methodName, auto&& methodPracticalItersComplexity)
    {
        this->solvingMethodIndex = solvingMethodIndex;

        this->methodName = std::forward<decltype(methodName)>(methodName);
        this->methodPracticalItersComplexity = std::forward<decltype(methodPracticalItersComplexity)>(methodPracticalItersComplexity);
    }
    LSESolvingMethodIndex GetSolvingMethodIndex() const noexcept;
    const std::string& GetMethodName() const;
    const std::string& GetMethodPracticalItersComplexity() const;

private:
    LSESolvingMethodIndex solvingMethodIndex;
    std::string methodName{"[не вказано]"};
    std::string methodPracticalItersComplexity{"невідомо"};
};

extern std::vector<ComboBoxMethodRecord> comboBoxMethodRecords;
