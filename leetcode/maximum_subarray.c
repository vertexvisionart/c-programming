/* LeetCode 53: Maximum Subarray — Kadane's algorithm. */
int maxSubArray(int *nums, int numsSize) {
    int sum = 0;
    int res = nums[0];
    for (int i = 0; i < numsSize; i++) {
        if (sum < 0) sum = 0;
        sum += nums[i];
        if (sum > res) res = sum;
    }
    return res;
}
