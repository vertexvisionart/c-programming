#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef long long i64;

#define MAX_PRIMES 16
#define MAX_DIV    8192
#define ITERATIONS 20

static unsigned long long rng_state;

static unsigned int rng_next(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return (unsigned int)(rng_state & 0xFFFFFFFFu);
}

static int factor_number(i64 value, i64 *primes, int *exps)
{
    int count = 0;
    for (i64 p = 2; p * p <= value; p++) {
        if (value % p != 0) {
            continue;
        }
        primes[count] = p;
        exps[count] = 0;
        while (value % p == 0) {
            value /= p;
            exps[count]++;
        }
        count++;
    }
    if (value > 1) {
        primes[count] = value;
        exps[count] = 1;
        count++;
    }
    return count;
}

static int enumerate_divisors(const i64 *primes, const int *exps,
                              int prime_count, i64 *values, int *strides)
{
    values[0] = 1;
    int size = 1;
    for (int i = 0; i < prime_count; i++) {
        strides[i] = size;
        i64 p = primes[i];
        int e = exps[i];
        int new_size = size * (e + 1);
        for (int k = 0; k < size; k++) {
            i64 base_value = values[k];
            i64 power = base_value;
            for (int c = 1; c <= e; c++) {
                power *= p;
                values[k + c * size] = power;
            }
        }
        size = new_size;
    }
    return size;
}

static void build_frequencies(const i64 *data, int n,
                              const i64 *primes, const int *exps,
                              int prime_count, const int *strides,
                              int divisor_count, int *freq)
{
    memset(freq, 0, sizeof(int) * (size_t)divisor_count);
    for (int j = 0; j < n; j++) {
        i64 remaining = data[j];
        int idx = 0;
        for (int i = 0; i < prime_count; i++) {
            i64 p = primes[i];
            int limit = exps[i];
            int c = 0;
            while (c < limit && remaining % p == 0) {
                remaining /= p;
                c++;
            }
            idx += c * strides[i];
        }
        freq[idx]++;
    }
}

static void zeta_transform(int *freq, const int *exps, int prime_count,
                           const int *strides, int divisor_count)
{
    for (int i = 0; i < prime_count; i++) {
        int stride = strides[i];
        int limit = exps[i];
        int group = stride * (limit + 1);
        for (int c = limit - 1; c >= 0; c--) {
            for (int base = 0; base < divisor_count; base += group) {
                int start = base + c * stride;
                for (int offset = 0; offset < stride; offset++) {
                    int k = start + offset;
                    freq[k] += freq[k + stride];
                }
            }
        }
    }
}

static i64 pick_best_divisor(const i64 *values, const int *freq,
                             int divisor_count, int threshold, i64 current_best)
{
    i64 result = current_best;
    for (int k = 0; k < divisor_count; k++) {
        if (freq[k] >= threshold && values[k] > result) {
            result = values[k];
        }
    }
    return result;
}

int main(void)
{
    FILE *input = fopen("input.txt", "r");
    FILE *output = fopen("output.txt", "w");

    if (input == NULL || output == NULL) {
        if (input != NULL) {
            fclose(input);
        }
        if (output != NULL) {
            fclose(output);
        }
        return 1;
    }

    int n;
    if (fscanf(input, "%d", &n) != 1) {
        fclose(input);
        fclose(output);
        return 1;
    }

    i64 *data = malloc(sizeof(i64) * (size_t)n);
    if (data == NULL) {
        fclose(input);
        fclose(output);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        if (fscanf(input, "%lld", &data[i]) != 1) {
            free(data);
            fclose(input);
            fclose(output);
            return 1;
        }
    }
    fclose(input);

    int threshold = (n + 1) / 2;

    rng_state = (unsigned long long)time(NULL);
    rng_state ^= 0x9E3779B97F4A7C15ULL;
    if (rng_state == 0) {
        rng_state = 0xDEADBEEFCAFEBABEULL;
    }

    i64 primes[MAX_PRIMES];
    int exps[MAX_PRIMES];
    int strides[MAX_PRIMES];
    static i64 divisor_values[MAX_DIV];
    static int divisor_freq[MAX_DIV];

    i64 best = 1;
    int iterations = ITERATIONS;
    if (iterations > n) {
        iterations = n;
    }
    if (iterations < 1) {
        iterations = 1;
    }

    int *used = calloc((size_t)n, sizeof(int));
    if (used == NULL) {
        free(data);
        fclose(output);
        return 1;
    }

    int done = 0;
    int attempts = 0;
    int attempt_cap = iterations * 4 + 4;
    while (done < iterations && attempts < attempt_cap) {
        attempts++;
        int idx = (int)(rng_next() % (unsigned int)n);
        if (used[idx]) {
            continue;
        }
        used[idx] = 1;
        done++;

        i64 x = data[idx];
        if (x <= 0) {
            continue;
        }

        int prime_count = factor_number(x, primes, exps);
        int divisor_count = enumerate_divisors(primes, exps, prime_count,
                                               divisor_values, strides);

        if (divisor_count > MAX_DIV) {
            continue;
        }

        build_frequencies(data, n, primes, exps, prime_count, strides,
                          divisor_count, divisor_freq);
        zeta_transform(divisor_freq, exps, prime_count, strides, divisor_count);
        best = pick_best_divisor(divisor_values, divisor_freq,
                                 divisor_count, threshold, best);
    }

    fprintf(output, "%lld\n", best);

    free(used);
    free(data);
    fclose(output);
    return 0;
}
