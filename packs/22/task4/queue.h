#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct queue queue_t;

queue_t *queue_create(void);
void queue_destroy(queue_t *queue);
queue_t *queue_new(void);
void queue_delete(queue_t *queue);

size_t queue_size(const queue_t *queue);
bool queue_empty(const queue_t *queue);

bool queue_push(queue_t *queue, int value);
int queue_pop(queue_t *queue);
int queue_peek(const queue_t *queue);
void queue_clear(queue_t *queue);
void queue_print(const queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif
