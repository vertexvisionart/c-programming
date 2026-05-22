#include <stdio.h>

long long binpow(long long base, long long b, long long M) {
  long long resu result = 1;
  base = base % M;

  while (b > 0) {
    if (b % 2 == 1) {
      result = (result * base) % M;
    }
    base = (base * base) % M;
    b /= 2;
  }

  return result;
}

int main() {

  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int T;
  long long M;

  if (scanf("%d %lld", &T, &M) != 2)
    return 0;

  for (int i = 0; i < T; i++) {
    long long A;
    scanf("%lld", &A);

    if (A == 0) {
      printf("-1\n");
    } else {
      printf("%lld\n", binpow(A, M - 2, M));
    }
  }

  return 0;
}
