#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list list_t;

list_t *list_create(void);
void list_destroy(list_t *list);
list_t *list_new(void);
void list_delete(list_t *list);

size_t list_size(const list_t *list);
bool list_empty(const list_t *list);
bool list_is_empty(const list_t *list);

bool list_push_back(list_t *list, int value);
bool list_push_front(list_t *list, int value);
bool list_pop_back(list_t *list, int *value);
bool list_pop_front(list_t *list, int *value);
bool list_front(const list_t *list, int *value);
bool list_back(const list_t *list, int *value);
int list_at(const list_t *list, size_t index);
int list_get(const list_t *list, size_t index);
void list_clear(list_t *list);
void list_print(const list_t *list);

#ifdef __cplusplus
}
#endif

#endif
