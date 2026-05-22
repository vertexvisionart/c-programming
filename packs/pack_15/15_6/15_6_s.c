#include <math.h>
#include <stdio.h>

// Максимальный размер N <= 100
#define MAXN 110

// Глобальные матрицы для LUP
double A[MAXN][MAXN];
double L[MAXN][MAXN];
double U[MAXN][MAXN];
int P[MAXN]; // Матрица перестановок (храним как массив индексов)

// Алгоритм LUP-разложения (строго по слайдам 36-40)
// Возвращает 1 если успешно, 0 если матрица вырождена
int LUP_Decomposition(int n) {
  // Инициализация P: изначально P[i] = i
  for (int i = 0; i < n; i++) {
    P[i] = i;
  }

  for (int k = 0; k < n; k++) {
    // --- Pivoting (Выбор ведущего элемента, слайд 38) ---
    double max_val = 0.0;
    int k_prime = k;

    for (int i = k; i < n; i++) {
      if (fabs(A[i][k]) > max_val) {
        max_val = fabs(A[i][k]);
        k_prime = i;
      }
    }

    if (max_val < 1e-9)
      return 0; // Матрица вырождена (деление на 0)

    // Меняем местами строки в матрице перестановки P
    int temp_p = P[k];
    P[k] = P[k_prime];
    P[k_prime] = temp_p;

    // Меняем местами строки в самой матрице A
    for (int j = 0; j < n; j++) {
      double temp_a = A[k][j];
      A[k][j] = A[k_prime][j];
      A[k_prime][j] = temp_a;
    }

    // --- Schur Complement update (Дополнение Шура, слайд 36-37) ---
    // Формируем L и U прямо внутри алгоритма
    for (int i = k + 1; i < n; i++) {
      A[i][k] = A[i][k] / A[k][k]; // Это элементы матрицы L
      for (int j = k + 1; j < n; j++) {
        A[i][j] =
            A[i][j] - A[i][k] * A[k][j]; // Обновляем остаток (Schur complement)
      }
    }
  }
  return 1;
}

// Функция решения системы LUP: Ax = b -> PAx = Pb -> LUx = Pb
void LUP_Solve(int n, double *b, double *x) {
  double y[MAXN];

  // 1. Прямой ход (Forward Substitution): Решаем Ly = Pb
  // Т.к. мы храним L и U в одной матрице A (L под диагональю, U над):
  // Элементы L[i][k] лежат в A[i][k] при i > k. Диагональ L - единицы.

  for (int i = 0; i < n; i++) {
    double sum = 0.0;
    for (int k = 0; k < i; k++) {
      sum += A[i][k] * y[k];
    }
    // b[P[i]] - потому что мы переставляли строки
    y[i] = b[P[i]] - sum;
  }

  // 2. Обратный ход (Backward Substitution): Решаем Ux = y
  // Элементы U[i][k] лежат в A[i][k] при i <= k.

  for (int i = n - 1; i >= 0; i--) {
    double sum = 0.0;
    for (int k = i + 1; k < n; k++) {
      sum += A[i][k] * x[k];
    }
    x[i] = (y[i] - sum) / A[i][i];
  }
}

int main(void) {
  // freopen("input.txt", "r", stdin);
  // freopen("output.txt", "w", stdout);

  int n;
  if (scanf("%d", &n) != 1)
    return 0;

  double X[MAXN], Y[MAXN];
  double B[MAXN];      // Вектор правой части (y_i)
  double Coeffs[MAXN]; // Ответ (c_1 ... c_n)

  // Считываем точки и строим СЛАУ
  for (int i = 0; i < n; i++) {
    scanf("%lf %lf", &X[i], &Y[i]);
    B[i] = Y[i];

    // Строим матрицу Вандермонда
    // A[i][j] = (X[i])^j
    double val = 1.0;
    for (int j = 0; j < n; j++) {
      A[i][j] = val;
      val *= X[i];
    }
  }

  // 1. Делаем разложение
  if (!LUP_Decomposition(n)) {
    return 0; // Ошибка
  }

  // 2. Решаем систему
  LUP_Solve(n, B, Coeffs);

  // Вывод
  for (int i = 0; i < n; i++) {
    printf("%.10lf%c", Coeffs[i], (i == n - 1) ? '\n' : ' ');
  }

  return 0;
}
