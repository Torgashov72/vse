#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

using namespace std;

// Функция для выделения памяти под матрицу
double** allocateMatrix(int n) {
    double** mat = new double* [n];
    for (int i = 0; i < n; ++i) {
        mat[i] = new double[n];
    }
    return mat;
}

// Функция для выделения памяти под вектор
double* allocateVector(int n) {
    return new double[n];
}

// Функция для освобождения памяти матрицы
void freeMatrix(double** mat, int n) {
    for (int i = 0; i < n; ++i) {
        delete[] mat[i];
    }
    delete[] mat;
}

// Функция для освобождения памяти вектора
void freeVector(double* vec) {
    delete[] vec;
}

// Вычисление нормы разности векторов (max norm)
double computeNorm(double* x_old, double* x_new, int n) {
    double maxDiff = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = fabs(x_new[i] - x_old[i]);
        if (diff > maxDiff) {
            maxDiff = diff;
        }
    }
    return maxDiff;
}

int main() {
    setlocale(LC_ALL, "Russian");

    ifstream inFile("input.txt");
    ofstream outFile("output.txt");

    if (!inFile.is_open()) {
        cout << "Ошибка: Не удалось открыть файл input.txt" << endl;
        return 1;
    }

    if (!outFile.is_open()) {
        cout << "Ошибка: Не удалось создать файл output.txt" << endl;
        inFile.close();
        return 1;
    }

    int n;
    double omega, epsilon;
    int maxIter;

    // Чтение параметров
    inFile >> n;
    inFile >> omega >> epsilon >> maxIter;

    // Выделение памяти
    double** A = allocateMatrix(n);
    double* f = allocateVector(n);
    double* x = allocateVector(n);
    double* x_prev = allocateVector(n);

    // Чтение матрицы A
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            inFile >> A[i][j];
        }
    }

    // Чтение вектора f
    for (int i = 0; i < n; ++i) {
        inFile >> f[i];
    }

    // Инициализация начального приближения нулями
    for (int i = 0; i < n; ++i) {
        x[i] = 0.0;
        x_prev[i] = 0.0;
    }

    outFile << "Параметры метода:" << endl;
    outFile << "Размерность: " << n << endl;
    outFile << "Omega (параметр релаксации): " << omega << endl;
    outFile << "Точность (epsilon): " << epsilon << endl;
    outFile << "Макс. итераций: " << maxIter << endl;
    outFile << "----------------------------------------" << endl;

    // --- ПРОВЕРКА ДИАГОНАЛИ ПЕРЕД ЦИКЛОМ ---
    // Так как матрица A не меняется, достаточно проверить один раз
    bool diagonalError = false;
    for (int i = 0; i < n; ++i) {
        if (fabs(A[i][i]) < 1e-12) {
            outFile << "Ошибка: Нулевой элемент на главной диагонали в строке " << i << endl;
            diagonalError = true;
            break; // Выходим из цикла проверки
        }
    }

    int iter = 0;
    double norm = epsilon + 1.0;
    bool converged = false;

    // Запускаем итерации только если нет ошибок диагонали
    if (!diagonalError) {
        while (iter < maxIter && norm >= epsilon) {
            // Сохраняем предыдущее приближение
            for (int i = 0; i < n; ++i) {
                x_prev[i] = x[i];
            }

            for (int i = 0; i < n; ++i) {
                // Здесь проверка уже не нужна, так как мы проверили A[i][i] выше

                double sum1 = 0.0;
                for (int j = 0; j < i; ++j) {
                    sum1 += A[i][j] * x[j];
                }

                double sum2 = 0.0;
                for (int j = i + 1; j < n; ++j) {
                    sum2 += A[i][j] * x_prev[j];
                }

                // Формула верхней релаксации
                x[i] = (1.0 - omega) * x_prev[i] + (omega / A[i][i]) * (f[i] - sum1 - sum2);
            }

            norm = computeNorm(x_prev, x, n);
            iter++;
        }

        if (norm < epsilon) {
            converged = true;
        }
    }

    // Вывод результатов
    outFile << "Результаты:" << endl;
    if (diagonalError) {
        outFile << "Вычисления прерваны из-за ошибки матрицы." << endl;
    }
    else if (converged) {
        outFile << "Метод сошелся за " << iter << " итераций." << endl;
        outFile << "Достигнутая точность: " << norm << endl;
    }
    else {
        outFile << "Метод не сошелся за " << maxIter << " итераций." << endl;
        outFile << "Последняя норма невязки: " << norm << endl;
    }

    outFile << "----------------------------------------" << endl;
    outFile << "Вектор решения X:" << endl;
    outFile << fixed << setprecision(10);

    // Выводим решение только если не было ошибки диагонали
    if (!diagonalError) {
        for (int i = 0; i < n; ++i) {
            outFile << "x[" << i << "] = " << x[i] << endl;
        }

        // Проверка невязки
        outFile << "----------------------------------------" << endl;
        outFile << "Проверка невязки (Ax - f):" << endl;
        double* residual = allocateVector(n);
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += A[i][j] * x[j];
            }
            residual[i] = sum - f[i];
            outFile << "R[" << i << "] = " << residual[i] << endl;
        }
        freeVector(residual);
    }

    // Освобождение памяти (вместо метки cleanup)
    freeMatrix(A, n);
    freeVector(f);
    freeVector(x);
    freeVector(x_prev);

    inFile.close();
    outFile.close();

    cout << "Расчет завершен. Результаты сохранены в output.txt" << endl;

    // Возвращаем код ошибки, если была проблема с диагональю
    return diagonalError ? 1 : 0;
}
