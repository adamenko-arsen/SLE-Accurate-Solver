from numpy import matrix

def summ(f, t, fn):
    s = 0
    for i in range(f, t + 1):
        s += fn(i)

    return s

def CholeskyDec(A):
    class LDL:
        def __init__(self, *, L, D):
            self.L = L
            self.D = D

    n = len(A)

    L = [
        [0] * n for _ in range(n)
    ]
    D = [
        [0] * n for _ in range(n)

    ]
    for j in range(n):
        L[j][j] = 1

        D[j][j] = A[j][j] - summ(0, j - 1, (
            lambda k: L[j][k]**2 * D[k][k]
        ))

        for i in range(j + 1, n):
            L[i][j] = (
                A[i][j] - summ(0, j - 1, (
                    lambda k: L[i][k] * D[k][k] * L[j][k]
                ))
            ) / (D[j][j])

    return LDL(L = L, D = D)

def CholeskySolveY(L, B):
    n = len(B)

    Y = [0] * n

    for i in range(n):
        Y[i] = B[i] - summ(0, i - 1, (
            lambda j: L[i][j] * Y[j]
        ))

    return Y

def CholeskySolveZ(D, Y):
    n = len(Y)

    Z = [0] * n

    for i in range(n):
        Z[i] = Y[i] / D[i][i]

    return Z

def CholeskySolveX(L, Z):
    n = len(Z)

    X = [0] * n

    for i in range(n - 1, 0 - 1, -1):
        X[i] = Z[i] - summ(i + 1, n - 1, (
            lambda j: L[j][i] * X[j]
        ))

    return X

A = [
    [ 9,  -2,  -6],
    [-2,  16, -12],
    [-6, -12,  18]
]
B = [-13, -6, 24]

LDL = CholeskyDec(A)

Y = CholeskySolveY(LDL.L, B)
Z = CholeskySolveZ(LDL.D, Y)
X = CholeskySolveX(LDL.L, Z)

print(LDL.L)
print()
print(matrix(Y).T)
print()
print(matrix(X).T)
print()
print(matrix(A) * matrix(X).T)
