/* Greatest common divisor via the Euclidean algorithm. */
#include <stdio.h>

int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

int main(void) {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    for (int i = 0; i < T; i++) {
        int a, b;
        scanf("%d %d", &a, &b);
        printf("%d\n", gcd(a, b));
    }
    return 0;
}
