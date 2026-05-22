#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Суффиксный массив: prefix doubling + radix sort, O(n log n).
 * Достаточно быстро для n=500000 в 0.5с.
 */

#define MAXN 500005

static int sa[MAXN];   /* суффиксный массив */
static int rk[MAXN];   /* rank[i] = ранг суффикса начиная с i */
static int tmp[MAXN];  /* временный массив рангов */
static int cnt[MAXN];  /* счётчики для radix sort */
static int buf[MAXN];  /* временный буфер для radix sort */

static int n;

/* Одна сортировка по ключу key[sa[i]] */
static void rsort(int *sa_out, int *key, int sz, int maxv) {
    int i;
    /* обнуление cnt */
    for (i = 0; i <= maxv; i++) cnt[i] = 0;
    for (i = 0; i < n; i++) cnt[key[sa[i]] < 0 ? 0 : key[sa[i]] + 1]++;
    /* cnt[0] — элементы с key[.] < 0 (суффиксы за концом, ранг считаем -1) */
    for (i = 1; i <= maxv + 1; i++) cnt[i] += cnt[i-1];
    for (i = 0; i < n; i++) {
        int k = key[sa[i]];
        buf[cnt[k < 0 ? 0 : k]++] = sa[i];
    }
    memcpy(sa_out, buf, n * sizeof(int));
}

static void build_sa(const char *s_str) {
    int i, h;
    n = (int)strlen(s_str);

    /* Шаг 0: инициализация */
    for (i = 0; i < n; i++) sa[i] = i;
    for (i = 0; i < n; i++) rk[i] = s_str[i] - 'a';

    /* Prefix doubling */
    for (h = 1; h < n; h <<= 1) {
        /* Сортируем по паре (rk[i], rk[i+h]).
           Radix sort: сначала по вторичному ключу rk[i+h],
                       затем по первичному rk[i] (стабильно). */

        /* 1) Сортировка по rk[i+h].
              sa_h[j] = позиции, отсортированные по rk[pos+h].
              rk[pos+h] для pos+h >= n считаем -1 (меньше всех). */
        {
            int maxv = 0;
            for (i = 0; i < n; i++) if (rk[i] > maxv) maxv = rk[i];
            /* Обнуляем */
            for (i = 0; i <= maxv + 1; i++) cnt[i] = 0;
            /* Позиции, для которых pos+h >= n, ранг вторичного ключа = -1 */
            for (i = 0; i < n; i++) {
                int v = (sa[i] + h < n) ? rk[sa[i] + h] + 1 : 0;
                cnt[v]++;
            }
            for (i = 1; i <= maxv + 1; i++) cnt[i] += cnt[i-1];
            /* Заполняем в обратном порядке для стабильности */
            for (i = n - 1; i >= 0; i--) {
                int v = (sa[i] + h < n) ? rk[sa[i] + h] + 1 : 0;
                buf[--cnt[v]] = sa[i];
            }
            memcpy(sa, buf, n * sizeof(int));
        }

        /* 2) Стабильная сортировка по rk[i] */
        {
            int maxv = 0;
            for (i = 0; i < n; i++) if (rk[i] > maxv) maxv = rk[i];
            for (i = 0; i <= maxv; i++) cnt[i] = 0;
            for (i = 0; i < n; i++) cnt[rk[sa[i]]]++;
            for (i = 1; i <= maxv; i++) cnt[i] += cnt[i-1];
            for (i = n - 1; i >= 0; i--) buf[--cnt[rk[sa[i]]]] = sa[i];
            memcpy(sa, buf, n * sizeof(int));
        }

        /* 3) Пересчитываем ранги */
        tmp[sa[0]] = 0;
        for (i = 1; i < n; i++) {
            int prev = sa[i-1], curr = sa[i];
            int r2_prev = (prev + h < n) ? rk[prev + h] : -1;
            int r2_curr = (curr + h < n) ? rk[curr + h] : -1;
            if (rk[prev] == rk[curr] && r2_prev == r2_curr)
                tmp[curr] = tmp[prev];
            else
                tmp[curr] = tmp[prev] + 1;
        }
        memcpy(rk, tmp, n * sizeof(int));

        /* Если все ранги уникальны — готово */
        if (rk[sa[n-1]] == n - 1) break;
    }
}

static char s[MAXN];

int main(void) {
    freopen("input.txt",  "r", stdin);
    freopen("output.txt", "w", stdout);

    if (!fgets(s, MAXN, stdin)) return 1;
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1]=='\n'||s[len-1]=='\r'||s[len-1]==' ')) len--;
    s[len] = '\0';

    build_sa(s);

    int i;
    for (i = 0; i < n; i++) {
        printf("%d", sa[i]);
        if (i < n-1) putchar(' ');
    }
    putchar('\n');
    return 0;
}