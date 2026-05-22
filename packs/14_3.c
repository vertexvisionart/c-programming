#include <stdio.h>

long long gcd(long long a, long long b, long long *x, long long *y) {

  if (a == 0) {
    *x = 0;
    *y = 1;
    return b;
  }

  long long x1, y1;
  long long d = gcd(b % a, a, &x1, &y1);

  *x = y1 - (b / a) * x1;
  *y = x1;

  return d;
}

int main() {

  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int k;
  if (scanf("%d", &k) != 1)
    return 0;

  long long M[k];
  long long A[k];

  for (int i = 0; i < k; i++)
    scanf("%lld", &M[i]);

  for (int i = 0; i < k; i++)
    scanf("%lld", &A[i]);

  unsigned __int128 P = 1;
  for (int i = 0; i < k; i++)
    P *= M[i];

  unsigned __int128 res = 0;

  for (int i = 0; i < k; i++) {
    unsigned __int128 Ni = P / M[i];

    long long x, y;
    gcd((long long)(Ni % M[i]), M[i], &x, &y);

    x = (x % M[i] + M[i]) % M[i];

    unsigned __int128 term = (unsigned __int128)A[i] * Ni;
    term = term % P;
    term = (term * x) % P;

    res = (res + term) % P;
  }

  printf("%lld\n", (long long)res);

  return 0;
}
