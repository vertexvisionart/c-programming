#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THRESHOLD 32

typedef long long Value;

typedef struct {
    const Value *a;
    const Value *b;
    Value *c;
    int n;
    Value mod;
    int depth;
} StrassenTask;

static Value norm_mod(__int128 x, Value mod) {
    if (mod == 1) {
        return 0;
    }
    x %= mod;
    if (x < 0) {
        x += mod;
    }
    return (Value)x;
}

static void add_block(const Value *a,
                      int sa,
                      const Value *b,
                      int sb,
                      Value *out,
                      int n,
                      Value mod,
                      int sign) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            out[i * n + j] =
                norm_mod((__int128)a[i * sa + j] + sign * (__int128)b[i * sb + j], mod);
        }
    }
}

static void classical(const Value *a,
                      int sa,
                      const Value *b,
                      int sb,
                      Value *c,
                      int sc,
                      int n,
                      Value mod) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            __int128 sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += (__int128)a[i * sa + k] * b[k * sb + j];
                sum %= mod;
            }
            c[i * sc + j] = norm_mod(sum, mod);
        }
    }
}

static Value *new_matrix(int n) {
    return (Value *)calloc((size_t)n * (size_t)n, sizeof(Value));
}

static void strassen(const Value *a,
                     int sa,
                     const Value *b,
                     int sb,
                     Value *c,
                     int sc,
                     int n,
                     Value mod,
                     int depth);

static void *strassen_thread(void *arg) {
    StrassenTask *task = (StrassenTask *)arg;
    strassen(task->a, task->n, task->b, task->n, task->c, task->n, task->n,
             task->mod, task->depth);
    return NULL;
}

static void combine(Value *c,
                    int sc,
                    const Value *m1,
                    const Value *m2,
                    const Value *m3,
                    const Value *m4,
                    const Value *m5,
                    const Value *m6,
                    const Value *m7,
                    int n,
                    Value mod) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int idx = i * n + j;
            c[i * sc + j] = norm_mod((__int128)m1[idx] + m4[idx] - m5[idx] + m7[idx], mod);
            c[i * sc + j + n] = norm_mod((__int128)m3[idx] + m5[idx], mod);
            c[(i + n) * sc + j] = norm_mod((__int128)m2[idx] + m4[idx], mod);
            c[(i + n) * sc + j + n] =
                norm_mod((__int128)m1[idx] - m2[idx] + m3[idx] + m6[idx], mod);
        }
    }
}

static void strassen(const Value *a,
                     int sa,
                     const Value *b,
                     int sb,
                     Value *c,
                     int sc,
                     int n,
                     Value mod,
                     int depth) {
    if (n <= THRESHOLD || mod == 1) {
        classical(a, sa, b, sb, c, sc, n, mod);
        return;
    }

    int k = n / 2;
    const Value *a11 = a;
    const Value *a12 = a + k;
    const Value *a21 = a + k * sa;
    const Value *a22 = a + k * sa + k;
    const Value *b11 = b;
    const Value *b12 = b + k;
    const Value *b21 = b + k * sb;
    const Value *b22 = b + k * sb + k;

    Value *left[7];
    Value *right[7];
    Value *m[7];
    for (int i = 0; i < 7; ++i) {
        left[i] = new_matrix(k);
        right[i] = new_matrix(k);
        m[i] = new_matrix(k);
        if (left[i] == NULL || right[i] == NULL || m[i] == NULL) {
            exit(1);
        }
    }

    add_block(a11, sa, a22, sa, left[0], k, mod, 1);
    add_block(b11, sb, b22, sb, right[0], k, mod, 1);

    add_block(a21, sa, a22, sa, left[1], k, mod, 1);
    memcpy(right[1], b11, (size_t)k * sizeof(Value));
    for (int i = 1; i < k; ++i) {
        memcpy(right[1] + i * k, b11 + i * sb, (size_t)k * sizeof(Value));
    }

    memcpy(left[2], a11, (size_t)k * sizeof(Value));
    for (int i = 1; i < k; ++i) {
        memcpy(left[2] + i * k, a11 + i * sa, (size_t)k * sizeof(Value));
    }
    add_block(b12, sb, b22, sb, right[2], k, mod, -1);

    memcpy(left[3], a22, (size_t)k * sizeof(Value));
    for (int i = 1; i < k; ++i) {
        memcpy(left[3] + i * k, a22 + i * sa, (size_t)k * sizeof(Value));
    }
    add_block(b21, sb, b11, sb, right[3], k, mod, -1);

    add_block(a11, sa, a12, sa, left[4], k, mod, 1);
    memcpy(right[4], b22, (size_t)k * sizeof(Value));
    for (int i = 1; i < k; ++i) {
        memcpy(right[4] + i * k, b22 + i * sb, (size_t)k * sizeof(Value));
    }

    add_block(a21, sa, a11, sa, left[5], k, mod, -1);
    add_block(b11, sb, b12, sb, right[5], k, mod, 1);

    add_block(a12, sa, a22, sa, left[6], k, mod, -1);
    add_block(b21, sb, b22, sb, right[6], k, mod, 1);

    pthread_t threads[7];
    StrassenTask tasks[7];
    int started[7] = {0};
    int parallel = (depth == 0 && n >= 128);

    for (int i = 0; i < 7; ++i) {
        tasks[i].a = left[i];
        tasks[i].b = right[i];
        tasks[i].c = m[i];
        tasks[i].n = k;
        tasks[i].mod = mod;
        tasks[i].depth = depth + 1;
        if (parallel &&
            pthread_create(&threads[i], NULL, strassen_thread, &tasks[i]) == 0) {
            started[i] = 1;
        } else {
            strassen(left[i], k, right[i], k, m[i], k, k, mod, depth + 1);
        }
    }
    for (int i = 0; i < 7; ++i) {
        if (started[i]) {
            pthread_join(threads[i], NULL);
        }
    }

    combine(c, sc, m[0], m[1], m[2], m[3], m[4], m[5], m[6], k, mod);

    for (int i = 0; i < 7; ++i) {
        free(left[i]);
        free(right[i]);
        free(m[i]);
    }
}

static FILE *open_input(int *file_mode) {
    FILE *f = fopen("input.txt", "rb");
    if (f != NULL) {
        *file_mode = 1;
        return f;
    }
    *file_mode = 0;
    return stdin;
}

static FILE *open_output(int file_mode) {
    if (file_mode) {
        FILE *f = fopen("output.txt", "wb");
        if (f != NULL) {
            return f;
        }
    }
    return stdout;
}

int main(void) {
    int file_mode = 0;
    FILE *in = open_input(&file_mode);
    FILE *out = open_output(file_mode);

    int n = 0;
    Value mod = 0;
    if (fscanf(in, "%d %lld", &n, &mod) != 2 || n <= 0 || mod <= 0) {
        return 0;
    }

    Value *a = new_matrix(n);
    Value *b = new_matrix(n);
    Value *c = new_matrix(n);
    if (a == NULL || b == NULL || c == NULL) {
        free(a);
        free(b);
        free(c);
        return 1;
    }

    for (int i = 0; i < n * n; ++i) {
        long long x;
        fscanf(in, "%lld", &x);
        a[i] = norm_mod(x, mod);
    }
    for (int i = 0; i < n * n; ++i) {
        long long x;
        fscanf(in, "%lld", &x);
        b[i] = norm_mod(x, mod);
    }

    strassen(a, n, b, n, c, n, n, mod, 0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (j > 0) {
                fputc(' ', out);
            }
            fprintf(out, "%lld", c[i * n + j]);
        }
        fputc('\n', out);
    }

    free(a);
    free(b);
    free(c);
    if (in != stdin) {
        fclose(in);
    }
    if (out != stdout) {
        fclose(out);
    }
    return 0;
}
