#!/usr/bin/env python

from numpy import matrix
from math import sqrt
from copy import deepcopy

def Solve(A, B):
    OA = A
    A = deepcopy(A)
    n = len(B)

    for i in range(n):
        A[i].append(B[i])

    for i in range(n - 1):
        for j in range(i + 1, n):
            b = A[j][i]
            a = A[i][i]
            c = a / sqrt(a**2 + b**2)
            s = b / sqrt(a**2 + b**2)
            
            print(b, a, c, s)
            print(-s * A[0][0] + c * A[1][0])

            for k in range(i, n + 1):
                t = A[i][k]
                A[i][k] = c * A[i][k] + s * A[j][k]
                A[j][k] = -s * t + c * A[j][k]

    X = [0] * n

    for i in range(n - 1, 0 - 1, -1):
        s = 0

        for j in range(i + 1, n):
            s += A[i][j] * X[j]

        X[i] = (A[i][n] - s) / A[i][i]

    return X

#A = [
#    [2, 4, 1, 2],
#    [2, 3, 7, 5],
#    [8, 3, 9, 0],
#    [8, 3, 2, 8]
#]
#B = [1, 2, 3, 4]

A = [
    [1, 2],
    [4, 3]
]
B = [5, 6]

print(Solve(A, B))
