#define _FILE_OFFSET_BITS 64
#define _XOPEN_SOURCE 700

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static int read_token(FastInput *in, char *token, size_t capacity) {
    int c = next_char(in);
    while (c != EOF && c <= ' ') {
        c = next_char(in);
    }
    if (c == EOF) {
        return 0;
    }

    size_t len = 0;
    while (c != EOF && c > ' ') {
        if (len + 1 < capacity) {
            token[len++] = (char)c;
        }
        c = next_char(in);
    }
    token[len] = '\0';
    return 1;
}

static long long parse_ll(const char *s) {
    int sign = 1;
    long long value = 0;
    if (*s == '-') {
        sign = -1;
        ++s;
    } else if (*s == '+') {
        ++s;
    }
    while (*s != '\0') {
        value = value * 10 + (*s - '0');
        ++s;
    }
    return sign * value;
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

static int read_int_at(int fd, long long index, int *value) {
    unsigned char *ptr = (unsigned char *)value;
    size_t done = 0;
    off_t offset = (off_t)(index * (long long)sizeof(int));
    while (done < sizeof(int)) {
        ssize_t got = pread(fd, ptr + done, sizeof(int) - done, offset + (off_t)done);
        if (got <= 0) {
            return 0;
        }
        done += (size_t)got;
    }
    return 1;
}

static int write_int_at(int fd, long long index, int value) {
    const unsigned char *ptr = (const unsigned char *)&value;
    size_t done = 0;
    off_t offset = (off_t)(index * (long long)sizeof(int));
    while (done < sizeof(int)) {
        ssize_t wrote = pwrite(fd, ptr + done, sizeof(int) - done, offset + (off_t)done);
        if (wrote <= 0) {
            return 0;
        }
        done += (size_t)wrote;
    }
    return 1;
}

int main(void) {
    int file_mode = 0;
    FILE *input = open_input(&file_mode);
    FILE *output_file = open_output(file_mode);
    FastInput in = {input, {0}, 0, 0};
    FastOutput out = {output_file, {0}, 0};

    long long q = 0;
    if (!read_ll(&in, &q)) {
        return 0;
    }

    const char *temp_name = "p25z4_stack.bin";
    int stack_file = open(temp_name, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (stack_file < 0) {
        return 1;
    }

    long long size = 0;
    int top_value = 0;
    char token[32];

    for (long long i = 0; i < q && read_token(&in, token, sizeof(token)); ++i) {
        if (token[0] == '+') {
            long long value = 0;
            if (token[1] != '\0') {
                value = parse_ll(token + 1);
            } else if (!read_ll(&in, &value)) {
                break;
            }
            top_value = (int)value;
            write_int_at(stack_file, size, top_value);
            ++size;
        } else if (token[0] == '-') {
            if (size > 0) {
                int value = 0;
                read_int_at(stack_file, size - 1, &value);
                --size;
                write_ll(&out, value);
                write_char(&out, '\n');
                if (size > 0) {
                    read_int_at(stack_file, size - 1, &top_value);
                }
            }
        } else {
            if (size > 0) {
                write_ll(&out, top_value);
                write_char(&out, '\n');
            }
        }
    }

    flush_output(&out);
    close(stack_file);
    remove(temp_name);

    if (input != stdin) {
        fclose(input);
    }
    if (output_file != stdout) {
        fclose(output_file);
    }
    return 0;
}
