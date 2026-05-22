#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vector vector_t;

vector_t *vector_new(void);
vector_t *vector_create(void);
void vector_delete(vector_t *vec);
void vector_destroy(vector_t *instance);

bool vector_push(vector_t *instance, int element);
bool vector_push_back(vector_t *instance, int element);
void vector_pop(vector_t *instance);
void vector_pop_back(vector_t *instance);
void vector_clear(vector_t *instance);

bool vector_reserve(vector_t *instance, size_t required_capacity);
bool vector_resize(vector_t *instance, size_t element_count, int fill_element);

bool vector_empty(const vector_t *instance);
size_t vector_size(const vector_t *instance);
size_t vector_capacity(const vector_t *instance);

bool vector_at(const vector_t *instance, size_t position, int *destination);
int vector_get(const vector_t *instance, size_t position);
bool vector_front(const vector_t *instance, int *destination);
bool vector_back(const vector_t *instance, int *destination);
const int *vector_data(const vector_t *instance);
void vector_print(const vector_t *instance);

#ifdef __cplusplus
}
#endif

#endif
