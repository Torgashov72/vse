#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;

const double EPS = 1e-9;

vector<double> gauss(vector<vector<double>> a, vector<double> b) {
    int n = a.size();
    double det = 1.0;

    cout << "ПРЯМОЙ ХОД" << endl << endl;

    for (int i = 0; i < n; i++) {
        cout << "ШАГ " << i + 1 << endl;

        int maxRow = i;
        for (int k = i + 1; k < n; k++) {
            if (fabs(a[k][i]) > fabs(a[maxRow][i])) {
                maxRow = k;
            }
        }

        if (fabs(a[maxRow][i]) < EPS) {
            cout << "ОШИБКА МАТРИЦА ВЫРОЖДЕНА" << endl;
            cout << "ОПРЕДЕЛИТЕЛЬ 0" << endl;
            cout << "НЕТ ЕДИНСТВЕННОГО РЕШЕНИЯ" << endl;
            return vector<double>();
        }

        if (maxRow != i) {
            swap(a[i], a[maxRow]);
            swap(b[i], b[maxRow]);
            det *= -1;
            cout << "   МЕНЯЕМ СТРОКИ " << i + 1 << " И " << maxRow + 1 << endl;
        }

        det *= a[i][i];

        for (int k = i + 1; k < n; k++) {
            double coeff = a[k][i] / a[i][i];
            for (int j = i; j < n; j++) {
                a[k][j] -= coeff * a[i][j];
            }
            b[k] -= coeff * b[i];
        }

        cout << "   ДИАГОНАЛЬНЫЙ ЭЛЕМЕНТ  " << "A" << i + 1 << i + 1 << "  " << a[i][i] << endl;
        cout << "   ОПРЕДЕЛИТЕЛЬ  " << det << endl << endl;
    }

    cout << "ОПРЕДЕЛИТЕЛЬ  " << det << endl << endl;
    cout << "ОБРАТНЫЙ ХОД" << endl << endl;

    vector<double> x(n);
    for (int i = n - 1; i >= 0; i--) {
        x[i] = b[i];
        for (int j = i + 1; j < n; j++) {
            x[i] -= a[i][j] * x[j];
        }
        x[i] /= a[i][i];
        cout << "X" << i + 1 << "  " << fixed << setprecision(3) << x[i] << endl;
    }

    return x;
}

int main() {
    setlocale(LC_ALL, "rus");
    ifstream f("C:/Users/egorg/OneDrive/Рабочий стол/доклады/iletext.txt");
    if (!f.is_open()) {
        cout << "ОШИБКА ФАЙЛ input.txt НЕ НАЙДЕН" << endl;
        return 1;
    }

    int n;
    f >> n;

    vector<vector<double>> a(n, vector<double>(n));
    vector<double> b(n);

    cout << "ИСХОДНАЯ СИСТЕМА" << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            f >> a[i][j];
            cout << setw(10) << a[i][j] << " ";
        }
        cout << endl;
    }

    for (int i = 0; i < n; i++) {
        f >> b[i];
    }
    cout << "ПРАВАЯ ЧАСТЬ  ";
    for (int i = 0; i < n; i++) {
        cout << setw(10) << b[i] << " ";
    }
    cout << endl << endl;

    f.close();

    vector<double> x = gauss(a, b);

    if (!x.empty()) {
        cout << endl << "РЕШЕНИЕ" << endl;
        for (int i = 0; i < n; i++) {
            cout << "X" << i + 1 << "  " << fixed << setprecision(6) << x[i] << endl;
        }
        cout << endl;
    }

    return 0;
}
