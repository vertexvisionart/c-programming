#include "modular.h"

int MOD = 2;

static int fold_mod(long long value)
{
    long long result = value % MOD;

    if (result < 0) {
        result += MOD;
    }

    return (int) result;
}

int pnorm(int value)
{
    return fold_mod(value);
}

int padd(int lhs, int rhs)
{
    int left = pnorm(lhs);
    int right = pnorm(rhs);

    return fold_mod((long long) left + right);
}

int psub(int lhs, int rhs)
{
    int left = pnorm(lhs);
    int right = pnorm(rhs);

    return fold_mod((long long) left - right);
}

int pmul(int lhs, int rhs)
{
    int left = pnorm(lhs);
    int right = pnorm(rhs);

    return fold_mod((long long) left * right);
}

static int invert_residue(int value)
{
    long long old_r = MOD;
    long long r = pnorm(value);
    long long old_t = 0;
    long long t = 1;

    if (r == 0) {
        return 0;
    }

    while (r != 0) {
        long long quotient = old_r / r;
        long long next_r = old_r - quotient * r;
        long long next_t = old_t - quotient * t;

        old_r = r;
        r = next_r;
        old_t = t;
        t = next_t;
    }

    if (old_r != 1) {
        return 0;
    }

    return fold_mod(old_t);
}

int pdiv(int lhs, int rhs)
{
    return pmul(lhs, invert_residue(rhs));
}
