#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 500005

int p[MAXN], rk[MAXN], tmp[MAXN], n;
char s[MAXN];

int k_global;

int cmp(const void *a, const void *b) {
    int x = *(int*)a, y = *(int*)b;
    if (rk[x] != rk[y]) return rk[x] - rk[y];
    int rx = x + k_global < n ? rk[x + k_global] : -1;
    int ry = y + k_global < n ? rk[y + k_global] : -1;
    return rx - ry;
}

int main() {
    scanf("%s", s);
    n = strlen(s);

    for (int i = 0; i < n; i++) {
        p[i] = i;
        rk[i] = s[i];
    }

    for (int k = 1; k < n; k <<= 1) {
        k_global = k;
        qsort(p, n, sizeof(int), cmp);

        tmp[p[0]] = 0;
        for (int i = 1; i < n; i++)
            tmp[p[i]] = tmp[p[i-1]] + (cmp(&p[i-1], &p[i]) ? 1 : 0);
        memcpy(rk, tmp, n * sizeof(int));

        if (rk[p[n-1]] == n-1) break;
    }

    for (int i = 0; i < n; i++) {
        printf("%d", p[i]);
        if (i < n-1) printf(" ");
    }
    printf("\n");

    return 0;
}