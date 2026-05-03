#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

using namespace std;

// Линейная интерполяция по табличным данным
double interpolate(double x, double* x_tab, double* f_tab, int n) {
    // Поиск интервала [x_tab[i], x_tab[i+1]], содержащего x
    int i = 0;
    while (i < n - 2 && x_tab[i + 1] < x) {
        i++;
    }

    // Линейная интерполяция
    double t = (x - x_tab[i]) / (x_tab[i + 1] - x_tab[i]);
    return f_tab[i] * (1.0 - t) + f_tab[i + 1] * t;
}

// Интегрирование по формуле прямоугольников (средние точки)
double rectangleMidpoint(double a, double b, int n,
    double* x_tab, double* f_tab, int n_tab) {
    double h = (b - a) / n;
    double sum = 0.0;

    for (int i = 0; i < n; i++) {
        double x_mid = a + (i + 0.5) * h;
        sum += interpolate(x_mid, x_tab, f_tab, n_tab);
    }

    return sum * h;
}

// Апостериорная оценка погрешности методом Рунге
double rungeError(double a, double b, int n,
    double* x_tab, double* f_tab, int n_tab) {
    double I_h = rectangleMidpoint(a, b, n, x_tab, f_tab, n_tab);
    double I_h2 = rectangleMidpoint(a, b, 2 * n, x_tab, f_tab, n_tab);

    return fabs(I_h2 - I_h) / 3.0;
}

// Интегрирование с автоматическим выбором шага
double integrateAdaptive(double a, double b, double eps,
    double* x_tab, double* f_tab, int n_tab,
    int& final_n, double& final_error) {
    int n = 10;
    double error = rungeError(a, b, n, x_tab, f_tab, n_tab);

    while (error > eps && n < 1000000) {
        n *= 2;
        error = rungeError(a, b, n, x_tab, f_tab, n_tab);
    }

    final_n = n;
    final_error = error;

    return rectangleMidpoint(a, b, n, x_tab, f_tab, n_tab);
}

int main() {
    setlocale(LC_ALL, "Russian");
    // === Чтение входных данных из файла ===
    ifstream fin("input.txt");
    if (!fin.is_open()) {
        cerr << "Ошибка: не удалось открыть файл input.txt" << endl;
        return 1;
    }

    int n_tab;
    fin >> n_tab;

    double* x_tab = new double[n_tab];
    double* f_tab = new double[n_tab];

    for (int i = 0; i < n_tab; i++) {
        fin >> x_tab[i] >> f_tab[i];
    }

    // === Чтение параметров интегрирования из файла ===
    double a, b, eps;
    fin >> a >> b >> eps;
    fin.close();

    // Проверка диапазона
    if (a < x_tab[0] || b > x_tab[n_tab - 1]) {
        cerr << "Ошибка: интервал [" << a << ", " << b
            << "] выходит за границы табличных данных ["
            << x_tab[0] << ", " << x_tab[n_tab - 1] << "]" << endl;
        delete[] x_tab;
        delete[] f_tab;
        return 1;
    }

    // === Вычисление интеграла ===
    int final_n;
    double final_error;

    double result = integrateAdaptive(a, b, eps, x_tab, f_tab, n_tab,
        final_n, final_error);

    // === Запись результатов в файл ===
    ofstream fout("output.txt");
    if (!fout.is_open()) {
        cerr << "Ошибка: не удалось открыть файл output.txt" << endl;
        delete[] x_tab;
        delete[] f_tab;
        return 1;
    }

    fout << fixed << setprecision(20);
    fout << "=== Результаты численного интегрирования ===" << endl;
    fout << "Метод: формула прямоугольников (средние точки), 2-й порядок" << endl;
    fout << "Интервал: [" << a << ", " << b << "]" << endl;
    fout << "Требуемая точность: " << eps << endl << endl;

    fout << "Значение интеграла: " << result << endl;
    fout << "Оценка погрешности (Рунге): " << final_error << endl;
    fout << "Число подынтервалов: " << final_n << endl;
    fout << "Фактический шаг: " << (b - a) / final_n << endl;

    fout.close();

    // === Вывод на экран ===
    cout << "Результаты записаны в файл output.txt" << endl;
    cout << "Значение интеграла: " << setprecision(10) << result << endl;
    cout << "Оценка погрешности: " << setprecision(10) << final_error << endl;


    delete[] x_tab;
    delete[] f_tab;

    return 0;
}
