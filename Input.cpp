#include <iostream>
#include <fstream>
#include <iomanip>
#include <windows.h>

using namespace std;

double** allocateMatrix(int n) {
    double** m = new double* [n];
    for (int i = 0; i < n; ++i) {
        m[i] = new double[n];
    }
    return m;
}

void freeMatrix(double** m, int n) {
    for (int i = 0; i < n; ++i) {
        delete[] m[i];
    }
    delete[] m;
}

void readMatrix(ifstream& fin, double** m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            fin >> m[i][j];
        }
    }
}

void printMatrix(double** m, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cout << fixed << setprecision(2) << m[i][j] << "\t";
        }
        cout << endl;
    }
}

double calculateDeterminant(double** A, int n) {
    const double EPS = 1e-10;

    if (n <= 0 || n > 1000) {
        cerr << "Error: Invalid matrix size" << endl;
        return 0.0;
    }

    double** L = allocateMatrix(n);
    double** U = allocateMatrix(n);
    double det = 1.0;
    bool singular = false;

    try {
        // Инициализация
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                L[i][j] = 0.0;
                U[i][j] = 0.0;
            }
            U[i][i] = 1.0;
        }

        // LU-разложение
        for (int j = 0; j < n; ++j) {
            for (int i = j; i < n; ++i) {
                double sum = 0.0;
                for (int k = 0; k < j; ++k) {
                    sum += L[i][k] * U[k][j];
                }
                L[i][j] = A[i][j] - sum;
            }

            // Проверка на малый pivot
            if (abs(L[j][j]) < EPS) {
                singular = true;
                break;
            }

            for (int i = j + 1; i < n; ++i) {
                double sum = 0.0;
                for (int k = 0; k < j; ++k) {
                    sum += L[j][k] * U[k][i];
                }
                U[j][i] = (A[j][i] - sum) / L[j][j];
            }
        }

        if (singular) {
            det = 0.0;
        }
        else {
            for (int i = 0; i < n; ++i) {
                det *= L[i][i];
            }
            // Проверка на переполнение
            if (isinf(det) || isnan(det)) {
                cerr << "Warning: Determinant overflow" << endl;
                det = 0.0;
            }
        }
    }
    catch (...) {
        cerr << "Error: Exception during calculation" << endl;
        det = 0.0;
    }

    freeMatrix(L, n);
    freeMatrix(U, n);
    return det;
}

int main() {


    ifstream fin("C:/temp/input.txt");  // ← Изменённая строка
    if (!fin.is_open()) {
        cerr << "Error: Cannot open C:/temp/input.txt" << endl;
        cerr << "Make sure the file exists!" << endl;
        system("pause");
        return 1;
    }

    int n;
    int exampleCount = 1;
    while (fin >> n) {
        if (n <= 0) break;

        double** A = allocateMatrix(n);
        readMatrix(fin, A, n);

        cout << "Example " << exampleCount++ << " (Size " << n << "x" << n << "):" << endl;
        printMatrix(A, n);

        double det = calculateDeterminant(A, n);
        cout << "Determinant: " << fixed << setprecision(6) << det << endl;
        cout << "-----------------------------------" << endl;

        freeMatrix(A, n);
    }

    fin.close();
    system("pause");
    return 0;
}