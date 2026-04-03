#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

const int MAX_N = 100;

// Функция для замены точки на запятую (для Excel)
string toComma(double value, int precision = 6) {
    stringstream ss;
    ss << fixed << setprecision(precision) << value;
    string result = ss.str();
    size_t pos = result.find_last_of('.');
    if (pos != string::npos) result[pos] = ',';
    return result;
}

int main() {
    locale::global(locale::classic());

    // 1. Чтение данных
    ifstream infile("input.txt");
    if (!infile.is_open()) {
        cerr << "Ошибка: Нет файла input.txt\n";
        return 1;
    }

    int N;
    infile >> N;
    if (N < 2 || N > MAX_N) {
        cerr << "Ошибка: N от 2 до " << MAX_N << "\n";
        return 1;
    }

    double x[MAX_N], y[MAX_N];
    for (int i = 0; i < N; ++i) infile >> x[i] >> y[i];
    infile.close();

    // 2. Таблица конечных разностей
    double diff[MAX_N][MAX_N];
    for (int i = 0; i < N; ++i) diff[i][0] = y[i];

    for (int j = 1; j < N; ++j) {
        for (int i = 0; i < N - j; ++i) {
            diff[i][j] = diff[i + 1][j - 1] - diff[i][j - 1];
        }
    }

    double h = x[1] - x[0];

    // 3. Вывод в файл
    ofstream outfile("output.txt");
    outfile << fixed << setprecision(6);
    outfile << "X_interp;Y_interp\n";

    const int GRAPH_POINTS = 100;
    double step = (x[N - 1] - x[0]) / GRAPH_POINTS;

    // ИСПРАВЛЕНИЕ: Фиксируем центральный узел для всего графика
    // Это гарантирует, что мы строим один непрерывный многочлен
    int k = N / 2; 

    for (int s = 0; s <= GRAPH_POINTS; ++s) {
        double xq = x[0] + s * step;
        double q = (xq - x[k]) / h;
        
        double res = y[k];
        double coef = 1.0;

        // Формула Гаусса (вперёд)
        for (int m = 1; m < N; ++m) {
            // Обновление коэффициента
            if (m == 1) coef = q;
            else if (m % 2 == 0) coef *= (q - m / 2.0) / m;
            else                 coef *= (q + (m - 1) / 2.0) / m;

            // Правильный индекс для формулы Гаусса
            int idx = k - m / 2;
            
            // Проверка границ таблицы разностей
            if (idx < 0 || idx + m >= N) break;
            
            res += coef * diff[idx][m];
        }

        outfile << toComma(xq) << ";" << toComma(res) << "\n";
    }

    outfile.close();
    cout << "Готово! Откройте output.txt в Excel.\n";
    return 0;
}
