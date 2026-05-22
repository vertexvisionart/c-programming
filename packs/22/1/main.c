#include "modular.h"
#include <assert.h>

int main() {
    // --- 1. Базовый модуль ---
    MOD = 13;
    assert(pnorm(45) == 6);
    assert(pnorm(-4) == 9);
    assert(pnorm(13) == 0);   // Нормализация самого модуля
    assert(pnorm(-13) == 0);  // Нормализация отрицательного модуля
    assert(pnorm(0) == 0);
    assert(padd(7, 8) == 2);
    assert(padd(0, 0) == 0);
    assert(psub(2, 5) == 10);
    assert(psub(5, 5) == 0);  // Вычитание в ноль
    assert(psub(0, 12) == 1);
    assert(pmul(3, 4) == 12);
    assert(pmul(0, 5) == 0);  // Умножение на ноль
    assert(pdiv(7, 4) == 5);
    assert(pdiv(1, 2) == 7);
    assert(pdiv(0, 5) == 0);  // Деление нуля
    assert(pdiv(6, 6) == 1);  // Деление числа на само себя
    
    // --- 2. Коварный модуль 2 (Ловит баги нулевой степени в Ферма) ---
    MOD = 2;
    assert(pnorm(5) == 1);
    assert(pnorm(-1) == 1);
    assert(pnorm(-2) == 0);
    assert(pnorm(0) == 0);
    assert(padd(1, 1) == 0);
    assert(psub(0, 1) == 1);
    assert(psub(1, 1) == 0);
    assert(pmul(1, 1) == 1);
    assert(pdiv(1, 1) == 1);  // Если степень MOD-2 не обработана, тут будет ошибка!
    assert(pdiv(0, 1) == 0);

    // --- 3. Средний модуль (Ловит переполнение int без вызова TLE) ---
    MOD = 100003;
    assert(pdiv(100000, 2) == 50000);
    assert(pdiv(1, 100002) == 100002);
    assert(pdiv(50, 50) == 1);
    assert(pmul(100000, 100000) == 999999937); // Гарантированное переполнение 32-bit int
    assert(pmul(100002, 100002) == 1);
    
    // --- 4. Большой модуль (Ловит баги отрицательных чисел) ---
    MOD = 1000000007;
    // Если нормализация сделана через 1 if (a < 0) a += MOD, то -1000000008 даст баг
    assert(pnorm(-1) == 1000000006);
    assert(pnorm(-1000000008) == 1000000006);
    assert(pnorm(1000000008) == 1);
    assert(pnorm(1000000000) == 1000000000);
    
    assert(padd(1000000006, 5) == 4);
    assert(psub(2, 5) == 1000000004);
    assert(psub(0, 1000000006) == 1);
    assert(psub(1000000006, 1000000006) == 0);
    assert(pmul(1000000006, 2) == 1000000005);
    
    return 0;
}