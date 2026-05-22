#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <json.h>

typedef struct {
    uint8_t *data;
    size_t   size;
    size_t   cap;
} Buf;

static void buf_init(Buf *b) {
    b->cap  = 4096;
    b->data = (uint8_t *)malloc(b->cap);
    b->size = 0;
}

static void buf_need(Buf *b, size_t n) {
    while (b->size + n > b->cap) b->cap *= 2;
    b->data = (uint8_t *)realloc(b->data, b->cap);
}

static void buf_u8(Buf *b, uint8_t v) {
    buf_need(b, 1);
    b->data[b->size++] = v;
}

static void buf_i32(Buf *b, int32_t v) {
    buf_need(b, 4);
    b->data[b->size++] = (uint8_t)(v);
    b->data[b->size++] = (uint8_t)(v >> 8);
    b->data[b->size++] = (uint8_t)(v >> 16);
    b->data[b->size++] = (uint8_t)(v >> 24);
}

static void buf_f64(Buf *b, double v) {
    buf_need(b, 8);
    memcpy(b->data + b->size, &v, 8);
    b->size += 8;
}

static void buf_cstr(Buf *b, const char *s) {
    size_t len = strlen(s);
    buf_need(b, len + 1);
    memcpy(b->data + b->size, s, len + 1);
    b->size += len + 1;
}

static void buf_set_i32(Buf *b, size_t offset, int32_t v) {
    b->data[offset]   = (uint8_t)(v);
    b->data[offset+1] = (uint8_t)(v >> 8);
    b->data[offset+2] = (uint8_t)(v >> 16);
    b->data[offset+3] = (uint8_t)(v >> 24);
}

static uint8_t bson_type_of(struct json_object *obj) {
    switch (json_object_get_type(obj)) {
        case json_type_string:  return 2;
        case json_type_double:
        case json_type_int:
        case json_type_boolean: return 8;
        case json_type_null:    return 10;
        case json_type_array:   return 4;
        case json_type_object:  return 3;
        default:                return 10;
    }
}

static void write_document(Buf *b, struct json_object *obj);

static void write_value(Buf *b, struct json_object *obj) {
    switch (json_object_get_type(obj)) {
        case json_type_string: {
            const char *s = json_object_get_string(obj);
            int32_t len = (int32_t)(strlen(s) + 1);
            buf_i32(b, len);
            buf_cstr(b, s);
            break;
        }
        case json_type_double:
        case json_type_int:
            buf_f64(b, json_object_get_double(obj));
            break;
        case json_type_boolean:
            buf_u8(b, json_object_get_boolean(obj) ? 1 : 0);
            break;
        case json_type_null:
            break;
        case json_type_array:
        case json_type_object:
            write_document(b, obj);
            break;
        default:
            break;
    }
}

static void write_document(Buf *b, struct json_object *obj) {
    size_t start = b->size;
    buf_i32(b, 0);

    if (json_object_get_type(obj) == json_type_array) {
        int i, n = json_object_array_length(obj);
        for (i = 0; i < n; i++) {
            struct json_object *elem = json_object_array_get_idx(obj, i);
            char key[32];
            snprintf(key, sizeof(key), "%d", i);
            buf_u8(b, bson_type_of(elem));
            buf_cstr(b, key);
            write_value(b, elem);
        }
    } else {
        struct json_object_iterator it  = json_object_iter_begin(obj);
        struct json_object_iterator end = json_object_iter_end(obj);
        while (!json_object_iter_equal(&it, &end)) {
            const char         *key = json_object_iter_peek_name(&it);
            struct json_object *val = json_object_iter_peek_value(&it);
            buf_u8(b, bson_type_of(val));
            buf_cstr(b, key);
            write_value(b, val);
            json_object_iter_next(&it);
        }
    }

    buf_u8(b, 0);
    buf_set_i32(b, start, (int32_t)(b->size - start));
}

int main(void) {
    struct json_object *root = json_object_from_file("input.txt");
    if (!root) {
        fprintf(stderr, "Failed to parse input.txt\n");
        return 1;
    }

    Buf b;
    buf_init(&b);
    write_document(&b, root);

    FILE *out = fopen("output.txt", "wb");
    if (!out) { perror("fopen output.txt"); return 1; }
    fwrite(b.data, 1, b.size, out);
    fclose(out);

    free(b.data);
    json_object_put(root);
    return 0;
}