#ifndef P26_QUEUE_H
#define P26_QUEUE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*task_func_t)(void *arg);

typedef struct task {
    task_func_t function;
    void *arg;
} task_t;

typedef struct queue queue_t;

queue_t *queue_create(size_t capacity);
queue_t *queue_new(size_t capacity);
void queue_destroy(queue_t *queue);
void queue_delete(queue_t *queue);

int queue_push(queue_t *queue, task_t task);
int queue_try_push(queue_t *queue, task_t task);
int queue_pop(queue_t *queue, task_t *task);
int queue_try_pop(queue_t *queue, task_t *task);
void queue_close(queue_t *queue);

size_t queue_size(queue_t *queue);
int queue_empty(queue_t *queue);
int queue_closed(queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif
