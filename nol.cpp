#include <iostream>
#include <cmath>
using namespace std;
const float eps = 1e-7;
int main() {
    setlocale(LC_ALL, "rus");

    int n;
    cout << "Введите размерность матрицы " << endl;
    cin >> n;

    float** a;
    a = new float* [n];
    for (int i = 0; i < n; i++)
        a[i] = new float[n + 1];

   
    cout << "Введите расширенную матрицу системы" << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n + 1; j++) {
            cout << "a[" << i << "][" << j << "] = ";
            cin >> a[i][j];
        }
    }

    // Прямой ход 
    for (int k = 0; k < n; k++) {
        int maxRow = k;
        for (int i = k + 1; i < n; i++) {
            if (abs(a[i][k]) > abs(a[maxRow][k])) {
                maxRow = i;
            }
        }

        // Перестановка строк
        for (int j = k; j < n + 1; j++) {
            float temp = a[k][j];
            a[k][j] = a[maxRow][j];
            a[maxRow][j] = temp;
        }

        if (abs(a[k][k]) < eps)
        {
            cout << "Ошибка деление на ноль" << endl;
            return 1;
        }

        // Нормализация и исключение
        for (int i = k + 1; i < n; i++) {
            float factor = a[i][k] / a[k][k];
            for (int j = k; j < n + 1; j++) {
                a[i][j] -= factor * a[k][j];
            }
        }
    }

    // Обратный ход
    float* x = new float[n];
    for (int i = n - 1; i >= 0; i--) {
        x[i] = a[i][n];
        for (int j = i + 1; j < n; j++) {
            x[i] -= a[i][j] * x[j];
        }
        x[i] /= a[i][i];
    }

    // Вывод результата
    cout << "\nРешение системы" << endl;
    for (int i = 0; i < n; i++) {
        cout << "x[" << i << "] = " << x[i] << endl;
    }
}
