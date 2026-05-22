#include <stdio.h>

struct ListNode {
  int val;
  struct ListNode *next;
};

struct ListNode *mergeTwoLists(struct ListNode *list1, struct ListNode *list2) {
  // 1. Создаем "фальшивую голову" (Dummy Node).
  // Она лежит просто на стеке (не malloc), удалять её не надо.
  // Это просто точка старта, к которой мы будем всё прицеплять.
  struct ListNode dummy;
  dummy.val = -1; // Значение не важно
  dummy.next = NULL;

  // 2. Указатель-хвост (Tail).
  // Сначала он указывает на пустышку. Он будет бежать и "шить".
  struct ListNode *tail = &dummy;

  // 3. Пока оба списка живы — выбираем меньшего
  while (list1 != NULL && list2 != NULL) {
    if (list1->val <= list2->val) {
      tail->next = list1;  // Цепляем узел из первого списка
      list1 = list1->next; // Сдвигаем указатель в первом списке
    } else {
      tail->next = list2;  // Цепляем узел из второго списка
      list2 = list2->next; // Сдвигаем указатель во втором списке
    }
    tail = tail->next; // Хвост всегда должен быть на конце нашей новой цепочки
  }

  // 4. Кто-то кончился. Прицепляем остаток другого списка целиком.
  // Нам не нужен цикл! Мы просто говорим: "дальше идет вот этот кусок".
  if (list1 != NULL) {
    tail->next = list1;
  } else {
    tail->next = list2;
  }

  // 5. Возвращаем то, что идет после фальшивой головы
  return dummy.next;
}
