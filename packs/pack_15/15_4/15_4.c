#include <stdio.h>

#define MOD_COUNT 1000000007

long long power(long long n, long long exp, long long mod) {
  long long res = 1;
  n %= mod;
  while (exp > 0) {
    if (exp % 2 == 1)
      res = (res * n) % mod;
    n = (n * n) % mod;
    exp /= 2;
  }
  return res;
}

long long modInverse(long long n, long long P) { return power(n, P - 2, P); }

int main(void) {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int N, K;
  long long P;

  if (scanf("%d %d %lld", &N, &K, &P) != 3)
    return 0;

  long long a[K][N + 1];
  long long ans[N];
  int where[N];
  for (int i = 0; i < N; ++i)
    where[i] = -1;

  for (int i = 0; i < K; i++) {
    for (int j = 0; j <= N; j++) {
      scanf("%lld", &a[i][j]);
      a[i][j] %= P;
    }
  }

  int row = 0;
  for (int col = 0; col < N && row < K; col++) {
    int pivot = row;
    for (int k = row + 1; k < K; k++) {
      if (a[k][col] != 0) {
        pivot = k;
        break;
      }
    }

    if (a[pivot][col] == 0)
      continue;

    for (int j = col; j <= N; j++) {
      long long temp = a[row][j];
      a[row][j] = a[pivot][j];
      a[pivot][j] = temp;
    }

    long long firstN = a[row][col];
    long long inv = modInverse(firstN, P);

    for (int j = col; j <= N; j++) {
      a[row][j] = (a[row][j] * inv) % P;
    }

    for (int k = 0; k < K; k++) {
      if (k != row) {
        long long factor = a[k][col];
        if (factor != 0) {
          for (int j = col; j <= N; j++) {
            long long val = (a[row][j] * factor) % P;
            a[k][j] = (a[k][j] - val + P) % P;
          }
        }
      }
    }
    where[col] = row;
    row++;
  }

  for (int i = row; i < K; i++) {
    if (a[i][N] != 0) {
      printf("0\n");
      return 0;
    }
  }

  if (row < N) {
    printf("%lld\n", power(P, N - row, MOD_COUNT));
    return 0;
  }

  printf("1\n");

  for (int i = 0; i < N; i++) {
    if (where[i] != -1) {
      ans[i] = a[where[i]][N];
    } else {
      ans[i] = 0;
    }
  }

  for (int i = 0; i < N; i++) {
    printf("%lld\n", ans[i]);
  }

  return 0;
}
