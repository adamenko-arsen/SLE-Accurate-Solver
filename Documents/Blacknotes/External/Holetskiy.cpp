#include <iostream>
#include <vector>

using namespace std;

// Perform Gauss-Holetskiy elimination
vector<double> gaussHoletskiy(const vector<vector<double>>& A, const vector<double>& b) {
    int n = A.size();

    // Augment the coefficient matrix A with the constant vector b
    vector<vector<double>> augmented(n, vector<double>(n + 1));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            augmented[i][j] = A[i][j];
        }
        augmented[i][n] = b[i];
    }

    // Forward elimination
    for (int i = 0; i < n; ++i) {
        // Make the diagonal element 1
        double divisor = augmented[i][i];
        for (int j = i; j <= n; ++j) {
            augmented[i][j] /= divisor;
        }

        // Make the other elements in the current column zero
        for (int k = 0; k < n; ++k) {
            if (k != i) {
                double factor = augmented[k][i];
                for (int j = i; j <= n; ++j) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    // Extract solution
    vector<double> solution(n);
    for (int i = 0; i < n; ++i) {
        solution[i] = augmented[i][n];
    }

    return solution;
}

int main() {
    // Example coefficients matrix and free coefficient vector
    vector<vector<double>> A = {{2, -1, 1},
                                 {-3, 3, 2},
                                 {1, 1, 4}};
    vector<double> b = {1, 4, 2};

    // Solve equations
    vector<double> solution = gaussHoletskiy(A, b);

    // Print solution
    cout << "Solution:" << endl;
    for (int i = 0; i < solution.size(); i++) {
        cout << "x" << i + 1 << " = " << solution[i] << endl;
    }

    return 0;
}
