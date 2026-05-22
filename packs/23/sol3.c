#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *fin  = fopen("input.txt",  "r");
    FILE *fout = fopen("output.txt", "w");

    int n;
    fscanf(fin, "%d", &n);

    int *nums = (int*)malloc((size_t)n * sizeof(int));
    long long sum = 0;
    int maxv = 0;
    for (int i = 0; i < n; i++) {
        fscanf(fin, "%d", &nums[i]);
        sum += nums[i];
        if (nums[i] > maxv) maxv = nums[i];
    }

    /* Если сумма нечётна — поровну не разделить. */
    if (sum & 1LL) {
        fprintf(fout, "false\n");
        free(nums); fclose(fin); fclose(fout);
        return 0;
    }

    long long target = sum / 2;

    /* Если какой-то элемент больше target — разбить невозможно. */
    if ((long long)maxv > target) {
        fprintf(fout, "false\n");
        free(nums); fclose(fin); fclose(fout);
        return 0;
    }

    /* Битсет длиной target+1 бит: dp[i] = 1, если сумму i можно набрать. */
    size_t words = (size_t)((target / 64) + 1);
    unsigned long long *dp = (unsigned long long*)calloc(words, sizeof(unsigned long long));
    dp[0] = 1ULL;                 /* пустое подмножество даёт сумму 0 */

    for (int i = 0; i < n; i++) {
        int num = nums[i];
        size_t ws = (size_t)(num / 64);     /* сдвиг в словах */
        int    bs = num % 64;               /* сдвиг внутри слова */

        /* Идём сверху вниз — одно и то же число не учитываем дважды. */
        if (bs == 0) {
            for (size_t j = words; j-- > ws; ) {
                dp[j] |= dp[j - ws];
            }
        } else {
            for (size_t j = words; j-- > ws; ) {
                unsigned long long hi = dp[j - ws] << bs;
                unsigned long long lo = (j > ws)
                    ? (dp[j - ws - 1] >> (64 - bs))
                    : 0ULL;
                dp[j] |= hi | lo;
            }
        }
    }

    size_t tw = (size_t)(target / 64);
    int    tb = (int)   (target % 64);
    int ok = (int)((dp[tw] >> tb) & 1ULL);

    fprintf(fout, "%s\n", ok ? "true" : "false");

    free(dp);
    free(nums);
    fclose(fin);
    fclose(fout);
    return 0;
}