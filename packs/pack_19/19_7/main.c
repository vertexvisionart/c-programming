#include <stdio.h>
#include <string.h>

#define MAXN 2000
#define WORDS ((MAXN + 63) / 64)

typedef unsigned long long u64;

int N;
u64 R[MAXN][WORDS];

static inline void set_bit(int i, int j) { R[i][j / 64] |= (1ULL << (j % 64)); }

static inline int get_bit(int i, int j) {
  return (R[i][j / 64] >> (j % 64)) & 1;
}

int main() {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  scanf("%d", &N);

  int words = (N + 63) / 64;

  /* Чтение */
  for (int i = 0; i < N; i++) {
    char buf[MAXN + 2];
    scanf("%s", buf);
    for (int j = 0; j < N; j++)
      if (buf[j] == '1')
        set_bit(i, j);
  }

  for (int i = 0; i < N; i++)
    set_bit(i, i);

  for (int k = 0; k < N; k++) {
    for (int i = 0; i < N; i++) {
      if (!get_bit(i, k))
        continue;

      for (int w = 0; w < words; w++)
        R[i][w] |= R[k][w];
    }
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++)
      putchar('0' + get_bit(i, j));
    putchar('\n');
  }

  return 0;
}
