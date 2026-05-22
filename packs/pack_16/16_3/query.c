#include <stdint.h>

extern int64_t Sum(int l, int r);
extern int g_n;

int Query(int l, int64_t sum) {
  int left = l;
  int right = g_n;
  int ans = l;

  while (left <= right) {
    int mid = left + (right - left) / 2;
    if (Sum(l, mid) <= sum) {
      ans = mid;
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return ans;
}
