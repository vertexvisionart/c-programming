typedef struct Factors {
  int k;          // сколько различных простых в разложении
  int primes[32]; // различные простые в порядке возрастания
  int powers[32]; // в какие степени надо эти простые возводить
} Factors;

void Factorize(int X, Factors *res) {
  res->k = 0;
  for (int i = 2; i * i <= X; i++) {
    if (X % i == 0) {
      res->primes[res->k] = i;
      res->powers[res->k] = 0;
      while (X % i == 0) {
        res->powers[res->k]++;
        X /= i;
      }
      res->k++;
    }
  }
  if (X > 1) {
    res->primes[res->k] = X;
    res->powers[res->k] = 1;
    res->k++;
  }
}
