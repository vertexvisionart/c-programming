#include "modular.h"

#include <assert.h>

int main(void)
{
    int a;
    int b;

    MOD = 13;
    assert(pnorm(45) == 6);
    assert(pnorm(0) == 0);
    assert(pnorm(12) == 12);
    assert(pnorm(13) == 0);
    assert(pnorm(26) == 0);
    assert(pnorm(27) == 1);
    assert(pnorm(-1) == 12);
    assert(pnorm(-13) == 0);
    assert(pnorm(-14) == 12);
    assert(pnorm(-45) == 7);

    assert(padd(0, 0) == 0);
    assert(padd(7, 6) == 0);
    assert(padd(7, 12) == 6);
    assert(padd(12, 12) == 11);

    assert(psub(3, 5) == 11);
    assert(psub(2, 3) == 12);
    assert(psub(8, 8) == 0);

    assert(pmul(0, 7) == 0);
    assert(pmul(7, 0) == 0);
    assert(pmul(9, 12) == 4);
    assert(pmul(12, 12) == 1);

    assert(pdiv(7, 4) == 5);
    assert(pmul(4, pdiv(7, 4)) == 7);
    assert(pdiv(1, 2) == 7);
    assert(pdiv(0, 5) == 0);
    for (a = 1; a < 13; ++a) {
        assert(pdiv(a, 1) == a);
        assert(pdiv(a, a) == 1);
        assert(pmul(a, pdiv(1, a)) == 1);
    }
    for (a = 0; a < 13; ++a) {
        for (b = 1; b < 13; ++b) {
            assert(pmul(pdiv(a, b), b) == a);
        }
    }

    MOD = 17;
    assert(pnorm(34) == 0);
    assert(pnorm(-17) == 0);
    assert(pnorm(-1) == 16);
    assert(padd(16, 1) == 0);
    assert(psub(0, 1) == 16);
    assert(pmul(8, 9) == 4);
    assert(pdiv(3, 5) == 4);
    assert(pmul(5, pdiv(1, 5)) == 1);

    MOD = 2;
    assert(pnorm(5) == 1);
    assert(pnorm(4) == 0);
    assert(pnorm(-2) == 0);
    assert(pnorm(-1) == 1);
    assert(padd(1, 1) == 0);
    assert(psub(0, 1) == 1);
    assert(pmul(0, 1) == 0);
    assert(pmul(1, 1) == 1);
    assert(pdiv(1, 1) == 1);

    MOD = 1000000007;
    assert(pnorm(-1000000000) == 7);
    assert(pnorm(1000000000) == 1000000000);
    assert(pnorm(1000000007) == 0);
    assert(pnorm(-1000000007) == 0);
    assert(pnorm(-1000000008) == 1000000006);
    assert(pnorm(999999999) == 999999999);
    assert(padd(1000000000, 1000000000) == 999999993);
    assert(pmul(1000000000, 1000000000) == 49);
    assert(padd(1000000006, 1) == 0);
    assert(psub(0, 1) == 1000000006);
    assert(psub(0, 1000000000) == 7);
    assert(psub(1000000000, 1000000000) == 0);
    assert(pmul(1000000006, 2) == 1000000005);

    return 0;
}
