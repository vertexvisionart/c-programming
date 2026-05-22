#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_N 10000000
#define CHUNK 131072
#define MAX_RUNS ((MAX_N + CHUNK - 1) / CHUNK + 2)
#define IN_BUF_SIZE 8192
#define OUT_BUF_SIZE 8192
#define RUN_BUF 256

typedef struct {
    off_t off;
    int left;
    int pos;
    int len;
    int buf[RUN_BUF];
} Run;

static unsigned char in_buf[IN_BUF_SIZE];
static char out_buf[OUT_BUF_SIZE];
static int chunk[CHUNK];
static Run runs[MAX_RUNS];
static off_t run_off[MAX_RUNS];
static int run_len[MAX_RUNS];
static int heap[MAX_RUNS];
static int cur[MAX_RUNS];

static int in_fd = -1;
static int out_fd = -1;
static int tmp_fd = -1;
static int in_pos = 0;
static int in_len = 0;
static int out_pos = 0;
static int heap_size = 0;

static int nc(void) {
    if (in_pos >= in_len) {
        ssize_t got = read(in_fd, in_buf, sizeof(in_buf));
        if (got <= 0) return EOF;
        in_len = (int)got;
        in_pos = 0;
    }
    return in_buf[in_pos++];
}

static int read_int(int *v) {
    int c = nc();
    while (c != EOF && c <= ' ') c = nc();
    if (c == EOF) return 0;

    int sign = 1;
    if (c == '-') {
        sign = -1;
        c = nc();
    }

    long x = 0;
    while (c != EOF && c > ' ') {
        x = x * 10 + (c - '0');
        c = nc();
    }
    *v = (int)(sign * x);
    return 1;
}

static void isort(int *a, int l, int r) {
    for (int i = l + 1; i <= r; ++i) {
        int x = a[i];
        int j = i - 1;
        while (j >= l && a[j] > x) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = x;
    }
}

static void sort_ints(int *a, int n) {
    struct Range {
        int l;
        int r;
    } st[64];
    int top = 0;

    if (n <= 1) return;
    st[top++] = (struct Range){0, n - 1};

    while (top > 0) {
        int l = st[--top].l;
        int r = st[top].r;

        while (r - l > 24) {
            int m = l + (r - l) / 2;
            if (a[m] < a[l]) {
                int t = a[m]; a[m] = a[l]; a[l] = t;
            }
            if (a[r] < a[l]) {
                int t = a[r]; a[r] = a[l]; a[l] = t;
            }
            if (a[r] < a[m]) {
                int t = a[r]; a[r] = a[m]; a[m] = t;
            }

            int pivot = a[m];
            int i = l;
            int j = r;
            while (i <= j) {
                while (a[i] < pivot) ++i;
                while (a[j] > pivot) --j;
                if (i <= j) {
                    int t = a[i]; a[i] = a[j]; a[j] = t;
                    ++i;
                    --j;
                }
            }

            if (j - l < r - i) {
                if (i < r) st[top++] = (struct Range){i, r};
                r = j;
            } else {
                if (l < j) st[top++] = (struct Range){l, j};
                l = i;
            }
        }

        if (l < r) isort(a, l, r);
    }
}

static int write_all(int fd, const void *buf, size_t n) {
    const char *p = (const char *)buf;
    while (n > 0) {
        ssize_t done = write(fd, p, n);
        if (done <= 0) return 0;
        p += done;
        n -= (size_t)done;
    }
    return 1;
}

static int pread_all(int fd, void *buf, size_t n, off_t off) {
    char *p = (char *)buf;
    while (n > 0) {
        ssize_t got = pread(fd, p, n, off);
        if (got <= 0) return 0;
        p += got;
        off += got;
        n -= (size_t)got;
    }
    return 1;
}

static void out_flush(void) {
    if (out_pos > 0) {
        write_all(out_fd, out_buf, (size_t)out_pos);
        out_pos = 0;
    }
}

static void out_char(char c) {
    if (out_pos >= OUT_BUF_SIZE) out_flush();
    out_buf[out_pos++] = c;
}

static void out_text(const char *s, int len) {
    for (int i = 0; i < len; ++i) out_char(s[i]);
}

static int int_to_str(int v, char *out) {
    char tmp[12];
    int len = 0;
    int pos = 0;
    unsigned int u;

    if (v < 0) {
        out[pos++] = '-';
        u = (unsigned int)(-(long long)v);
    } else {
        u = (unsigned int)v;
    }

    if (u == 0) {
        out[pos++] = '0';
        return pos;
    }

    while (u > 0) {
        tmp[len++] = (char)('0' + (u % 10));
        u /= 10;
    }
    while (len > 0) out[pos++] = tmp[--len];
    return pos;
}

static int run_next(int id, int *value) {
    Run *r = &runs[id];
    if (r->pos >= r->len) {
        if (r->left <= 0) return 0;

        int take = r->left < RUN_BUF ? r->left : RUN_BUF;
        size_t bytes = (size_t)take * sizeof(int);
        if (!pread_all(tmp_fd, r->buf, bytes, r->off)) return 0;

        r->off += (off_t)bytes;
        r->left -= take;
        r->pos = 0;
        r->len = take;
    }

    *value = r->buf[r->pos++];
    return 1;
}

static int heap_less(int a, int b) {
    if (cur[a] != cur[b]) return cur[a] < cur[b];
    return a < b;
}

static void heap_push(int id) {
    int i = heap_size++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (heap_less(heap[p], id)) break;
        heap[i] = heap[p];
        i = p;
    }
    heap[i] = id;
}

static int heap_pop(void) {
    int res = heap[0];
    int x = heap[--heap_size];
    int i = 0;
    while (1) {
        int l = i * 2 + 1;
        int r = l + 1;
        if (l >= heap_size) break;
        int child = r < heap_size && heap_less(heap[r], heap[l]) ? r : l;
        if (heap_less(x, heap[child])) break;
        heap[i] = heap[child];
        i = child;
    }
    if (heap_size > 0) heap[i] = x;
    return res;
}

int main(void) {
    in_fd = open("input.txt", O_RDONLY);
    if (in_fd < 0) return 1;

    int n;
    if (!read_int(&n) || n <= 0) {
        close(in_fd);
        out_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd >= 0) close(out_fd);
        return 0;
    }

    tmp_fd = open(".p25z3.tmp", O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (tmp_fd < 0) {
        close(in_fd);
        return 1;
    }

    int remaining = n;
    int run_count = 0;
    off_t off = 0;
    while (remaining > 0) {
        int cnt = remaining < CHUNK ? remaining : CHUNK;
        int got = 0;
        while (got < cnt && read_int(&chunk[got])) ++got;
        if (got == 0) break;

        sort_ints(chunk, got);

        size_t bytes = (size_t)got * sizeof(int);
        if (run_count >= MAX_RUNS || !write_all(tmp_fd, chunk, bytes)) {
            close(in_fd);
            close(tmp_fd);
            unlink(".p25z3.tmp");
            return 1;
        }

        run_off[run_count] = off;
        run_len[run_count] = got;
        off += (off_t)bytes;
        remaining -= got;
        ++run_count;
    }

    close(in_fd);

    out_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        close(tmp_fd);
        unlink(".p25z3.tmp");
        return 1;
    }

    for (int i = 0; i < run_count; ++i) {
        runs[i].off = run_off[i];
        runs[i].left = run_len[i];
        runs[i].pos = 0;
        runs[i].len = 0;
        if (run_next(i, &cur[i])) heap_push(i);
    }

    int first = 1;
    while (heap_size > 0) {
        int id = heap_pop();
        char text[12];
        if (!first) out_char(' ');
        int len = int_to_str(cur[id], text);
        out_text(text, len);
        first = 0;

        if (run_next(id, &cur[id])) heap_push(id);
    }
    out_char('\n');
    out_flush();

    close(out_fd);
    close(tmp_fd);
    unlink(".p25z3.tmp");
    return 0;
}
