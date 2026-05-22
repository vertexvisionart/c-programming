#include <math.h>
#include <stdio.h>

void cf(long long p) {

  int x = (int)sqrt(p);
  long long prev_x = 1;
  long long prev_y = 0;
  long long curr_x = x;
  long long curr_y = 1;
  long long koren = x;

  long long val = (__int128)curr_x * (__int128)curr_x -
                  (__int128)p * (__int128)curr_y * (__int128)curr_y;

  if (val == -1) {
    printf("%lld %lld\n", curr_x, curr_y);
    return;
  }

  long long m = 0;
  long long d = 1;
  long long a = koren;

  while (1) {

    m = d * a - m;
    d = (p - m * m) / d;
    a = (koren + m) / d;

    long long new_x = a * curr_x + prev_x;
    long long new_y = a * curr_y + prev_y;

    if (new_x > 1e15) {
      printf("no solutions\n");
      break;
    }

    prev_x = curr_x;
    curr_x = new_x;
    prev_y = curr_y;
    curr_y = new_y;

    __int128 val1 = (__int128)curr_x * curr_x - (__int128)p * curr_y * curr_y;

    if (val1 == -1) {
      printf("%lld %lld\n", curr_x, curr_y);
      break;
    }

    if (val1 == 1) {
      printf("no solutions\n");
      break;
    }
  }
}

int main(void) {

  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);

  int T;
  scanf("%d", &T);

  long long p;
  for (int i = 0; i < T; i++) {
    scanf("%lld", &p);
    cf(p);
  }

  return 0;
}
