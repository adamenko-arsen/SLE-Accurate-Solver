#include "LinAlgUtility.hpp"

#include <cmath>

#include <iostream>

double LinAlgUtility::Determinant(const Matrix& oldM)
{
    if (! oldM.IsSquare())
    {
        return 0;
    }

    const auto n = oldM.TryGetEdgeSize();

    auto m = oldM;

    for (std::size_t fixedCol = 0; fixedCol < n - 1; fixedCol++)
    {
        if (detIsCloseToZero(m.At(fixedCol, fixedCol)))
        {
            std::size_t maxIndex = fixedCol;
            double maxValue = std::fabs(m.At(fixedCol, fixedCol));
        
            for (std::size_t probeCol = fixedCol + 1; probeCol < n; probeCol++)
            {
                auto mayNewMax = std::fabs(m.At(probeCol, fixedCol));

                if (mayNewMax > maxValue)
                {
                    maxIndex = probeCol;
                    maxValue = mayNewMax;
                }
            }

            if (detIsCloseToZero(maxValue))
            {
                return 0;
            }

            for (std::size_t swapRow = 0; swapRow < n; swapRow++)
            {
                std::swap(m.At(fixedCol, swapRow), m.At(maxIndex, swapRow));
            }
        }

        for (std::size_t lowerCol = fixedCol + 1; lowerCol < n; lowerCol++)
        {
            double k = m.At(lowerCol, fixedCol) / m.At(fixedCol, fixedCol);

            for (std::size_t redRow = fixedCol; redRow < n; redRow++)
            {
                m.At(lowerCol, redRow) -= k * m.At(fixedCol, redRow);
            }
        }
    }

    double det = 1;

    for (std::size_t diag = 0; diag < n; diag++)
    {
        auto diagElem = m.At(diag, diag);

        det *= diagElem;
    }

    return det;
}

bool LinAlgUtility::detIsCloseToZero(double number)
{
    return std::fabs(number) < 10e-9;
}
