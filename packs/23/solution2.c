#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int cmp(const void *a, const void *b) {
    double diff = *(double*)a - *(double*)b;
    if (diff < 0) return -1;
    if (diff > 0) return 1;
    return 0;
}

int main() {
    FILE *fin = fopen("input.txt", "r");
    FILE *fout = fopen("output.txt", "w");

    int n;
    fscanf(fin, "%d", &n);

    double *x = (double*)malloc(n * sizeof(double));
    double *y = (double*)malloc(n * sizeof(double));

    for (int i = 0; i < n; i++) {
        fscanf(fin, "%lf %lf", &x[i], &y[i]);
    }

    // Сортируем координаты
    qsort(x, n, sizeof(double), cmp);
    qsort(y, n, sizeof(double), cmp);

    // Находим медиану
    double px, py;
    if (n % 2 == 1) {
        px = x[n / 2];
        py = y[n / 2];
    } else {
        px = (x[n / 2 - 1] + x[n / 2]) / 2.0;
        py = (y[n / 2 - 1] + y[n / 2]) / 2.0;
    }

    // Вычисляем сумму расстояний
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += fabs(x[i] - px) + fabs(y[i] - py);
    }

    fprintf(fout, "%.1lf %.1lf\n%.1lf\n", px, py, sum);

    free(x);
    free(y);
    fclose(fin);
    fclose(fout);

    return 0;
}
