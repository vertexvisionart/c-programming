#include <assert.h>

#include "modular.h"

struct unary_case {
    int input;
    int expected;
};

struct binary_case {
    int left;
    int right;
    int expected;
};

static int model_norm(long long value)
{
    long long result = value % MOD;

    if (result < 0) {
        result += MOD;
    }

    return (int)result;
}

static int model_add(int left, int right)
{
    return model_norm((long long)left + right);
}

static int model_sub(int left, int right)
{
    return model_norm((long long)left - right);
}

static int model_mul(int left, int right)
{
    return model_norm((long long)left * right);
}

static int search_inverse(int value)
{
    int normalized = model_norm(value);
    int candidate;

    assert(normalized != 0);
    for (candidate = 1; candidate < MOD; ++candidate) {
        if (model_mul(normalized, candidate) == 1) {
            return candidate;
        }
    }

    assert(0);
    return 0;
}

static int model_div(int left, int right)
{
    return model_mul(left, search_inverse(right));
}

static void check_normalization_table(const struct unary_case *cases, int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        assert(pnorm(cases[i].input) == cases[i].expected);
    }
}

static void check_binary_table(
    int (*operation)(int, int),
    const struct binary_case *cases,
    int count
)
{
    int i;

    for (i = 0; i < count; ++i) {
        assert(operation(cases[i].left, cases[i].right) == cases[i].expected);
    }
}

static void sweep_small_modulus(int mod)
{
    int value;
    int left;
    int right;

    MOD = mod;
    for (value = -2 * mod - 3; value <= 2 * mod + 3; ++value) {
        int reduced = pnorm(value);
        assert(reduced == model_norm(value));
        assert(reduced >= 0);
        assert(reduced < MOD);
    }

    for (left = 0; left < MOD; ++left) {
        assert(pdiv(left, 1) == left);
        for (right = 0; right < MOD; ++right) {
            assert(padd(left, right) == model_add(left, right));
            assert(psub(left, right) == model_sub(left, right));
            assert(pmul(left, right) == model_mul(left, right));

            if (right != 0) {
                int value_div = pdiv(left, right);
                assert(value_div == model_div(left, right));
                assert(pmul(value_div, right) == left);
            }
        }
    }
}

static void run_table_checks_for_13(void)
{
    static const struct unary_case norm_cases[] = {
        {45, 6},
        {27, 1},
        {13, 0},
        {0, 0},
        {-1, 12},
        {-14, 12},
        {-45, 7}
    };
    static const struct binary_case add_cases[] = {
        {0, 0, 0},
        {7, 6, 0},
        {12, 12, 11},
        {7, 12, 6}
    };
    static const struct binary_case sub_cases[] = {
        {3, 5, 11},
        {2, 3, 12},
        {8, 8, 0}
    };
    static const struct binary_case mul_cases[] = {
        {0, 7, 0},
        {7, 0, 0},
        {9, 12, 4},
        {12, 12, 1}
    };
    static const struct binary_case div_cases[] = {
        {7, 4, 5},
        {1, 2, 7},
        {0, 5, 0}
    };

    MOD = 13;
    check_normalization_table(norm_cases, (int)(sizeof(norm_cases) / sizeof(norm_cases[0])));
    check_binary_table(padd, add_cases, (int)(sizeof(add_cases) / sizeof(add_cases[0])));
    check_binary_table(psub, sub_cases, (int)(sizeof(sub_cases) / sizeof(sub_cases[0])));
    check_binary_table(pmul, mul_cases, (int)(sizeof(mul_cases) / sizeof(mul_cases[0])));
    check_binary_table(pdiv, div_cases, (int)(sizeof(div_cases) / sizeof(div_cases[0])));
}

static void run_large_modulus_checks(void)
{
    static const struct unary_case norm_cases[] = {
        {-1000000000, 999999874},
        {-999999938, 999999936},
        {-1, 999999936},
        {0, 0},
        {999999936, 999999936},
        {999999937, 0},
        {1000000000, 63}
    };
    int numerators[] = {1, 3, 17, 123456789, 500000000, 999999936};
    int denominators[] = {1, 2, 19, 123456789, 500000000, 999999936};
    int i;

    MOD = 999999937;
    check_normalization_table(norm_cases, (int)(sizeof(norm_cases) / sizeof(norm_cases[0])));

    assert(padd(999999936, 10) == 9);
    assert(psub(3, 5) == 999999935);
    assert(psub(0, 1000000000) == model_sub(0, 1000000000));
    assert(pmul(999999936, 999999936) == 1);
    assert(pmul(500000000, 500000000) == model_mul(500000000, 500000000));

    for (i = 0; i < (int)(sizeof(numerators) / sizeof(numerators[0])); ++i) {
        int result = pdiv(numerators[i], denominators[i]);
        assert(result == model_div(numerators[i], denominators[i]));
        assert(pmul(result, denominators[i]) == numerators[i]);
    }
}

int main(void)
{
    run_table_checks_for_13();
    sweep_small_modulus(2);
    sweep_small_modulus(3);
    sweep_small_modulus(5);
    sweep_small_modulus(7);
    sweep_small_modulus(11);
    sweep_small_modulus(17);
    run_large_modulus_checks();

    return 0;
}
