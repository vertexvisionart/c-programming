#include <stdio.h>

#define IN_BUF_SIZE 65536
#define OUT_BUF_SIZE 65536

typedef struct {
    FILE *file;
    unsigned char buf[IN_BUF_SIZE];
    size_t pos;
    size_t len;
} FastInput;

typedef struct {
    FILE *file;
    unsigned char buf[OUT_BUF_SIZE];
    size_t pos;
} FastOutput;

static int next_char(FastInput *in) {
    if (in->pos >= in->len) {
        in->len = fread(in->buf, 1, IN_BUF_SIZE, in->file);
        in->pos = 0;
        if (in->len == 0) {
            return EOF;
        }
    }
    return in->buf[in->pos++];
}

static int read_ll(FastInput *in, long long *value) {
    int c = next_char(in);
    while (c != EOF && c <= ' ') {
        c = next_char(in);
    }
    if (c == EOF) {
        return 0;
    }

    int sign = 1;
    if (c == '-') {
        sign = -1;
        c = next_char(in);
    }

    long long result = 0;
    while (c != EOF && c > ' ') {
        result = result * 10 + (c - '0');
        c = next_char(in);
    }

    *value = sign * result;
    return 1;
}

static void flush_output(FastOutput *out) {
    if (out->pos > 0) {
        fwrite(out->buf, 1, out->pos, out->file);
        out->pos = 0;
    }
}

static void write_char(FastOutput *out, int c) {
    if (out->pos == OUT_BUF_SIZE) {
        flush_output(out);
    }
    out->buf[out->pos++] = (unsigned char)c;
}

static void write_str(FastOutput *out, const char *s) {
    while (*s != '\0') {
        write_char(out, *s);
        ++s;
    }
}

static void write_ll(FastOutput *out, long long value) {
    char tmp[32];
    int len = 0;
    if (value == 0) {
        write_char(out, '0');
        return;
    }
    if (value < 0) {
        write_char(out, '-');
        value = -value;
    }
    while (value > 0) {
        tmp[len++] = (char)('0' + value % 10);
        value /= 10;
    }
    while (len > 0) {
        write_char(out, tmp[--len]);
    }
}

static FILE *open_input(int *from_file) {
    FILE *file = fopen("input.txt", "rb");
    if (file != NULL) {
        *from_file = 1;
        return file;
    }
    *from_file = 0;
    return stdin;
}

static FILE *open_output(int to_file) {
    if (to_file) {
        FILE *file = fopen("output.txt", "wb");
        if (file != NULL) {
            return file;
        }
    }
    return stdout;
}

int main(void) {
    int file_mode = 0;
    FILE *input = open_input(&file_mode);
    FILE *output_file = open_output(file_mode);
    FastInput in = {input, {0}, 0, 0};
    FastOutput out = {output_file, {0}, 0};

    long long n = 0;
    long long x = 0;
    if (!read_ll(&in, &n) || !read_ll(&in, &x)) {
        return 0;
    }

    long long first = -1;
    long long last = -1;
    for (long long i = 0; i < n; ++i) {
        long long value = 0;
        if (!read_ll(&in, &value)) {
            break;
        }

        if (value == x) {
            if (first < 0) {
                first = i;
            }
            last = i;
        } else if (value > x) {
            break;
        }
    }

    if (first < 0) {
        write_str(&out, "NO\n");
    } else {
        long long count = last - first + 1;
        write_ll(&out, count);
        write_char(&out, '\n');
        for (long long i = first; i <= last; ++i) {
            if (i > first) {
                write_char(&out, ' ');
            }
            write_ll(&out, i);
        }
        write_char(&out, '\n');
    }

    flush_output(&out);
    if (input != stdin) {
        fclose(input);
    }
    if (output_file != stdout) {
        fclose(output_file);
    }
    return 0;
}
