from numpy import matrix
from copy import deepcopy

def ZeroMatrix(n):
    Z = [
        [0] * n for _ in range(n)
    ]
    return Z

def DiagMatrix(n):
    I = ZeroMatrix(n)
    for d in range(n):
        I[d][d] = 1

    return I

def MaxDiag(A, f):
    m = abs(A[f][f])
    i = f

    for j in range(f + 1, len(A)):
        if (pm := abs(A[j][f])) > m:
            m = pm
            i = j

    return i

def Sum(f, t, fn):
    s = 0
    for i in range(f, t + 1):
        s += fn(i)

    return s

def LUPDec(A):
    n = len(A)
    A = deepcopy(A)

    P = list(range(n))

    for j in range(n):
        md = MaxDiag(A, j)

        for r in range(n):
            A[j][r], A[md][r] = A[md][r], A[j][r]

        P[j], P[md] = P[md], P[j]

        for i in range(j, n):
            A[i][j] -= Sum(0, j - 1, (
                lambda k: A[i][k] * A[k][j]
            ))

            if i >= j + 1:
                A[j][i] -= Sum(0, j - 1, (
                    lambda k: A[j][k] * A[k][i]
                ))
                A[j][i] /= A[j][j]

    L = ZeroMatrix(n)
    U = DiagMatrix(n)

    for j in range(n):
        for i in range(j + 1):
            L[j][i] = A[j][i]

    for j in range(n):
        for i in range(j + 1, n):
            U[j][i] = A[j][i]

    return L, U, P

def LUPSolveY(L, P, B):
    n = len(B)
    Y = [0] * n

    for i in range(n):
        Y[i] = (
            B[P[i]] - Sum(0, i - 1, (
                lambda k: L[i][k] * Y[k]
            ))
        ) / L[i][i]

    return Y

def LUPSolveX(U, Y):
    n = len(Y)
    X = [0] * n

    for i in range(n - 1, 0 - 1, -1):
        X[i] = Y[i] - Sum(i + 1, n - 1, (
            lambda k: U[i][k] * X[k]
        ))

    return X

A = [
    [0, 8, 0],
    [3, 0, 4],
    [2, 6, 0]
]
B = [28, 40, 20]

LUP = LUPDec(A)
Y = LUPSolveY(LUP[0], LUP[2], B)
X = LUPSolveX(LUP[1], Y)

print(matrix(LUP[0]))
print()
print(matrix(LUP[1]))
print()
print(matrix(LUP[2]).T)
print()
print()
print(matrix(Y).T)
print()
print(matrix(X).T)
print()
print()
print(matrix(A) * matrix(X).T - matrix(B).T)
