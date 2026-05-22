// main.c
#include "primes.h"
#include <assert.h>

int main() {
  // isPrime tests
  assert(isPrime(0) == 0);
  assert(isPrime(1) == 0);
  assert(isPrime(2) == 1);
  assert(isPrime(3) == 1);
  assert(isPrime(4) == 0);
  assert(isPrime(17) == 1);
  assert(isPrime(18) == 0);
  assert(isPrime(19) == 1);
  assert(isPrime(97) == 1);
  assert(isPrime(100) == 0);

  // findNextPrime tests
  assert(findNextPrime(0) == 2);
  assert(findNextPrime(1) == 2);
  assert(findNextPrime(2) == 2);
  assert(findNextPrime(3) == 3);
  assert(findNextPrime(4) == 5);
  assert(findNextPrime(5) == 5);
  assert(findNextPrime(6) == 7);
  assert(findNextPrime(10) == 11);
  assert(findNextPrime(11) == 11);
  assert(findNextPrime(12) == 13);
  assert(findNextPrime(20) == 23);
  assert(findNextPrime(23) == 23);

  // getPrimesCount tests
  assert(getPrimesCount(0, 10) == 4);
  assert(getPrimesCount(10, 20) == 4);
  assert(getPrimesCount(2, 3) == 1);
  assert(getPrimesCount(2, 2) == 0);
  assert(getPrimesCount(90, 100) == 1);
  assert(getPrimesCount(1, 2) == 0);
  assert(getPrimesCount(100, 200) == 21);

  // edge cases near 10^7
  int next = findNextPrime(10000000);
  assert(next > 10000000);
  assert(isPrime(next) == 1);
  int cnt = getPrimesCount(10000000 - 10, 10000000);
  assert(cnt >= 0 && cnt <= 10);

  return 0;
}
