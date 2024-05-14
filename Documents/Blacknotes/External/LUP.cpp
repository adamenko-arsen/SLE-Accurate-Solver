#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

// Function to perform LU decomposition
void luDecomposition(const vector<vector<double>>& A, vector<vector<double>>& L, vector<vector<double>>& U, vector<int>& P) {
    int n = A.size();

    // Initialize permutation matrix P
    for (int i = 0; i < n; i++) {
        P[i] = i;
    }

    // Initializing L and U matrices
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i <= j) {
                U[i][j] = A[i][j];
                if (i == j)
                    L[i][j] = 1;
                else
                    L[i][j] = 0;
            }
            else {
                L[i][j] = A[i][j];
                U[i][j] = 0;
            }
        }
    }

    // Performing LU decomposition with partial pivoting
    for (int k = 0; k < n; k++) {
        double maxVal = 0;
        int maxIndex = k;
        for (int i = k; i < n; i++) {
            if (abs(U[i][k]) > maxVal) {
                maxVal = abs(U[i][k]);
                maxIndex = i;
            }
        }

        // Swap rows in U matrix
        if (maxIndex != k) {
            swap(U[k], U[maxIndex]);
            swap(P[k], P[maxIndex]);
        }

        for (int i = k + 1; i < n; i++) {
            L[i][k] = U[i][k] / U[k][k];
            for (int j = k; j < n; j++) {
                U[i][j] -= L[i][k] * U[k][j];
            }
        }
    }
}

// Function to solve the system of linear equations using LUP decomposition
vector<double> solveEquations(const vector<vector<double>>& A, const vector<double>& b) {
    int n = A.size();

    // Initialize matrices L, U, and P
    vector<vector<double>> L(n, vector<double>(n));
    vector<vector<double>> U(n, vector<double>(n));
    vector<int> P(n);

    // Perform LU decomposition
    luDecomposition(A, L, U, P);

    // Solve Ly = Pb
    vector<double> Pb(n);
    for (int i = 0; i < n; i++) {
        Pb[i] = b[P[i]];
        for (int j = 0; j < i; j++) {
            Pb[i] -= L[i][j] * Pb[j];
        }
    }

    // Solve Ux = y
    vector<double> x(n);
    for (int i = n - 1; i >= 0; i--) {
        x[i] = Pb[i];
        for (int j = i + 1; j < n; j++) {
            x[i] -= U[i][j] * x[j];
        }
        x[i] /= U[i][i];
    }

    return x;
}

int main() {
    // Example coefficients matrix and free coefficient vector
    vector<vector<double>> A = {{2, -1, 1},
                                 {-3, 3, 2},
                                 {1, 1, 4}};
    vector<double> b = {1, 4, 2};

    // Solve equations
    vector<double> solution = solveEquations(A, b);

    // Print solution
    cout << "Solution:" << endl;
    for (int i = 0; i < solution.size(); i++) {
        cout << "x" << i + 1 << " = " << solution[i] << endl;
    }

    return 0;
}
