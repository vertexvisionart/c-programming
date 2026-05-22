#include "list.h"
#include <stdlib.h>
#include <stdio.h>

// Внутренняя структура узла (скрыта от пользователя)
typedef struct Node {
    int value;
    struct Node* prev;
    struct Node* next;
} Node;

// Внутренняя структура списка (скрыта от пользователя)
struct List {
    Node* head;
    Node* tail;
    int size;
};

// Создание пустого списка
List* list_create(void) {
    List* list = (List*)malloc(sizeof(List));
    if (list) {
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;
    }
    return list;
}

// Удаление списка и очистка памяти
void list_destroy(List* list) {
    if (!list) return;
    Node* current = list->head;
    while (current) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

// Добавление в конец
void list_push_back(List* list, int value) {
    if (!list) return;
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return;
    
    node->value = value;
    node->next = NULL;
    node->prev = list->tail;
    
    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node; // Если список был пуст
    }
    list->tail = node;
    list->size++;
}

// Добавление в начало
void list_push_front(List* list, int value) {
    if (!list) return;
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return;
    
    node->value = value;
    node->prev = NULL;
    node->next = list->head;
    
    if (list->head) {
        list->head->prev = node;
    } else {
        list->tail = node; // Если список был пуст
    }
    list->head = node;
    list->size++;
}

// Извлечение с конца
int list_pop_back(List* list) {
    if (!list || !list->tail) return 0;
    
    Node* node = list->tail;
    int value = node->value;
    
    list->tail = node->prev;
    if (list->tail) {
        list->tail->next = NULL;
    } else {
        list->head = NULL; // Если удалили последний элемент
    }
    
    free(node);
    list->size--;
    return value;
}

// Извлечение с начала
int list_pop_front(List* list) {
    if (!list || !list->head) return 0;
    
    Node* node = list->head;
    int value = node->value;
    
    list->head = node->next;
    if (list->head) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL; // Если удалили последний элемент
    }
    
    free(node);
    list->size--;
    return value;
}

// Просмотр первого элемента
int list_front(const List* list) {
    if (!list || !list->head) return 0;
    return list->head->value;
}

// Просмотр последнего элемента
int list_back(const List* list) {
    if (!list || !list->tail) return 0;
    return list->tail->value;
}

// Получение размера
int list_size(const List* list) {
    if (!list) return 0;
    return list->size;
}

// Вывод списка в требуемом формате 
void list_print(const List* list) {
    if (!list) return;
    
    printf("[");
    Node* current = list->head;
    while (current) {
        printf("%d", current->value);
        if (current->next) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
    
    // Сбрасываем буфер, чтобы Python сразу увидел вывод
    fflush(stdout); 
}