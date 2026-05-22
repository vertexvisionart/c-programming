#ifndef LIST_H
#define LIST_H

// Непрозрачный указатель: сама структура описана только в .c файле
typedef struct List List;

// Конструктор и деструктор
List* list_create(void);
void list_destroy(List* list);

// Добавление элементов
void list_push_back(List* list, int value);
void list_push_front(List* list, int value);

// Извлечение элементов (удаляют элемент из списка и возвращают его значение)
int list_pop_back(List* list);
int list_pop_front(List* list);

// Просмотр элементов (без удаления)
int list_front(const List* list);
int list_back(const List* list);

// Получение размера
int list_size(const List* list);

// Вывод списка в формате [a, b, c]
void list_print(const List* list);

#endif // LIST_H