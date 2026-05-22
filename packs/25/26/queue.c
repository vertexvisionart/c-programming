#include "queue.h"

#include <pthread.h>
#include <stdlib.h>

struct queue {
    task_t *items;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
    int closed;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

queue_t *queue_create(size_t capacity) {
    if (capacity == 0) {
        capacity = 1;
    }

    queue_t *queue = (queue_t *)calloc(1, sizeof(queue_t));
    if (queue == NULL) {
        return NULL;
    }
    queue->items = (task_t *)calloc(capacity, sizeof(task_t));
    if (queue->items == NULL) {
        free(queue);
        return NULL;
    }

    queue->capacity = capacity;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    return queue;
}

queue_t *queue_new(size_t capacity) {
    return queue_create(capacity);
}

void queue_close(queue_t *queue) {
    if (queue == NULL) {
        return;
    }
    pthread_mutex_lock(&queue->mutex);
    queue->closed = 1;
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
}

void queue_destroy(queue_t *queue) {
    if (queue == NULL) {
        return;
    }
    queue_close(queue);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    free(queue->items);
    free(queue);
}

void queue_delete(queue_t *queue) {
    queue_destroy(queue);
}

int queue_push(queue_t *queue, task_t task) {
    if (queue == NULL) {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    while (!queue->closed && queue->size == queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    if (queue->closed) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    queue->items[queue->tail] = task;
    queue->tail = (queue->tail + 1) % queue->capacity;
    ++queue->size;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    return 1;
}

int queue_try_push(queue_t *queue, task_t task) {
    if (queue == NULL) {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    if (queue->closed || queue->size == queue->capacity) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    queue->items[queue->tail] = task;
    queue->tail = (queue->tail + 1) % queue->capacity;
    ++queue->size;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    return 1;
}

int queue_pop(queue_t *queue, task_t *task) {
    if (queue == NULL || task == NULL) {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    while (!queue->closed && queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    if (queue->size == 0 && queue->closed) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    *task = queue->items[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    --queue->size;
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    return 1;
}

int queue_try_pop(queue_t *queue, task_t *task) {
    if (queue == NULL || task == NULL) {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    *task = queue->items[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    --queue->size;
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    return 1;
}

size_t queue_size(queue_t *queue) {
    if (queue == NULL) {
        return 0;
    }
    pthread_mutex_lock(&queue->mutex);
    size_t size = queue->size;
    pthread_mutex_unlock(&queue->mutex);
    return size;
}

int queue_empty(queue_t *queue) {
    return queue_size(queue) == 0;
}

int queue_closed(queue_t *queue) {
    if (queue == NULL) {
        return 1;
    }
    pthread_mutex_lock(&queue->mutex);
    int closed = queue->closed;
    pthread_mutex_unlock(&queue->mutex);
    return closed;
}
