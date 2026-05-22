#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

struct vector {
    int *data;
    size_t size;
    size_t capacity;
};

static bool vector_reallocate(vector_t *vec, size_t new_capacity)
{
    int *new_data;

    if (vec == NULL) {
        return false;
    }
    if (new_capacity == vec->capacity) {
        return true;
    }

    new_data = (int *) realloc(vec->data, new_capacity * sizeof(*new_data));
    if (new_data == NULL && new_capacity != 0) {
        return false;
    }

    vec->data = new_data;
    vec->capacity = new_capacity;
    if (vec->size > new_capacity) {
        vec->size = new_capacity;
    }
    return true;
}

static bool vector_ensure_capacity(vector_t *vec, size_t min_capacity)
{
    size_t new_capacity;

    if (vec == NULL) {
        return false;
    }
    if (vec->capacity >= min_capacity) {
        return true;
    }

    new_capacity = vec->capacity == 0 ? 1 : vec->capacity;
    while (new_capacity < min_capacity) {
        if (new_capacity > ((size_t) -1) / 2) {
            new_capacity = min_capacity;
            break;
        }
        new_capacity *= 2;
    }

    return vector_reallocate(vec, new_capacity);
}

bool vector_reserve(vector_t *vec, size_t min_capacity)
{
    if (vec == NULL) {
        return false;
    }
    if (vec->capacity >= min_capacity) {
        return true;
    }
    return vector_reallocate(vec, min_capacity);
}

vector_t *vector_create(void)
{
    vector_t *vec = (vector_t *) calloc(1, sizeof(*vec));
    return vec;
}

vector_t *vector_new(void)
{
    return vector_create();
}

void vector_destroy(vector_t *vec)
{
    if (vec == NULL) {
        return;
    }
    free(vec->data);
    free(vec);
}

void vector_delete(vector_t *vec)
{
    vector_destroy(vec);
}

size_t vector_size(const vector_t *vec)
{
    return vec == NULL ? 0 : vec->size;
}

size_t vector_capacity(const vector_t *vec)
{
    return vec == NULL ? 0 : vec->capacity;
}

bool vector_empty(const vector_t *vec)
{
    return vec == NULL || vec->size == 0;
}

bool vector_push_back(vector_t *vec, int value)
{
    if (vec == NULL) {
        return false;
    }
    if (!vector_ensure_capacity(vec, vec->size + 1)) {
        return false;
    }
    vec->data[vec->size++] = value;
    return true;
}

bool vector_push(vector_t *vec, int value)
{
    return vector_push_back(vec, value);
}

void vector_pop_back(vector_t *vec)
{
    if (vec == NULL || vec->size == 0) {
        return;
    }
    vec->size--;
}

void vector_pop(vector_t *vec)
{
    vector_pop_back(vec);
}

bool vector_resize(vector_t *vec, size_t new_size, int value)
{
    size_t i;

    if (vec == NULL) {
        return false;
    }
    if (new_size > vec->size) {
        if (!vector_ensure_capacity(vec, new_size)) {
            return false;
        }
        for (i = vec->size; i < new_size; ++i) {
            vec->data[i] = value;
        }
    }
    vec->size = new_size;
    return true;
}

void vector_clear(vector_t *vec)
{
    if (vec == NULL) {
        return;
    }
    vec->size = 0;
}

const int *vector_data(const vector_t *vec)
{
    if (vec == NULL) {
        return NULL;
    }
    return vec->data;
}

bool vector_at(const vector_t *vec, size_t index, int *value)
{
    if (vec == NULL || value == NULL || index >= vec->size) {
        return false;
    }
    *value = vec->data[index];
    return true;
}

int vector_get(const vector_t *vec, size_t index)
{
    int value = 0;

    if (!vector_at(vec, index, &value)) {
        return 0;
    }
    return value;
}

bool vector_front(const vector_t *vec, int *value)
{
    return vector_at(vec, 0, value);
}

bool vector_back(const vector_t *vec, int *value)
{
    if (vec == NULL || value == NULL || vec->size == 0) {
        return false;
    }
    *value = vec->data[vec->size - 1];
    return true;
}

void vector_print(const vector_t *vec)
{
    size_t i;

    if (vec == NULL) {
        printf("[]\n");
        return;
    }

    printf("[");
    for (i = 0; i < vec->size; ++i) {
        if (i != 0) {
            printf(", ");
        }
        printf("%d", vec->data[i]);
    }
    printf("]\n");
}
