#include <stdint.h>

int64_t prev[100005];
int g_n = 0;

void Init(const int *arr, int n) {
  g_n = n;
  prev[0] = 0;
  for (int i = 0; i < n; i++) {
    prev[i + 1] = prev[i] + arr[i];
  }
}

int64_t Sum(int l, int r) { return prev[r] - prev[l]; }
