#include <stdbool.h>

// The API isBadVersionion is defined for you.
bool isBadVersion(int version);

int firstBadVersion(int n) {

  long long int left = 0;
  long long int right = n;

  while (left < right) {
    long long mid = left + (right - left) / 2;
    if (isBadVersion(mid) == true) {
      right = mid;
    } else {
      left = mid + 1;
    }
  }
  return left;
}
