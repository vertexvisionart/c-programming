/* LeetCode 278: First Bad Version — binary search. */
#include <stdbool.h>

/* Provided by the judge. */
bool isBadVersion(int version);

int firstBadVersion(int n) {
    long long left = 0;
    long long right = n;

    while (left < right) {
        long long mid = left + (right - left) / 2;
        if (isBadVersion((int)mid)) right = mid;
        else                        left = mid + 1;
    }
    return (int)left;
}
