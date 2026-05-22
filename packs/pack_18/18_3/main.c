#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Лексикографическое сравнение пар и троек
static inline bool leq2(int a1, int a2, int b1, int b2) {
    return (a1 < b1 || (a1 == b1 && a2 <= b2));
}

static inline bool leq3(int a1, int a2, int a3, int b1, int b2, int b3) {
    return (a1 < b1 || (a1 == b1 && leq2(a2, a3, b2, b3)));
}

// Устойчивая сортировка подсчетом (Radix Sort)
static void radixPass(int* a, int* b, int* r, int n, int K) {
    int* c = (int*)calloc(K + 1, sizeof(int));
    for (int i = 0; i < n; i++) c[r[a[i]]]++;
    for (int i = 0, sum = 0; i <= K; i++) {
        int t = c[i];
        c[i] = sum;
        sum += t;
    }
    for (int i = 0; i < n; i++) b[c[r[a[i]]]++] = a[i];
    free(c);
}

// Основная рекурсивная функция алгоритма DC3
void suffixArray(int* s, int* SA, int n, int K) {
    int n0 = (n + 2) / 3, n1 = (n + 1) / 3, n2 = n / 3, n02 = n0 + n2;

    // Выделяем память под массивы для текущего шага рекурсии
    int* s12 = (int*)malloc((n02 + 3) * sizeof(int));
    int* SA12 = (int*)malloc((n02 + 3) * sizeof(int));
    int* s0 = (int*)malloc(n0 * sizeof(int));
    int* SA0 = (int*)malloc(n0 * sizeof(int));

    // Важно: зануляем "хвосты" для безопасного сравнения троек
    s12[n02] = s12[n02 + 1] = s12[n02 + 2] = 0;
    SA12[n02] = SA12[n02 + 1] = SA12[n02 + 2] = 0;

    // 1. Выбираем суффиксы, индексы которых не кратны 3 (mod 1 и mod 2)
    for (int i = 0, j = 0; i < n + (n0 - n1); i++) {
        if (i % 3 != 0) s12[j++] = i;
    }

    // Сортируем выбранные суффиксы по первым трем символам
    radixPass(s12, SA12, s + 2, n02, K);
    radixPass(SA12, s12, s + 1, n02, K);
    radixPass(s12, SA12, s, n02, K);

    // Находим лексикографические имена для троек
    int name = 0, c0 = -1, c1 = -1, c2 = -1;
    for (int i = 0; i < n02; i++) {
        if (s[SA12[i]] != c0 || s[SA12[i] + 1] != c1 || s[SA12[i] + 2] != c2) {
            name++;
            c0 = s[SA12[i]];
            c1 = s[SA12[i] + 1];
            c2 = s[SA12[i] + 2];
        }
        if (SA12[i] % 3 == 1) {
            s12[SA12[i] / 3] = name;
        } else {
            s12[SA12[i] / 3 + n0] = name;
        }
    }

    // Рекурсивный вызов, если имена не уникальны
    if (name < n02) {
        suffixArray(s12, SA12, n02, name);
        for (int i = 0; i < n02; i++) s12[SA12[i]] = i + 1;
    } else {
        for (int i = 0; i < n02; i++) SA12[s12[i] - 1] = i;
    }

    // 2. Сортируем суффиксы, индексы которых кратны 3 (mod 0)
    for (int i = 0, j = 0; i < n02; i++) {
        if (SA12[i] < n0) s0[j++] = 3 * SA12[i];
    }
    radixPass(s0, SA0, s, n0, K);

    // 3. Сливаем отсортированные суффиксы mod 0 и mod 1/2
    for (int p = 0, t = n0 - n1, k = 0; k < n; k++) {
        int i = (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2);
        int j = SA0[p];

        bool is_less;
        if (SA12[t] < n0) {
            is_less = leq2(s[i], s12[SA12[t] + n0], s[j], s12[j / 3]);
        } else {
            is_less = leq3(s[i], s[i + 1], s12[SA12[t] - n0 + 1], s[j], s[j + 1], s12[j / 3 + n0]);
        }

        if (is_less) {
            SA[k] = i; t++;
            if (t == n02) {
                for (k++; p < n0; p++, k++) SA[k] = SA0[p];
            }
        } else {
            SA[k] = j; p++;
            if (p == n0) {
                for (k++; t < n02; t++, k++) {
                    SA[k] = (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2);
                }
            }
        }
    }

    // Освобождаем память после слияния
    free(s12);
    free(SA12);
    free(s0);
    free(SA0);
}

int main() {
    FILE *fin = fopen("input.txt", "r");
    if (!fin) return 1;

    // Выделяем память под строку с небольшим запасом (500 000 + 5)
    char* str = (char*)malloc(500005 * sizeof(char));
    if (fscanf(fin, "%500004s", str) != 1) {
        fclose(fin);
        free(str);
        return 1;
    }
    fclose(fin);

    int n = strlen(str);

    // Массив s должен иметь как минимум 3 нулевых элемента в конце
    int* s = (int*)calloc(n + 3, sizeof(int));
    int* SA = (int*)malloc(n * sizeof(int));

    // Кодируем символы как числа от 1 до 26.
    for (int i = 0; i < n; i++) {
        s[i] = str[i] - 'a' + 1;
    }

    // Вызываем алгоритм DC3
    suffixArray(s, SA, n, 26);

    FILE *fout = fopen("output.txt", "w");
    if (fout) {
        for (int i = 0; i < n; i++) {
            fprintf(fout, "%d%s", SA[i], (i == n - 1) ? "" : " ");
        }
        fprintf(fout, "\n");
        fclose(fout);
    }

    // Чистим память в конце
    free(str);
    free(s);
    free(SA);

    return 0;
}