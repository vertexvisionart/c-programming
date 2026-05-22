#include "modular.h"

int MOD = 2;

static int pnorm_ll(long long value)
{
    long long result = value % MOD;
    if (result < 0) {
        result += MOD;
    }
    return (int) result;
}

int pnorm(int value)
{
    return pnorm_ll(value);
}

int padd(int lhs, int rhs)
{
    return pnorm_ll((long long) lhs + rhs);
}

int psub(int lhs, int rhs)
{
    return pnorm_ll((long long) lhs - rhs);
}

int pmul(int lhs, int rhs)
{
    return (int) (((long long) lhs * rhs) % MOD);
}

static int ppow(int base, int exponent)
{
    int result = 1;
    int value = base;

    while (exponent > 0) {
        if ((exponent & 1) != 0) {
            result = pmul(result, value);
        }
        value = pmul(value, value);
        exponent >>= 1;
    }

    return result;
}

int pdiv(int lhs, int rhs)
{
    return pmul(lhs, ppow(rhs, MOD - 2));
}
