#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 1000000000U

typedef struct {
    uint32_t *d;
    size_t len;
    size_t cap;
} BigInt;

typedef struct {
    const BigInt *a;
    const BigInt *b;
    BigInt *out;
} MulTask;

static void bigint_init(BigInt *x) {
    x->d = NULL;
    x->len = 0;
    x->cap = 0;
}

static void bigint_free(BigInt *x) {
    free(x->d);
    x->d = NULL;
    x->len = 0;
    x->cap = 0;
}

static int bigint_reserve(BigInt *x, size_t need) {
    if (x->cap >= need) {
        return 1;
    }
    size_t cap = x->cap ? x->cap : 1;
    while (cap < need) {
        cap *= 2;
    }
    uint32_t *p = (uint32_t *)realloc(x->d, cap * sizeof(uint32_t));
    if (p == NULL) {
        return 0;
    }
    x->d = p;
    x->cap = cap;
    return 1;
}

static void bigint_trim(BigInt *x) {
    while (x->len > 0 && x->d[x->len - 1] == 0) {
        --x->len;
    }
}

static int bigint_set_u32(BigInt *x, uint32_t value) {
    if (value == 0) {
        x->len = 0;
        return 1;
    }
    if (!bigint_reserve(x, 1)) {
        return 0;
    }
    x->d[0] = value;
    x->len = 1;
    return 1;
}

static void bigint_move(BigInt *dst, BigInt *src) {
    bigint_free(dst);
    *dst = *src;
    bigint_init(src);
}

static int bigint_add(BigInt *out, const BigInt *a, const BigInt *b) {
    size_t n = a->len > b->len ? a->len : b->len;
    if (!bigint_reserve(out, n + 1)) {
        return 0;
    }

    uint64_t carry = 0;
    for (size_t i = 0; i < n; ++i) {
        uint64_t cur = carry;
        if (i < a->len) {
            cur += a->d[i];
        }
        if (i < b->len) {
            cur += b->d[i];
        }
        out->d[i] = (uint32_t)(cur % BASE);
        carry = cur / BASE;
    }
    out->len = n;
    if (carry) {
        out->d[out->len++] = (uint32_t)carry;
    }
    return 1;
}

static int bigint_sub(BigInt *out, const BigInt *a, const BigInt *b) {
    if (!bigint_reserve(out, a->len)) {
        return 0;
    }

    int64_t borrow = 0;
    for (size_t i = 0; i < a->len; ++i) {
        int64_t cur = (int64_t)a->d[i] - borrow - (i < b->len ? b->d[i] : 0);
        if (cur < 0) {
            cur += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        out->d[i] = (uint32_t)cur;
    }
    out->len = a->len;
    bigint_trim(out);
    return 1;
}

static int bigint_mul(BigInt *out, const BigInt *a, const BigInt *b) {
    if (a->len == 0 || b->len == 0) {
        out->len = 0;
        return 1;
    }

    size_t need = a->len + b->len + 1;
    if (!bigint_reserve(out, need)) {
        return 0;
    }
    memset(out->d, 0, need * sizeof(uint32_t));

    for (size_t i = 0; i < a->len; ++i) {
        unsigned __int128 carry = 0;
        for (size_t j = 0; j < b->len; ++j) {
            unsigned __int128 cur =
                (unsigned __int128)a->d[i] * b->d[j] + out->d[i + j] + carry;
            out->d[i + j] = (uint32_t)(cur % BASE);
            carry = cur / BASE;
        }
        size_t pos = i + b->len;
        while (carry > 0) {
            unsigned __int128 cur = (unsigned __int128)out->d[pos] + carry;
            out->d[pos] = (uint32_t)(cur % BASE);
            carry = cur / BASE;
            ++pos;
        }
    }

    out->len = need;
    bigint_trim(out);
    return 1;
}

static void *mul_thread(void *arg) {
    MulTask *task = (MulTask *)arg;
    bigint_mul(task->out, task->a, task->b);
    return NULL;
}

static void run_three_muls(const BigInt *a,
                           const BigInt *diff,
                           const BigInt *b,
                           BigInt *c,
                           BigInt *aa,
                           BigInt *bb) {
    if (a->len < 64) {
        bigint_mul(c, a, diff);
        bigint_mul(aa, a, a);
        bigint_mul(bb, b, b);
        return;
    }

    MulTask tasks[3] = {{a, diff, c}, {a, a, aa}, {b, b, bb}};
    pthread_t threads[3];
    int started[3] = {0, 0, 0};

    for (int i = 0; i < 3; ++i) {
        if (pthread_create(&threads[i], NULL, mul_thread, &tasks[i]) == 0) {
            started[i] = 1;
        } else {
            bigint_mul(tasks[i].out, tasks[i].a, tasks[i].b);
        }
    }
    for (int i = 0; i < 3; ++i) {
        if (started[i]) {
            pthread_join(threads[i], NULL);
        }
    }
}

static void fib_pair(unsigned int n, BigInt *fn, BigInt *fn1) {
    if (n == 0) {
        bigint_set_u32(fn, 0);
        bigint_set_u32(fn1, 1);
        return;
    }

    BigInt a, b;
    bigint_init(&a);
    bigint_init(&b);
    fib_pair(n / 2, &a, &b);

    BigInt two_b, diff, c, aa, bb, d, sum;
    bigint_init(&two_b);
    bigint_init(&diff);
    bigint_init(&c);
    bigint_init(&aa);
    bigint_init(&bb);
    bigint_init(&d);
    bigint_init(&sum);

    bigint_add(&two_b, &b, &b);
    bigint_sub(&diff, &two_b, &a);
    run_three_muls(&a, &diff, &b, &c, &aa, &bb);
    bigint_add(&d, &aa, &bb);

    if ((n & 1U) == 0) {
        bigint_move(fn, &c);
        bigint_move(fn1, &d);
    } else {
        bigint_add(&sum, &c, &d);
        bigint_move(fn, &d);
        bigint_move(fn1, &sum);
    }

    bigint_free(&a);
    bigint_free(&b);
    bigint_free(&two_b);
    bigint_free(&diff);
    bigint_free(&c);
    bigint_free(&aa);
    bigint_free(&bb);
    bigint_free(&d);
    bigint_free(&sum);
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

static void bigint_print(FILE *out, const BigInt *x) {
    if (x->len == 0) {
        fprintf(out, "0\n");
        return;
    }
    fprintf(out, "%u", x->d[x->len - 1]);
    for (size_t i = x->len - 1; i-- > 0;) {
        fprintf(out, "%09u", x->d[i]);
    }
    fputc('\n', out);
}

int main(void) {
    int file_mode = 0;
    FILE *in = open_input(&file_mode);
    FILE *out = open_output(file_mode);

    unsigned int n = 0;
    if (fscanf(in, "%u", &n) != 1) {
        return 0;
    }

    BigInt fn, fn1;
    bigint_init(&fn);
    bigint_init(&fn1);
    fib_pair(n, &fn, &fn1);
    bigint_print(out, &fn);

    bigint_free(&fn);
    bigint_free(&fn1);
    if (in != stdin) {
        fclose(in);
    }
    if (out != stdout) {
        fclose(out);
    }
    return 0;
}
