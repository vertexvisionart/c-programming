#include "list.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct list_node {
    int value;
    struct list_node *prev;
    struct list_node *next;
} list_node_t;

struct list {
    list_node_t *head;
    list_node_t *tail;
    size_t size;
};

static list_node_t *list_node_create(int value)
{
    list_node_t *node = (list_node_t *) malloc(sizeof(*node));
    if (node == NULL) {
        return NULL;
    }
    node->value = value;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

list_t *list_create(void)
{
    return (list_t *) calloc(1, sizeof(list_t));
}

list_t *list_new(void)
{
    return list_create();
}

void list_clear(list_t *list)
{
    list_node_t *node;
    list_node_t *next;

    if (list == NULL) {
        return;
    }

    node = list->head;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void list_destroy(list_t *list)
{
    if (list == NULL) {
        return;
    }
    list_clear(list);
    free(list);
}

void list_delete(list_t *list)
{
    list_destroy(list);
}

size_t list_size(const list_t *list)
{
    return list == NULL ? 0 : list->size;
}

bool list_empty(const list_t *list)
{
    return list == NULL || list->size == 0;
}

bool list_is_empty(const list_t *list)
{
    return list_empty(list);
}

bool list_push_back(list_t *list, int value)
{
    list_node_t *node;

    if (list == NULL) {
        return false;
    }

    node = list_node_create(value);
    if (node == NULL) {
        return false;
    }

    node->prev = list->tail;
    if (list->tail != NULL) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
    list->size++;
    return true;
}

bool list_push_front(list_t *list, int value)
{
    list_node_t *node;

    if (list == NULL) {
        return false;
    }

    node = list_node_create(value);
    if (node == NULL) {
        return false;
    }

    node->next = list->head;
    if (list->head != NULL) {
        list->head->prev = node;
    } else {
        list->tail = node;
    }
    list->head = node;
    list->size++;
    return true;
}

bool list_pop_back(list_t *list, int *value)
{
    list_node_t *node;

    if (list == NULL || value == NULL || list->tail == NULL) {
        return false;
    }

    node = list->tail;
    *value = node->value;

    list->tail = node->prev;
    if (list->tail != NULL) {
        list->tail->next = NULL;
    } else {
        list->head = NULL;
    }

    free(node);
    list->size--;
    return true;
}

bool list_pop_front(list_t *list, int *value)
{
    list_node_t *node;

    if (list == NULL || value == NULL || list->head == NULL) {
        return false;
    }

    node = list->head;
    *value = node->value;

    list->head = node->next;
    if (list->head != NULL) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL;
    }

    free(node);
    list->size--;
    return true;
}

bool list_front(const list_t *list, int *value)
{
    if (list == NULL || value == NULL || list->head == NULL) {
        return false;
    }
    *value = list->head->value;
    return true;
}

bool list_back(const list_t *list, int *value)
{
    if (list == NULL || value == NULL || list->tail == NULL) {
        return false;
    }
    *value = list->tail->value;
    return true;
}

int list_at(const list_t *list, size_t index)
{
    const list_node_t *node;
    size_t i;

    if (list == NULL || index >= list->size) {
        return 0;
    }

    if (index < list->size / 2) {
        node = list->head;
        for (i = 0; i < index; ++i) {
            node = node->next;
        }
    } else {
        node = list->tail;
        for (i = list->size - 1; i > index; --i) {
            node = node->prev;
        }
    }

    return node->value;
}

int list_get(const list_t *list, size_t index)
{
    return list_at(list, index);
}

void list_print(const list_t *list)
{
    const list_node_t *node;
    int first = 1;

    printf("[");
    if (list != NULL) {
        for (node = list->head; node != NULL; node = node->next) {
            if (!first) {
                printf(", ");
            }
            printf("%d", node->value);
            first = 0;
        }
    }
    printf("]\n");
}
