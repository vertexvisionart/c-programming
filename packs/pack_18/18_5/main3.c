#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Подключаем json-c без путей, как сказано в условии
#include <json.h>

// --- Простой динамический буфер для записи байтов ---
typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} Buffer;

void buf_init(Buffer *b) {
    b->cap = 1024;
    b->len = 0;
    b->data = (uint8_t*)malloc(b->cap);
}

void buf_append(Buffer *b, const void *data, size_t size) {
    if (size == 0) return;
    while (b->len + size > b->cap) {
        b->cap *= 2;
        b->data = (uint8_t*)realloc(b->data, b->cap);
    }
    memcpy(b->data + b->len, data, size);
    b->len += size;
}

void buf_append_byte(Buffer *b, uint8_t byte) {
    buf_append(b, &byte, 1);
}

void buf_append_int32(Buffer *b, int32_t val) {
    buf_append(b, &val, 4);
}

void buf_append_double(Buffer *b, double val) {
    buf_append(b, &val, 8);
}

void buf_append_cstr(Buffer *b, const char *str) {
    buf_append(b, str, strlen(str) + 1);
}

// Предварительное объявление для взаимной рекурсии
void serialize_document(Buffer *b, struct json_object *obj);

// Определение типа BSON
uint8_t get_bson_type(struct json_object *val) {
    if (!val) return 10; // Null
    switch (json_object_get_type(val)) {
        case json_type_null:    return 10; // Null
        case json_type_boolean: return 8;  // Boolean
        case json_type_double:  return 1;  // Number (Double)
        case json_type_int:     return 1;  // Number (Int maps to Double)
        case json_type_object:  return 3;  // Object
        case json_type_array:   return 4;  // Array
        case json_type_string:  return 2;  // String
        default:                return 10;
    }
}

// Сериализация "полезной нагрузки" значения
void serialize_value_payload(Buffer *b, struct json_object *val) {
    if (!val) return; // Для Null не пишется никаких данных

    switch (json_object_get_type(val)) {
        case json_type_null:
            break;
        case json_type_boolean: {
            uint8_t bool_val = json_object_get_boolean(val) ? 1 : 0;
            buf_append_byte(b, bool_val);
            break;
        }
        case json_type_double:
        case json_type_int: {
            // В условии сказано, что любое число пишется как double (8 байт)
            double d = json_object_get_double(val);
            buf_append_double(b, d);
            break;
        }
        case json_type_string: {
            const char *str = json_object_get_string(val);
            int32_t len = (int32_t)strlen(str) + 1;
            buf_append_int32(b, len);
            buf_append_cstr(b, str);
            break;
        }
        case json_type_object:
        case json_type_array:
            serialize_document(b, val);
            break;
    }
}

// Сериализация целого документа (объекта или массива)
void serialize_document(Buffer *b, struct json_object *obj) {
    size_t start_pos = b->len;
    buf_append_int32(b, 0); // Место под размер документа

    if (obj) {
        enum json_type type = json_object_get_type(obj);
        
        if (type == json_type_object) {
            json_object_object_foreach(obj, key, val) {
                buf_append_byte(b, get_bson_type(val));
                buf_append_cstr(b, key);
                serialize_value_payload(b, val);
            }
        } else if (type == json_type_array) {
            int len = json_object_array_length(obj);
            for (int i = 0; i < len; i++) {
                struct json_object *val = json_object_array_get_idx(obj, i);
                buf_append_byte(b, get_bson_type(val));
                
                // Имя поля в массиве — это его индекс строкой
                char idx_str[32];
                sprintf(idx_str, "%d", i);
                buf_append_cstr(b, idx_str);
                
                serialize_value_payload(b, val);
            }
        }
    }

    buf_append_byte(b, 0); // Нулевой байт завершает документ

    // Возвращаемся и записываем реальный размер
    int32_t total_size = (int32_t)(b->len - start_pos);
    memcpy(b->data + start_pos, &total_size, sizeof(int32_t));
}

int main() {
    // Читаем JSON-файл
    struct json_object *root = json_object_from_file("input.txt");
    if (!root) {
        return 1;
    }

    Buffer b;
    buf_init(&b);

    // Сериализуем корневой элемент
    serialize_document(&b, root);

    // Открываем выходной файл в БИНАРНОМ режиме (wb)
    FILE *fout = fopen("output.txt", "wb");
    if (fout) {
        fwrite(b.data, 1, b.len, fout);
        fclose(fout);
    }

    // Очищаем память
    json_object_put(root);
    free(b.data);

    return 0;
}