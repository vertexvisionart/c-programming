#include <stdio.h>

#define IN_BUF_SIZE 65536

typedef struct {
    FILE *file;
    unsigned char buf[IN_BUF_SIZE];
    size_t pos;
    size_t len;
} FastInput;

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
    FILE *output = open_output(file_mode);
    FastInput in = {input, {0}, 0, 0};

    long long n = 0;
    long long mod = 0;
    if (!read_ll(&in, &n) || !read_ll(&in, &mod) || mod == 0) {
        return 0;
    }

    long long sum = 0;
    for (long long i = 0; i < n; ++i) {
        long long x = 0;
        if (!read_ll(&in, &x)) {
            break;
        }
        x %= mod;
        if (x < 0) {
            x += mod;
        }
        sum += x;
        sum %= mod;
    }

    fprintf(output, "%lld\n", sum);

    if (input != stdin) {
        fclose(input);
    }
    if (output != stdout) {
        fclose(output);
    }
    return 0;
}
