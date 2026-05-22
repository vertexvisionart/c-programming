// primes.c
#include "primes.h"
#include <math.h>

#define MAX 10000000

static unsigned char sieve[MAX + 1];
static int primes[1000000];
static int prime_count = 0;
static int initialized = 0;

static void init(void) {
  if (initialized)
    return;

  for (int i = 0; i <= MAX; ++i)
    sieve[i] = 1;
  sieve[0] = sieve[1] = 0;

  for (int i = 2; i * i <= MAX; ++i) {
    if (sieve[i]) {
      for (int j = i * i; j <= MAX; j += i) {
        sieve[j] = 0;
      }
    }
  }

  for (int i = 2; i <= MAX; ++i) {
    if (sieve[i])
      primes[prime_count++] = i;
  }

  initialized = 1;
}

static int lower_bound(int arr[], int lo, int hi, int key) {
  while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    if (arr[mid] < key)
      lo = mid + 1;
    else
      hi = mid;
  }
  return lo;
}

int isPrime(int x) {
  init();
  if (x < 2)
    return 0;
  if (x > MAX) {
    int limit = (int)sqrt(x);
    for (int i = 0; i < prime_count && primes[i] <= limit; ++i) {
      if (x % primes[i] == 0)
        return 0;
    }
    return 1;
  }
  return sieve[x];
}

int findNextPrime(int x) {
  init();
  if (x <= 2)
    return 2;

  for (int i = x; i <= MAX; ++i) {
    if (sieve[i])
      return i;
  }

  for (int n = MAX + 1;; ++n) {
    int limit = (int)sqrt(n);
    int is_prime = 1;
    for (int i = 0; i < prime_count && primes[i] <= limit; ++i) {
      if (n % primes[i] == 0) {
        is_prime = 0;
        break;
      }
    }
    if (is_prime)
      return n;
  }
}

int getPrimesCount(int l, int r) {
  init();
  if (l >= r)
    return 0;
  int left = lower_bound(primes, 0, prime_count, l);
  int right = lower_bound(primes, 0, prime_count, r);
  return right - left;
}
