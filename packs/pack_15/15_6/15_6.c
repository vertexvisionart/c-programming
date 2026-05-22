#include <math.h>
#include <stdio.h>

#define MAXN 110

double A[MAXN][MAXN];
double L[MAXN][MAXN];
double U[MAXN][MAXN];
int P[MAXN];

int LUP_Decomposition(int n) {

  for (int i = 0; i < n; i++) {
    P[i] = i;
  }

  for (int k = 0; k < n; k++) {

    double max_val = 0.0;
    int k_prime = k;

    for (int i = k; i < n; i++) {
      if (fabs(A[i][k]) > max_val) {
        max_val = fabs(A[i][k]);
        k_prime = i;
      }
    }

    if (max_val < 1e-9)
      return 0;

    int temp_p = P[k];
    P[k] = P[k_prime];
    P[k_prime] = temp_p;

    for (int j = 0; j < n; j++) {
      double temp_a = A[k][j];
      A[k][j] = A[k_prime][j];
      A[k_prime][j] = temp_a;
    }

    for (int i = k + 1; i < n; i++) {
      A[i][k] = A[i][k] / A[k][k];
      for (int j = k + 1; j < n; j++) {
        A[i][j] = A[i][j] - A[i][k] * A[k][j];
      }
    }
  }
  return 1;
}

void LUP_Solve(int n, double *b, double *x) {
  double y[MAXN];

  for (int i = 0; i < n; i++) {
    double sum = 0.0;
    for (int k = 0; k < i; k++) {
      sum += A[i][k] * y[k];
    }

    y[i] = b[P[i]] - sum;
  }

  for (int i = n - 1; i >= 0; i--) {
    double sum = 0.0;
    for (int k = i + 1; k < n; k++) {
      sum += A[i][k] * x[k];
    }
    x[i] = (y[i] - sum) / A[i][i];
  }
}

int main(void) {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int n;
  if (scanf("%d", &n) != 1)
    return 0;

  double X[MAXN], Y[MAXN];
  double B[MAXN];
  double Coeffs[MAXN];

  for (int i = 0; i < n; i++) {
    scanf("%lf %lf", &X[i], &Y[i]);
    B[i] = Y[i];

    double val = 1.0;
    for (int j = 0; j < n; j++) {
      A[i][j] = val;
      val *= X[i];
    }
  }

  if (!LUP_Decomposition(n)) {
    return 0; // Ошибка
  }

  LUP_Solve(n, B, Coeffs);

  for (int i = 0; i < n; i++) {
    printf("%.10lf%c", Coeffs[i], (i == n - 1) ? '\n' : ' ');
  }

  return 0;
}
