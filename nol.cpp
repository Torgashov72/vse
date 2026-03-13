#include <iostream>
#include <cmath>
using namespace std;

const float EPS = 1e-7;  // Порог для сравнения с нулём (защита от погрешностей)

int main() {
    setlocale(LC_ALL, "rus");

    int n;
    cout << "Введите размерность матрицы (количество уравнений):" << endl;
    cin >> n;

    if (n <= 0) {
        cout << "Ошибка: размерность должна быть положительным числом!" << endl;
        return 1;
    }

    float** a = new float* [n];
    for (int i = 0; i < n; i++)
        a[i] = new float[n + 1];

    // Ввод расширенной матрицы
    cout << "Введите расширенную матрицу системы:" << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n + 1; j++) {
            cout << "a[" << i << "][" << j << "] = ";
            cin >> a[i][j];
        }
    }

    // Прямой ход метода Гаусса
    for (int k = 0; k < n; k++) {
        // Поиск главного элемента (выбор ведущей строки)
        int maxRow = k;
        for (int i = k + 1; i < n; i++) {
            if (abs(a[i][k]) > abs(a[maxRow][k])) {
                maxRow = i;
            }
        }

        // Перестановка строк
        for (int j = k; j < n + 1; j++) {
            swap(a[k][j], a[maxRow][j]);
        }

        // 🔴 ПРОВЕРКА №1: Если ведущий элемент ≈ 0, продолжаем поиск ниже
        if (abs(a[k][k]) < EPS) {
            bool found = false;
            for (int i = k + 1; i < n; i++) {                if (abs(a[i][k]) > EPS) {
                    // Меняем местами строки, если нашли ненулевой элемент
                    for (int j = k; j < n + 1; j++) {
                        swap(a[k][j], a[i][j]);
                    }
                    found = true;
                    break;
                }
            }
            // Если так и не нашли ненулевой элемент в столбце
            if (!found) {
                continue; // Пропускаем этот столбец (возможно, бесконечно много решений)
            }
        }

        // Исключение переменных
        for (int i = k + 1; i < n; i++) {
            // 🔴 ПРОВЕРКА №2: Защита от деления на ноль при вычислении множителя
            if (abs(a[k][k]) < EPS) continue;
            
            float factor = a[i][k] / a[k][k];
            for (int j = k; j < n + 1; j++) {
                a[i][j] -= factor * a[k][j];
            }
        }
    }

    // 🔴 ПРОВЕРКА №3: Анализ совместности системы после прямого хода
    bool hasInfiniteSolutions = false;
    for (int i = 0; i < n; i++) {
        bool allZero = true;
        for (int j = 0; j < n; j++) {
            if (abs(a[i][j]) > EPS) {
                allZero = false;
                break;
            }
        }
        // Если все коэффициенты ≈ 0, но свободный член ≠ 0 → противоречие
        if (allZero && abs(a[i][n]) > EPS) {
            cout << "\n❌ Система несовместна: нет решений (0 = " << a[i][n] << ")" << endl;
            
            // Освобождаем память
            for (int i = 0; i < n; i++) delete[] a[i];
            delete[] a;
            return 1;
        }
        // Если вся строка нулевая (0 = 0) → бесконечно много решений
        if (allZero && abs(a[i][n]) < EPS) {
            hasInfiniteSolutions = true;
        }    }

    if (hasInfiniteSolutions) {
        cout << "\n⚠️  Система имеет бесконечно много решений (вырожденная матрица)." << endl;
        cout << "Данный код предназначен только для единственного решения." << endl;
        
        for (int i = 0; i < n; i++) delete[] a[i];
        delete[] a;
        return 1;
    }

    // Обратный ход
    float* x = new float[n];
    for (int i = n - 1; i >= 0; i--) {
        // 🔴 ПРОВЕРКА №4: Проверка диагонального элемента перед делением
        if (abs(a[i][i]) < EPS) {
            cout << "\n❌ Ошибка: нулевой делитель при обратном ходе (x[" << i << "])." << endl;
            cout << "Система, вероятно, вырождена." << endl;
            delete[] x;
            for (int i = 0; i < n; i++) delete[] a[i];
            delete[] a;
            return 1;
        }

        x[i] = a[i][n];
        for (int j = i + 1; j < n; j++) {
            x[i] -= a[i][j] * x[j];
        }
        x[i] /= a[i][i];
    }

    // Вывод результата
    cout << "\n✅ Решение системы:" << endl;
    for (int i = 0; i < n; i++) {
        cout << "x[" << i << "] = " << x[i] << endl;
    }

    // Освобождение памяти
    delete[] x;
    for (int i = 0; i < n; i++) delete[] a[i];
    delete[] a;

    return 0;
}
