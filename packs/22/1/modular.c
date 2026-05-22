#include "modular.h"

int MOD;

int pnorm(int a) {
    int res = a % MOD;
    if (res < 0) {
        res += MOD;
    }
    return res;
}

int padd(int a, int b) {
    // Так как a и b < MOD, их сумма максимум 2*MOD - 2.
    // Это влезает в int, и вычесть MOD намного быстрее, чем брать остаток (%).
    int res = a + b;
    if (res >= MOD) {
        res -= MOD;
    }
    return res;
}

int psub(int a, int b) {
    int res = a - b;
    if (res < 0) {
        res += MOD;
    }
    return res;
}

int pmul(int a, int b) {
    // Здесь обязательно long long, так как a * b может быть порядка 10^18
    return (int)(((long long)a * b) % MOD);
}

// Быстрое возведение в степень (O(log exp))
static int ppow(int base, int exp) {
    long long res = 1;
    long long b = base;
    while (exp > 0) {
        if (exp % 2 == 1) {
            res = (res * b) % MOD;
        }
        b = (b * b) % MOD;
        exp /= 2;
    }
    return (int)res;
}

int pdiv(int a, int b) {
    return pmul(a, ppow(b, MOD - 2));
}