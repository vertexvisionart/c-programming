#include <stdio.h>
#include <stdlib.h>

#define INPUT_BUFFER_SIZE (1u << 16)
#define OUTPUT_BUFFER_SIZE (1u << 16)
#define STACK_BLOCK_SIZE 32768u

typedef struct {
    FILE *file;
    unsigned char buffer[INPUT_BUFFER_SIZE];
    size_t pos;
    size_t len;
} FastInput;

typedef struct {
    FILE *file;
    unsigned char buffer[OUTPUT_BUFFER_SIZE];
    size_t len;
} FastOutput;

typedef struct {
    FILE *file;
    int data[STACK_BLOCK_SIZE];
    size_t used;
    size_t disk_blocks;
} ExternalStack;

static int read_byte(FastInput *in) {
    if (in->pos == in->len) {
        in->len = fread(in->buffer, 1, INPUT_BUFFER_SIZE, in->file);
        in->pos = 0;
        if (in->len == 0) {
            return EOF;
        }
    }

    return in->buffer[in->pos++];
}

static int read_int(FastInput *in, int *value) {
    int c;
    int sign = 1;
    int result = 0;

    do {
        c = read_byte(in);
        if (c == EOF) {
            return 0;
        }
    } while (c <= ' ');

    if (c == '-') {
        sign = -1;
        c = read_byte(in);
    }

    while (c >= '0' && c <= '9') {
        result = result * 10 + (c - '0');
        c = read_byte(in);
    }

    *value = result * sign;
    return 1;
}

static int read_operation(FastInput *in) {
    int c;

    do {
        c = read_byte(in);
        if (c == EOF) {
            return EOF;
        }
    } while (c <= ' ');

    return c;
}

static void skip_token_tail(FastInput *in) {
    int c;

    do {
        c = read_byte(in);
    } while (c != EOF && c > ' ');
}

static void write_byte(FastOutput *out, unsigned char c) {
    if (out->len == OUTPUT_BUFFER_SIZE) {
        fwrite(out->buffer, 1, out->len, out->file);
        out->len = 0;
    }

    out->buffer[out->len++] = c;
}

static void write_int_line(FastOutput *out, int value) {
    char digits[16];
    int count = 0;
    long long x = value;

    if (x < 0) {
        write_byte(out, '-');
        x = -x;
    }

    if (x == 0) {
        write_byte(out, '0');
    } else {
        while (x > 0) {
            digits[count++] = (char)('0' + (x % 10));
            x /= 10;
        }

        while (count > 0) {
            write_byte(out, (unsigned char)digits[--count]);
        }
    }

    write_byte(out, '\n');
}

static void flush_output(FastOutput *out) {
    if (out->len > 0) {
        fwrite(out->buffer, 1, out->len, out->file);
        out->len = 0;
    }
}

static long stack_block_offset(size_t block_index) {
    return (long)(block_index * STACK_BLOCK_SIZE * sizeof(int));
}

static void save_full_block(ExternalStack *stack) {
    fseek(stack->file, stack_block_offset(stack->disk_blocks), SEEK_SET);
    fwrite(stack->data, sizeof(int), STACK_BLOCK_SIZE, stack->file);
    stack->disk_blocks++;
    stack->used = 0;
}

static void load_previous_block(ExternalStack *stack) {
    if (stack->used == 0 && stack->disk_blocks > 0) {
        stack->disk_blocks--;
        fseek(stack->file, stack_block_offset(stack->disk_blocks), SEEK_SET);
        fread(stack->data, sizeof(int), STACK_BLOCK_SIZE, stack->file);
        stack->used = STACK_BLOCK_SIZE;
    }
}

static void stack_push(ExternalStack *stack, int value) {
    if (stack->used == STACK_BLOCK_SIZE) {
        save_full_block(stack);
    }

    stack->data[stack->used++] = value;
}

static int stack_top(ExternalStack *stack) {
    load_previous_block(stack);
    return stack->data[stack->used - 1];
}

static int stack_pop(ExternalStack *stack) {
    int value;

    load_previous_block(stack);
    value = stack->data[stack->used - 1];
    stack->used--;

    return value;
}

int main(void) {
    FILE *input_file = fopen("input.txt", "rb");
    FILE *output_file;
    FILE *stack_file;
    int remove_stack_file = 0;
    FastInput input = {0};
    FastOutput output = {0};
    ExternalStack stack = {0};
    int q;
    int i;

    if (input_file != NULL) {
        output_file = fopen("output.txt", "wb");
        if (output_file == NULL) {
            fclose(input_file);
            return 1;
        }
    } else {
        input_file = stdin;
        output_file = stdout;
    }

    stack_file = tmpfile();
    if (stack_file == NULL) {
        stack_file = fopen("stack.tmp", "w+b");
        remove_stack_file = 1;
    }
    if (stack_file == NULL) {
        if (input_file != stdin) {
            fclose(input_file);
        }
        if (output_file != stdout) {
            fclose(output_file);
        }
        return 1;
    }

    setvbuf(stack_file, NULL, _IONBF, 0);

    input.file = input_file;
    output.file = output_file;
    stack.file = stack_file;

    if (!read_int(&input, &q)) {
        fclose(stack_file);
        if (input_file != stdin) {
            fclose(input_file);
        }
        if (output_file != stdout) {
            fclose(output_file);
        }
        if (remove_stack_file) {
            remove("stack.tmp");
        }
        return 0;
    }

    for (i = 0; i < q; i++) {
        int op = read_operation(&input);

        if (op == '+') {
            int value;
            read_int(&input, &value);
            stack_push(&stack, value);
        } else if (op == '-') {
            write_int_line(&output, stack_pop(&stack));
        } else if (op == '?' || op == 't') {
            if (op == 't') {
                skip_token_tail(&input);
            }
            write_int_line(&output, stack_top(&stack));
        }
    }

    flush_output(&output);

    fclose(stack_file);
    if (input_file != stdin) {
        fclose(input_file);
    }
    if (output_file != stdout) {
        fclose(output_file);
    }
    if (remove_stack_file) {
        remove("stack.tmp");
    }

    return 0;
}
