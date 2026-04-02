#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

const int MAX_N = 100; // Максимальное количество узлов

// Вспомогательная функция: заменяет точку на запятую в числе
string toComma(double value, int precision = 6) {
    stringstream ss;
    ss << fixed << setprecision(precision) << value;
    string result = ss.str();

    // Находим последнюю точку (десятичный разделитель) и меняем на запятую
    size_t pos = result.find_last_of('.');
    if (pos != string::npos) {
        result[pos] = ',';
    }
    return result;
}

int main() {
    setlocale(LC_ALL, "rus");
    // 1. Чтение данных из input.txt
    ifstream infile("C:/Users/Эдуард/source/repos/gauss_interpolation.cpp/input.txt");
    if (!infile.is_open()) {
        cerr << "Ошибка: Не удалось открыть файл input.txt\n";
        return 1;
    }

    int N;
    infile >> N;
    if (N < 2 || N > MAX_N) {
        cerr << "Ошибка: N должно быть от 2 до 100" << MAX_N << "\n";
        return 1;
    }

    double x[MAX_N], y[MAX_N];
    for (int i = 0; i < N; ++i) {
        infile >> x[i] >> y[i];
    }
    infile.close();

    // 2. Построение таблицы конечных разностей
    double diff[MAX_N][MAX_N];
    for (int i = 0; i < N; ++i) diff[i][0] = y[i];

    for (int j = 1; j < N; ++j) {
        for (int i = 0; i < N - j; ++i) {
            diff[i][j] = diff[i + 1][j - 1] - diff[i][j - 1];
        }
    }

    // Проверка шага
    double h = x[1] - x[0];
    if (h == 0.0) {
        cerr << "Ошибка: Шаг таблицы равен нулю.\n";
        return 1;
    }
    for (int i = 1; i < N - 1; ++i) {
        if (abs((x[i + 1] - x[i]) - h) > 1e-9) {
            cerr << "Внимание: Узлы не равноотстоящие. Погрешность может возрасти.\n";
            break;
        }
    }

    // 3. Подготовка вывода для Excel (с запятыми!)
    ofstream outfile("output.txt");
    if (!outfile.is_open()) {
        cerr << "Ошибка: Не удалось создать output.txt\n";
        return 1;
    }

    // Заголовки с точкой с запятой как разделителем столбцов (для надёжности в RU-Excel)
    outfile << "X_interp\tY_interp\n";

    const int GRAPH_POINTS = 100;
    double step = (x[N - 1] - x[0]) / GRAPH_POINTS;

    // 4. Вычисление интерполированных значений
    for (int s = 0; s <= GRAPH_POINTS; ++s) {
        double xq = x[0] + s * step;

        // Поиск ближайшего центрального узла
        int k = 0;
        double min_dist = abs(xq - x[0]);
        for (int i = 1; i < N; ++i) {
            double d = abs(xq - x[i]);
            if (d < min_dist) {
                min_dist = d;
                k = i;
            }
        }
        // Защита от выхода за границы таблицы разностей
        if (k == 0) k = 1;
        if (k == N - 1) k = N - 2;

        double q = (xq - x[k]) / h;
        double res = y[k];
        double coef = 1.0;

        // Рекуррентное вычисление коэффициентов формулы Гаусса
        for (int m = 1; m < N; ++m) {
            if (m == 1) {
                coef = q;
            }
            else if (m % 2 == 0) {
                coef *= (q - m / 2.0) / m;
            }
            else {
                coef *= (q + (m - 1) / 2.0) / m;
            }

            int idx = k - (m + 1) / 2;
            if (idx < 0 || idx + m >= N) break;
            res += coef * diff[idx][m];
        }

        // Вывод с запятыми и точкой с запятой как разделителем столбцов
        outfile << toComma(xq) << "\t" << toComma(res) << "\n";
    }

    outfile.close();
    cout << "Готово";

    return 0;
}