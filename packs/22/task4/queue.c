#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

struct queue {
    int *data;
    size_t size;
    size_t capacity;
    size_t head;
};

static int queue_reserve(queue_t *queue, size_t min_capacity)
{
    size_t new_capacity;
    size_t i;
    int *new_data;

    if (queue == NULL) {
        return 0;
    }
    if (queue->capacity >= min_capacity) {
        return 1;
    }

    new_capacity = queue->capacity == 0 ? 1 : queue->capacity;
    while (new_capacity < min_capacity) {
        if (new_capacity > ((size_t) -1) / 2) {
            new_capacity = min_capacity;
            break;
        }
        new_capacity *= 2;
    }

    new_data = (int *) malloc(new_capacity * sizeof(*new_data));
    if (new_data == NULL) {
        return 0;
    }

    for (i = 0; i < queue->size; ++i) {
        new_data[i] = queue->data[(queue->head + i) % (queue->capacity == 0 ? 1 : queue->capacity)];
    }

    free(queue->data);
    queue->data = new_data;
    queue->capacity = new_capacity;
    queue->head = 0;
    return 1;
}

queue_t *queue_create(void)
{
    return (queue_t *) calloc(1, sizeof(queue_t));
}

queue_t *queue_new(void)
{
    return queue_create();
}

void queue_destroy(queue_t *queue)
{
    if (queue == NULL) {
        return;
    }
    free(queue->data);
    free(queue);
}

void queue_delete(queue_t *queue)
{
    queue_destroy(queue);
}

size_t queue_size(const queue_t *queue)
{
    return queue == NULL ? 0 : queue->size;
}

bool queue_empty(const queue_t *queue)
{
    return queue == NULL || queue->size == 0;
}

bool queue_push(queue_t *queue, int value)
{
    size_t tail;

    if (queue == NULL) {
        return false;
    }
    if (!queue_reserve(queue, queue->size + 1)) {
        return false;
    }

    tail = (queue->head + queue->size) % queue->capacity;
    queue->data[tail] = value;
    queue->size++;
    return true;
}

int queue_pop(queue_t *queue)
{
    int value;

    if (queue == NULL || queue->size == 0) {
        return 0;
    }

    value = queue->data[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;
    if (queue->size == 0) {
        queue->head = 0;
    }
    return value;
}

int queue_peek(const queue_t *queue)
{
    if (queue == NULL || queue->size == 0) {
        return 0;
    }
    return queue->data[queue->head];
}

void queue_clear(queue_t *queue)
{
    if (queue == NULL) {
        return;
    }
    queue->size = 0;
    queue->head = 0;
}

void queue_print(const queue_t *queue)
{
    size_t i;

    printf("[");
    if (queue != NULL) {
        for (i = 0; i < queue->size; ++i) {
            if (i != 0) {
                printf(", ");
            }
            printf("%d", queue->data[(queue->head + i) % queue->capacity]);
        }
    }
    printf("]\n");
}
