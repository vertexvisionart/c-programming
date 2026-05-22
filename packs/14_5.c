#include <stdio.h>

#define MAXN 1000005
#define MOD 1000000007

long long int fact[MAXN];
long long invfact[MAXN];

long long power(long long int n) {
  long long res = 1;
  long long exp = MOD - 2;

  while (exp > 0) {
    if (exp % 2 == 1) {
      res = (res * n) % MOD;
    }
    n = (n * n) % MOD;
    exp /= 2;
  }

  return res;
}

int main(void) {

  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  fact[0] = 1;
  fact[1] = 1;

  for (int i = 2; i < MAXN; i++) {
    fact[i] = ((long long)i * fact[i - 1]) % MOD;
  }

  int T;
  long long int N;
  long long int K;

  for (int i = 0; i < MAXN; i++) {
    invfact[i] = power(fact[i]);
  }

  long long res;

  scanf("%d", &T);
  for (int i = 0; i < T; i++) {
    scanf("%lld %lld", &N, &K);
    res = (fact[N] * invfact[K]) % MOD;
    res = (res * invfact[N - K]) % MOD;
    printf("%lld\n", res);
  }

  return 0;
}
