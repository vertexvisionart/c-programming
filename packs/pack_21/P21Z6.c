#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum {
    INDEX_BITS = 19,
    VALUE_BITS = 30
};

static void packed_set(uint32_t *arr, uint64_t bit_pos, int bits, uint32_t value) {
    int word = (int)(bit_pos >> 5);
    int shift = (int)(bit_pos & 31u);
    uint64_t mask = (1ULL << bits) - 1ULL;

    arr[word] &= (uint32_t)~(mask << shift);
    arr[word] |= (uint32_t)((uint64_t)value << shift);
    if (shift + bits > 32) {
        int rem = shift + bits - 32;
        arr[word + 1] &= (uint32_t)~((1ULL << rem) - 1ULL);
        arr[word + 1] |= (uint32_t)((uint64_t)value >> (bits - rem));
    }
}

static uint32_t packed_get(const uint32_t *arr, uint64_t bit_pos, int bits) {
    int word = (int)(bit_pos >> 5);
    int shift = (int)(bit_pos & 31u);
    uint64_t mask = (1ULL << bits) - 1ULL;
    uint64_t value = (uint64_t)arr[word] >> shift;

    if (shift + bits > 32) {
        int rem = shift + bits - 32;
        value |= (uint64_t)(arr[word + 1] & ((1u << rem) - 1u)) << (bits - rem);
    }
    return (uint32_t)(value & mask);
}

static int parent_bits(int index) {
    if (index <= 2) {
        return 1;
    }
    return 32 - __builtin_clz((unsigned)(index - 1));
}

static uint64_t parent_offset_bits(int index) {
    int remaining = index - 1;
    int bits = 1;
    uint64_t total = 0;

    if (remaining <= 0) {
        return 0;
    }

    int first = remaining < 2 ? remaining : 2;
    total += (uint64_t)first;
    remaining -= first;
    bits = 2;

    while (remaining > 0) {
        int cnt = 1 << (bits - 1);
        if (cnt > remaining) {
            cnt = remaining;
        }
        total += (uint64_t)cnt * (uint64_t)bits;
        remaining -= cnt;
        ++bits;
    }

    return total;
}

static void parent_set(uint32_t *arr, int index, uint32_t value) {
    packed_set(arr, parent_offset_bits(index), parent_bits(index), value);
}

static uint32_t parent_get(const uint32_t *arr, int index) {
    if (index == 0) {
        return 0;
    }
    return packed_get(arr, parent_offset_bits(index), parent_bits(index));
}

static void value_set(uint32_t *arr, int index, uint32_t value) {
    packed_set(arr, (uint64_t)(index - 1) * VALUE_BITS, VALUE_BITS, value);
}

static uint32_t value_get(const uint32_t *arr, int index) {
    return packed_get(arr, (uint64_t)(index - 1) * VALUE_BITS, VALUE_BITS);
}

static void write_u24_fd(int fd, int index, uint32_t x) {
    unsigned char b[3];
    b[0] = (unsigned char)(x & 255u);
    b[1] = (unsigned char)((x >> 8) & 255u);
    b[2] = (unsigned char)((x >> 16) & 255u);
    lseek(fd, (off_t)index * 3, SEEK_SET);
    write(fd, b, 3);
}

static uint32_t read_u24_fd(int fd, int index) {
    unsigned char b[3];
    lseek(fd, (off_t)index * 3, SEEK_SET);
    read(fd, b, 3);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16);
}

int main(void) {
    int q;
    if (scanf("%d", &q) != 1) {
        return 0;
    }

    int top_fd = open("P21Z6_top.bin", O_CREAT | O_TRUNC | O_RDWR, 0600);
    if (top_fd < 0) {
        return 0;
    }

    int max_nodes = q + 1;
    size_t parent_words = (size_t)(parent_offset_bits(max_nodes) + parent_bits(max_nodes) + 31u) / 32u + 1u;
    size_t value_words = ((size_t)q * VALUE_BITS + 31u) / 32u + 1u;

    uint32_t *parent = (uint32_t *)calloc(parent_words, sizeof(uint32_t));
    uint32_t *value = (uint32_t *)calloc(value_words, sizeof(uint32_t));
    if (parent == NULL || value == NULL) {
        free(parent);
        free(value);
        close(top_fd);
        return 0;
    }

    int version_count = 0;
    int node_count = 0;
    write_u24_fd(top_fd, 0, 0);

    for (int step = 0; step < q; ++step) {
        char op;
        scanf(" %c", &op);

        if (op == '+') {
            uint32_t x;
            uint32_t prev_top;
            scanf("%u", &x);
            prev_top = read_u24_fd(top_fd, version_count);

            ++node_count;
            parent_set(parent, node_count, prev_top);
            value_set(value, node_count, x);

            ++version_count;
            write_u24_fd(top_fd, version_count, (uint32_t)node_count);
        } else if (op == '-') {
            uint32_t prev_top = read_u24_fd(top_fd, version_count);
            uint32_t new_top = parent_get(parent, (int)prev_top);

            ++version_count;
            write_u24_fd(top_fd, version_count, new_top);
        } else {
            int v;
            scanf("%d", &v);
            uint32_t cur = read_u24_fd(top_fd, v);
            if (cur == 0) {
                printf("-\n");
            } else {
                int first = 1;
                while (cur != 0) {
                    if (!first) {
                        putchar(' ');
                    }
                    first = 0;
                    printf("%u", value_get(value, (int)cur));
                    cur = parent_get(parent, (int)cur);
                }
                putchar('\n');
            }
        }
    }

    free(parent);
    free(value);
    close(top_fd);
    remove("P21Z6_top.bin");
    return 0;
}
