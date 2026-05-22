#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vector vector_t;

vector_t *vector_create(void);
void vector_destroy(vector_t *vec);
vector_t *vector_new(void);
void vector_delete(vector_t *vec);

size_t vector_size(const vector_t *vec);
size_t vector_capacity(const vector_t *vec);
bool vector_empty(const vector_t *vec);

bool vector_push_back(vector_t *vec, int value);
bool vector_push(vector_t *vec, int value);
void vector_pop_back(vector_t *vec);
void vector_pop(vector_t *vec);
bool vector_reserve(vector_t *vec, size_t min_capacity);
bool vector_resize(vector_t *vec, size_t new_size, int value);
void vector_clear(vector_t *vec);

const int *vector_data(const vector_t *vec);
bool vector_at(const vector_t *vec, size_t index, int *value);
int vector_get(const vector_t *vec, size_t index);
bool vector_front(const vector_t *vec, int *value);
bool vector_back(const vector_t *vec, int *value);
void vector_print(const vector_t *vec);

#ifdef __cplusplus
}
#endif

#endif
