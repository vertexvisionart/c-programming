#include <math.h>
#include <stdio.h>

#define Wr 1

int main(void) {

  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int N;

  scanf("%d", &N);

  double a[N][N + 1];
  double x[N];
  for (int i = 0; i < N; i++) {
    x[i] = 0.0;
    for (int j = 0; j <= N; j++) {
      scanf("%lf", &a[i][j]);
    }
  }

  for (int i = 0; i < N; i++) {
    int pivot = i;
    for (int k = i + 1; k < N; k++) {
      if (fabs(a[k][i]) > fabs(a[pivot][i])) {
        pivot = k;
      }
    }

    for (int j = 0; j <= N; j++) {
      double temp = a[i][j];
      a[i][j] = a[pivot][j];
      a[pivot][j] = temp;
    }
  }

  int iter = 0;
  double error = 0.0;

  do {
    error = 0.0;
    iter++;

    for (int i = 0; i < N; i++) {
      double old_value = x[i];
      double sum = 0.0;
      for (int j = 0; j < N; j++) {
        sum += a[i][j] * x[j];
      }
      double soul = a[i][N] - sum;
      x[i] = old_value + (Wr / a[i][i]) * soul;

      if (fabs(x[i] - old_value) > error) {
        error = fabs(x[i] - old_value);
      }
    }

  } while (iter < 1000 && fabs(error) > 0.00001);

  for (int i = 0; i < N; i++) {
    printf("%lf\n", x[i]);
  }
  return 0;
}
