#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

using namespace std;

// Выделение памяти под матрицу
double** allocateMatrix(int n) {
    double** mat = new double* [n];
    for (int i = 0; i < n; ++i) {
        mat[i] = new double[n];
    }
    return mat;
}

// Выделение памяти под вектор
double* allocateVector(int n) {
    return new double[n];
}

// Освобождение памяти матрицы
void freeMatrix(double** mat, int n) {
    for (int i = 0; i < n; ++i) {
        delete[] mat[i];
    }
    delete[] mat;
}

// Освобождение памяти вектора
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

// Вычисление вектора невязки F(x) для системы:
// F_i(x) = sum(A_ij * x_j) + sum(B_ij * x_j^2) - f_i
void calculateF(int n, double** A, double** B, double* f, double* x, double* F) {
    for (int i = 0; i < n; ++i) {
        double sumLinear = 0.0;
        double sumNonLinear = 0.0;
        for (int j = 0; j < n; ++j) {
            sumLinear += A[i][j] * x[j];
            sumNonLinear += B[i][j] * x[j] * x[j];
        }
        F[i] = sumLinear + sumNonLinear - f[i];
    }
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
    double tau, epsilon;
    int maxIter;

    // Чтение параметров
    // Формат: N tau epsilon maxIter
    inFile >> n >> tau >> epsilon >> maxIter;

    // Выделение памяти
    double** A = allocateMatrix(n); // Линейные коэффициенты
    double** B = allocateMatrix(n); // Нелинейные коэффициенты (при x^2)
    double* f = allocateVector(n);  // Правая часть
    double* x = allocateVector(n);  // Текущее приближение
    double* x_prev = allocateVector(n); // Предыдущее приближение
    double* F_vec = allocateVector(n); // Вектор значений функций

    // Чтение матрицы A (линейная часть)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            inFile >> A[i][j];
        }
    }

    // Чтение матрицы B (нелинейная часть)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            inFile >> B[i][j];
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

    outFile << "Параметры метода релаксации (нелинейный случай):" << endl;
    outFile << "Размерность: " << n << endl;
    outFile << "Tau (параметр релаксации): " << tau << endl;
    outFile << "Точность (epsilon): " << epsilon << endl;
    outFile << "Макс. итераций: " << maxIter << endl;
    outFile << "----------------------------------------" << endl;

    int iter = 0;
    double norm = epsilon + 1.0;
    bool converged = false;
    bool diverged = false;

    // Основной цикл итерационного процесса
    while (iter < maxIter && norm >= epsilon) {
        // Сохраняем предыдущее приближение
        for (int i = 0; i < n; ++i) {
            x_prev[i] = x[i];
        }

        // Вычисляем F(x_prev)
        calculateF(n, A, B, f, x_prev, F_vec);

        // Применяем метод релаксации: x_new = x_old - tau * F(x_old)
        for (int i = 0; i < n; ++i) {
            x[i] = x_prev[i] - tau * F_vec[i];
        }

        // Вычисление нормы разности
        norm = computeNorm(x_prev, x, n);
        iter++;

        // Проверка на расходимость (защита от бесконечного роста)
        if (norm > 1e10) {
            diverged = true;
            break;
        }
    }

    if (norm < epsilon) {
        converged = true;
    }

    // Вывод результатов
    outFile << "Результаты:" << endl;
    if (diverged) {
        outFile << "Метод расходится (погрешность слишком велика)." << endl;
        outFile << "Рекомендуется уменьшить параметр tau." << endl;
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
    for (int i = 0; i < n; ++i) {
        outFile << "x[" << i << "] = " << x[i] << endl;
    }

    // Проверка невязки (подстановка решения в F(x))
    outFile << "----------------------------------------" << endl;
    outFile << "Проверка невязки " << endl;
    calculateF(n, A, B, f, x, F_vec);
    for (int i = 0; i < n; ++i) {
        outFile << "F[" << i << "] = " << F_vec[i] << endl;
    }

    // Освобождение памяти
    freeMatrix(A, n);
    freeMatrix(B, n);
    freeVector(f);
    freeVector(x);
    freeVector(x_prev);
    freeVector(F_vec);

    inFile.close();
    outFile.close();

    cout << "Расчет завершен. Результаты сохранены в output.txt" << endl;

    return 0;
}
