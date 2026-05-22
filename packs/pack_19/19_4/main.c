#include <stdio.h>

#define MAXN 505

int N;
int R[MAXN][MAXN];

int main() {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  scanf("%d", &N);

  for (int i = 0; i < N; i++) {
    char buf[MAXN];
    scanf("%s", buf);
    for (int j = 0; j < N; j++)
      R[i][j] = buf[j] - '0';
  }

  for (int i = 0; i < N; i++)
    R[i][i] = 1;

  for (int k = 0; k < N; k++)
    for (int i = 0; i < N; i++)
      if (R[i][k])
        for (int j = 0; j < N; j++)
          if (R[k][j])
            R[i][j] = 1;

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++)
      putchar('0' + R[i][j]);
    putchar('\n');
  }

  return 0;
}
