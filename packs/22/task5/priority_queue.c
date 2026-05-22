#include "priority_queue.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

struct priority_queue {
    pq_item_t *items;
    size_t size;
    size_t capacity;
    size_t storage_capacity;
    size_t sequence;
    size_t *order;
};

static bool pq_has_higher_priority(const priority_queue_t *queue, size_t lhs, size_t rhs)
{
    if (queue->items[lhs].priority != queue->items[rhs].priority) {
        return queue->items[lhs].priority > queue->items[rhs].priority;
    }
    return queue->order[lhs] < queue->order[rhs];
}

static void pq_swap(priority_queue_t *queue, size_t lhs, size_t rhs)
{
    pq_item_t item_tmp = queue->items[lhs];
    size_t order_tmp = queue->order[lhs];

    queue->items[lhs] = queue->items[rhs];
    queue->order[lhs] = queue->order[rhs];
    queue->items[rhs] = item_tmp;
    queue->order[rhs] = order_tmp;
}

static void pq_heapify_up(priority_queue_t *queue, size_t index)
{
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (pq_has_higher_priority(queue, parent, index)) {
            break;
        }
        pq_swap(queue, parent, index);
        index = parent;
    }
}

static void pq_heapify_down(priority_queue_t *queue, size_t index)
{
    while (1) {
        size_t left = index * 2 + 1;
        size_t right = left + 1;
        size_t best = index;

        if (left < queue->size && pq_has_higher_priority(queue, left, best)) {
            best = left;
        }
        if (right < queue->size && pq_has_higher_priority(queue, right, best)) {
            best = right;
        }
        if (best == index) {
            break;
        }
        pq_swap(queue, index, best);
        index = best;
    }
}

static bool pq_reserve(priority_queue_t *queue, size_t new_capacity)
{
    pq_item_t *new_items;
    size_t *new_order;

    if (queue == NULL) {
        return false;
    }
    if (queue->storage_capacity >= new_capacity) {
        return true;
    }

    new_items = (pq_item_t *) realloc(queue->items, new_capacity * sizeof(*new_items));
    if (new_items == NULL) {
        return false;
    }

    new_order = (size_t *) realloc(queue->order, new_capacity * sizeof(*new_order));
    if (new_order == NULL) {
        queue->items = new_items;
        return false;
    }

    queue->items = new_items;
    queue->order = new_order;
    queue->storage_capacity = new_capacity;
    return true;
}

priority_queue_t *pq_create(size_t capacity)
{
    priority_queue_t *queue = (priority_queue_t *) calloc(1, sizeof(*queue));
    if (queue == NULL) {
        return NULL;
    }
    queue->capacity = capacity;
    if (capacity != 0 && !pq_reserve(queue, capacity)) {
        free(queue);
        return NULL;
    }
    return queue;
}

void pq_clear(priority_queue_t *queue, pq_free_func free_func)
{
    size_t i;

    if (queue == NULL) {
        return;
    }

    if (free_func != NULL) {
        for (i = 0; i < queue->size; ++i) {
            free_func(queue->items[i].data);
        }
    }

    queue->size = 0;
    queue->sequence = 0;

    if (queue->capacity == 0) {
        free(queue->items);
        free(queue->order);
        queue->items = NULL;
        queue->order = NULL;
        queue->storage_capacity = 0;
    }
}

void pq_destroy(priority_queue_t *queue, pq_free_func free_func)
{
    if (queue == NULL) {
        return;
    }
    pq_clear(queue, free_func);
    free(queue->items);
    free(queue->order);
    free(queue);
}

bool pq_push(priority_queue_t *queue, void *data, int priority)
{
    size_t target_capacity;

    if (queue == NULL) {
        return false;
    }
    if (queue->capacity != 0 && queue->size >= queue->capacity) {
        return false;
    }
    if (queue->size == SIZE_MAX || queue->sequence == SIZE_MAX) {
        return false;
    }

    if (queue->size == queue->storage_capacity) {
        target_capacity = queue->storage_capacity == 0 ? 1 : queue->storage_capacity * 2;
        if (target_capacity < queue->storage_capacity) {
            target_capacity = SIZE_MAX;
        }
        if (queue->capacity != 0 && target_capacity > queue->capacity) {
            target_capacity = queue->capacity;
        }
        if (target_capacity == 0 || !pq_reserve(queue, target_capacity)) {
            return false;
        }
    }

    queue->items[queue->size].data = data;
    queue->items[queue->size].priority = priority;
    queue->order[queue->size] = queue->sequence++;
    queue->size++;
    pq_heapify_up(queue, queue->size - 1);
    return true;
}

bool pq_pop(priority_queue_t *queue, void **data, int *priority)
{
    if (queue == NULL || queue->size == 0) {
        return false;
    }

    if (data != NULL) {
        *data = queue->items[0].data;
    }
    if (priority != NULL) {
        *priority = queue->items[0].priority;
    }

    queue->size--;
    if (queue->size > 0) {
        queue->items[0] = queue->items[queue->size];
        queue->order[0] = queue->order[queue->size];
        pq_heapify_down(queue, 0);
    }
    queue->items[queue->size].data = NULL;
    queue->items[queue->size].priority = 0;
    queue->order[queue->size] = 0;
    return true;
}

bool pq_peek(priority_queue_t *queue, void **data, int *priority)
{
    if (queue == NULL || queue->size == 0) {
        return false;
    }
    if (data != NULL) {
        *data = queue->items[0].data;
    }
    if (priority != NULL) {
        *priority = queue->items[0].priority;
    }
    return true;
}

size_t pq_size(const priority_queue_t *queue)
{
    return queue == NULL ? 0 : queue->size;
}

bool pq_is_empty(const priority_queue_t *queue)
{
    return queue == NULL || queue->size == 0;
}

bool pq_is_full(const priority_queue_t *queue)
{
    if (queue == NULL) {
        return false;
    }
    if (queue->capacity == 0) {
        return false;
    }
    return queue->size >= queue->capacity;
}
