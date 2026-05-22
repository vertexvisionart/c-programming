#include <math.h>
#include <stdio.h>

int main(void) {

  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int N;

  scanf("%d", &N);

  double a[N + 1][N + 1];
  double ans[N + 1];

  for (int i = 0; i < N; i++) {
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

    for (int j = i; j <= N; j++) {
      double temp = a[i][j];
      a[i][j] = a[pivot][j];
      a[pivot][j] = temp;
    }
    for (int k = i; k < N; k++) {
      double firstN = a[k][i];
      if (fabs(firstN) > 1e-9) {
        for (int j = i; j <= N; j++) {
          a[k][j] /= firstN;
        }
      }
    }

    for (int k = i + 1; k < N; k++) {
      if (fabs(a[k][i]) < 1e-9)
        continue;
      for (int j = 0; j <= N; j++) {
        a[k][j] -= a[i][j];
      }
    }
  }

  for (int i = N - 1; i >= 0; i--) {

    double sum = 0;
    for (int j = i + 1; j < N; j++) {
      sum += a[i][j] * ans[j];
    }

    ans[i] = a[i][N] - sum;
  }

  for (int i = 0; i < N; i++) {
    printf("%lf\n", ans[i]);
  }
  return 0;
}
