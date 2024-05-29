#pragma once

#include "Containers/Matrix.hpp"

struct LinAlgUtility final
{
    static double Determinant(const Matrix& squareMatrix);

private:
    static bool detIsCloseToZero(double x);
};
