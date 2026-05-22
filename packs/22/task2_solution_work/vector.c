#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

struct vector {
    int *buffer;
    size_t length;
    size_t reserved;
};

static bool vector_can_access(const vector_t *vec, size_t position)
{
    return vec != NULL && position < vec->length;
}

static size_t vector_capacity_seed(const vector_t *vec)
{
    if (vec == NULL || vec->reserved == 0) {
        return 1;
    }
    return vec->reserved;
}

static bool vector_resize_storage(vector_t *vec, size_t requested_capacity)
{
    int *updated_buffer;

    if (vec == NULL) {
        return false;
    }
    if (requested_capacity == vec->reserved) {
        return true;
    }

    updated_buffer =
        (int *) realloc(vec->buffer, requested_capacity * sizeof(*updated_buffer));
    if (updated_buffer == NULL && requested_capacity != 0) {
        return false;
    }

    vec->buffer = updated_buffer;
    vec->reserved = requested_capacity;
    if (vec->length > requested_capacity) {
        vec->length = requested_capacity;
    }
    return true;
}

static size_t vector_pick_capacity(const vector_t *vec, size_t required_capacity)
{
    size_t candidate_capacity = vector_capacity_seed(vec);

    while (candidate_capacity < required_capacity) {
        if (candidate_capacity > ((size_t) -1) / 2) {
            candidate_capacity = required_capacity;
            break;
        }
        candidate_capacity *= 2;
    }

    return candidate_capacity;
}

static bool vector_make_room(vector_t *vec, size_t required_capacity)
{
    if (vec == NULL) {
        return false;
    }
    if (vec->reserved >= required_capacity) {
        return true;
    }

    return vector_resize_storage(vec, vector_pick_capacity(vec, required_capacity));
}

static void vector_write(vector_t *vec, size_t position, int value)
{
    vec->buffer[position] = value;
}

static void vector_fill_tail(vector_t *vec, size_t first, size_t last, int value)
{
    size_t current = first;

    for (; current < last; ++current) {
        vector_write(vec, current, value);
    }
}

static void vector_set_size(vector_t *vec, size_t new_length)
{
    vec->length = new_length;
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
    free(vec->buffer);
    free(vec);
}

void vector_delete(vector_t *vec)
{
    vector_destroy(vec);
}

size_t vector_size(const vector_t *vec)
{
    return vec == NULL ? 0 : vec->length;
}

size_t vector_capacity(const vector_t *vec)
{
    return vec == NULL ? 0 : vec->reserved;
}

bool vector_empty(const vector_t *vec)
{
    return vec == NULL || vec->length == 0;
}

bool vector_push_back(vector_t *vec, int value)
{
    size_t next_slot;

    if (vec == NULL) {
        return false;
    }
    if (!vector_make_room(vec, vec->length + 1)) {
        return false;
    }

    next_slot = vec->length;
    vector_write(vec, next_slot, value);
    vector_set_size(vec, next_slot + 1);
    return true;
}

bool vector_push(vector_t *vec, int value)
{
    return vector_push_back(vec, value);
}

void vector_pop_back(vector_t *vec)
{
    if (vec != NULL && vec->length != 0) {
        vec->length--;
    }
}

void vector_pop(vector_t *vec)
{
    vector_pop_back(vec);
}

bool vector_resize(vector_t *vec, size_t new_size, int value)
{
    if (vec == NULL) {
        return false;
    }

    if (new_size <= vec->length) {
        vector_set_size(vec, new_size);
        return true;
    }

    if (!vector_make_room(vec, new_size)) {
        return false;
    }

    vector_fill_tail(vec, vec->length, new_size, value);
    vector_set_size(vec, new_size);
    return true;
}

void vector_clear(vector_t *vec)
{
    if (vec != NULL) {
        vec->length = 0;
    }
}

const int *vector_data(const vector_t *vec)
{
    return vec == NULL ? NULL : vec->buffer;
}

bool vector_at(const vector_t *vec, size_t index, int *value)
{
    if (value != NULL && vector_can_access(vec, index)) {
        *value = vec->buffer[index];
        return true;
    }
    return false;
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
    return !vector_empty(vec) && vector_at(vec, 0, value);
}

bool vector_back(const vector_t *vec, int *value)
{
    if (!vector_empty(vec) && value != NULL) {
        *value = vec->buffer[vec->length - 1];
        return true;
    }
    return false;
}

bool vector_reserve(vector_t *vec, size_t min_capacity)
{
    if (vec == NULL) {
        return false;
    }
    if (min_capacity <= vec->reserved) {
        return true;
    }

    return vector_resize_storage(vec, min_capacity);
}

void vector_print(const vector_t *vec)
{
    size_t item_index;

    if (vec == NULL) {
        printf("[]\n");
        return;
    }

    printf("[");
    if (!vector_empty(vec)) {
        printf("%d", vec->buffer[0]);
        for (item_index = 1; item_index < vec->length; ++item_index) {
            printf(", ");
            printf("%d", vec->buffer[item_index]);
        }
    }
    printf("]\n");
}
