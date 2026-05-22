#include <stdio.h>

typedef struct Factors {
  int k;          // сколько различных простых в разложении
  int primes[32]; // различные простые в порядке возрастания
  int powers[32]; // в какие степени надо эти простые возводить
} Factors;

extern void Factorize(int X, Factors *res);

int main(void) {
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int m;
  scanf("%d", &m);

  for (int i = 0; i < m; i++) {
    int number;
    scanf("%d", &number);

    Factors res;
    Factorize(number, &res);

    printf("%d = ", number);

    if (res.k == 0) {
      printf("1\n");
    } else {
      for (int j = 0; j < res.k; j++) {
        printf("%d^%d", res.primes[j], res.powers[j]);
        if (j < res.k - 1) {
          printf(" * ");
        }
      }
      printf("\n");
    }
  }

  return 0;
}
