#include "modular.h"

// Определение глобальной переменной
int MOD;

int pnorm(int a) {
    int res = a % MOD;
    // Если число отрицательное, остаток тоже может быть отрицательным в Си.
    // Приводим его к диапазону [0, MOD - 1]
    if (res < 0) {
        res += MOD;
    }
    return res;
}

int padd(int a, int b) {
    // Приводим к long long для избежания переполнения при сложении
    long long res = (long long)a + b;
    return (int)(res % MOD);
}

int psub(int a, int b) {
    long long res = (long long)a - b;
    if (res < 0) {
        res += MOD;
    }
    return (int)res;
}

int pmul(int a, int b) {
    // Умножение может дать до 10^18, поэтому обязательно используем long long
    long long res = (long long)a * b;
    return (int)(res % MOD);
}

// Вспомогательная функция для быстрого возведения в степень по модулю
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
    // Деление a / b эквивалентно a * b^(MOD - 2) по простому модулю (Малая теорема Ферма)
    int b_inv = ppow(b, MOD - 2);
    return pmul(a, b_inv);
}