#include <stdio.h>

int C[2005][2005];

int main(void) {

  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int M, T;
  scanf("%d %d", &M, &T);

  C[0][0] = 1;

  for (int n = 1; n <= 2000; n++) {
    C[n][0] = 1;
    for (int k = 1; k <= n; k++) {
      C[n][k] = (C[n - 1][k - 1] + C[n - 1][k]) % M;
    }
  }

  int N, K;

  for (int i = 0; i < T; i++) {
    scanf("%d %d", &N, &K);
    if (K < 0 || K > N) {
      printf("0\n");
    } else {
      printf("%d\n", C[N][K]);
    }
  }
  return 0;
}
