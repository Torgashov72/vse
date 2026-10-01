import numpy as np
import pandas as pd
import skfuzzy as fuzz
import matplotlib.pyplot as plt

# ============================================================
# ЗАДАЧА 1: Статистическая обработка + scikit-fuzzy
# ============================================================
print("=" * 70)
print("ЗАДАЧА 1: Функции принадлежности (Эксперты + scikit-fuzzy)")
print("Тема: Рост мужчины")
print("=" * 70)

# 1. Дискретные данные (исправленные: 1 эксперт = 1 терм на диапазон)
ranges_labels = ['160-165', '165-170', '170-175', '175-180', '180-185', '185-190', '190-195', '195-200']
# Центры диапазонов для построения графика
x_experts = np.array([162.5, 167.5, 172.5, 177.5, 182.5, 187.5, 192.5, 197.5]) 

terms = ['Низкий', 'Средний', 'Высокий']

experts = {
    'Эксперт 1': {'Низкий': [1, 1, 0, 0, 0, 0, 0, 0], 'Средний': [0, 0, 1, 1, 1, 0, 0, 0], 'Высокий': [0, 0, 0, 0, 0, 1, 1, 1]},
    'Эксперт 2': {'Низкий': [1, 1, 0, 0, 0, 0, 0, 0], 'Средний': [0, 0, 1, 1, 0, 0, 0, 0], 'Высокий': [0, 0, 0, 0, 1, 1, 1, 1]},
    'Эксперт 3': {'Низкий': [1, 0, 0, 0, 0, 0, 0, 0], 'Средний': [0, 1, 1, 1, 1, 0, 0, 0], 'Высокий': [0, 0, 0, 0, 0, 1, 1, 1]},
    'Эксперт 4': {'Низкий': [1, 1, 0, 0, 0, 0, 0, 0], 'Средний': [0, 0, 1, 1, 1, 0, 0, 0], 'Высокий': [0, 0, 0, 0, 0, 1, 1, 1]},
    'Эксперт 5': {'Низкий': [1, 1, 0, 0, 0, 0, 0, 0], 'Средний': [0, 0, 1, 1, 0, 0, 0, 0], 'Высокий': [0, 0, 0, 0, 1, 1, 1, 1]},
}

n_experts = len(experts)

# Подсчет голосов и расчет μ
votes = {term: np.zeros(len(ranges_labels), dtype=int) for term in terms}
for exp_data in experts.values():
    for term in terms:
        votes[term] += np.array(exp_data[term])

mu_experts = {term: votes[term] / n_experts for term in terms}

print("\n1. Степени принадлежности μ (по экспертам):")
df_mu = pd.DataFrame(mu_experts, index=ranges_labels).T
print(df_mu.round(2))

# 2. Непрерывное универсальное множество (от 160 до 200 см с шагом 0.1)
x_continuous = np.arange(160, 200.1, 0.1)

# 3. ГЕНЕРАЦИЯ ФУНКЦИЙ ПРИНАДЛЕЖНОСТИ ЧЕРЕЗ scikit-fuzzy
# Подбираем параметры (a, b, c, d) так, чтобы кривые огибали экспертные точки

# Z-образная функция (zmf) для "Низкий"
# Параметры (a, b): равна 1 при x <= a, плавно падает до 0 при x = b
mu_low = fuzz.zmf(x_continuous, 165, 180)

# П-образная функция (pimf) для "Средний"
# Параметры (a, b, c, d): равна 0 при x<=a, растет до 1 при x=b, держится до x=c, падает до 0 при x=d
mu_mid = fuzz.pimf(x_continuous, 160, 170, 180, 190)

# S-образная функция (smf) для "Высокий"
# Параметры (a, b): равна 0 при x <= a, плавно растет до 1 при x = b
mu_high = fuzz.smf(x_continuous, 175, 190)

# 4. Построение графика
plt.figure(figsize=(12, 7))

# Рисуем непрерывные аналитические кривые из scikit-fuzzy (сплошные линии)
plt.plot(x_continuous, mu_low, 'b-', linewidth=2.5, label='Низкий (Z-образная, skfuzzy)')
plt.plot(x_continuous, mu_mid, color='orange', linewidth=2.5, label='Средний (П-образная, skfuzzy)')
plt.plot(x_continuous, mu_high, 'g-', linewidth=2.5, label='Высокий (S-образная, skfuzzy)')

# Рисуем наши дискретные экспертные точки (маркеры)
plt.plot(x_experts, mu_experts['Низкий'], 'bo', markersize=9, label='Эксперты (Низкий)')
plt.plot(x_experts, mu_experts['Средний'], 'o', color='orange', markersize=9, label='Эксперты (Средний)')
plt.plot(x_experts, mu_experts['Высокий'], 'go', markersize=9, label='Эксперты (Высокий)')

plt.title('Функции принадлежности: Экспертные данные vs Модели scikit-fuzzy', fontsize=14)
plt.xlabel('Рост (см)', fontsize=12)
plt.ylabel('Степень принадлежности μ', fontsize=12)
plt.legend(fontsize=11, loc='center right')
plt.grid(True, linestyle='--', alpha=0.7)
plt.ylim(-0.05, 1.1)
plt.xlim(160, 200)
plt.xticks(np.arange(160, 205, 5))
plt.tight_layout()
plt.show()

print("\n✅ График построен!")
print("Сплошные линии — это непрерывные математические модели из scikit-fuzzy.")
print("Точки — это усредненные оценки экспертов.")
