#pragma once

#include "SLESolver.hpp"

#include <memory>
#include <optional>
#include <string>

enum SLESolvingMethodIndex
{
      LUP            = 0
    , GaussHoletskiy = 1
    , Rotation       = 2
};

std::unique_ptr<SLESolver> SLESolverFactoryProduce(SLESolvingMethodIndex solverIndex);

class ComboBoxMethodRecord
{
public:
    ComboBoxMethodRecord(SLESolvingMethodIndex solvingMethodIndex, auto&& methodName, auto&& methodPracticalItersComplexity)
    {
        this->solvingMethodIndex = solvingMethodIndex;

        this->methodName = std::forward<decltype(methodName)>(methodName);
        this->methodPracticalItersComplexity = std::forward<decltype(methodPracticalItersComplexity)>(methodPracticalItersComplexity);
    }

    SLESolvingMethodIndex GetSolvingMethodIndex() const noexcept;

    const std::string& GetMethodName() const;
    const std::string& GetMethodPracticalItersComplexity() const;

private:
    SLESolvingMethodIndex solvingMethodIndex;

    std::string methodName{"[не вказано]"};
    std::string methodPracticalItersComplexity{"невідомо"};
};

extern std::vector<ComboBoxMethodRecord> comboBoxMethodRecords;
