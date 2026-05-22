#include <stdio.h>
#include <stdlib.h>

// Input: list1 = [1,2,4], list2 = [1,3,4]
// Output: [1,1,2,3,4,4]

struct ListNode {
  int val;
  struct ListNode *next;
};

struct ListNode *mergeTwoLists(struct ListNode *list1, struct ListNode *list2) {

  struct ListNode *current = list1;

  int n = 0;
  int m = 0;

  while (current != NULL) {
    current = current->next;
    n++;
  }

  struct ListNode *current2 = list2;

  while (current2 != NULL) {
    current2 = current2->next;
    m++;
  }

  // printf("list1:%d list2:%d", n, m);
  struct ListNode *result =
      (struct ListNode *)malloc((n + m) * sizeof(struct ListNode));
  struct ListNode *forres = result;

  for (int i = 0; i < n + m; i++) {
    if (i < n + m - 1) {
      result[i].next = &result[i + 1];
    } else {
      result[i].next = NULL;
    }
  }

  int i = 0;
  int j = 0;

  int val_left = list1->val;
  int val_right = list2->val;

  struct ListNode *ptrleft = list1;
  struct ListNode *ptrright = list2;

  while (ptrright != NULL && ptrleft != NULL) {
    if (ptrleft->val <= ptrright->val) {
      result->val = ptrleft->val;
      ptrleft = ptrleft->next;
    } else {
      result->val = ptrright->val;
      ptrright = ptrright->next;
    }
    result = result->next;
  }

  while (ptrright != NULL) {
    result->val = ptrright->val;
    result = result->next;
    ptrright = ptrright->next;
  }

  while (ptrleft != NULL) {
    result->val = ptrleft->val;
    result = result->next;
    ptrleft = ptrleft->next;
  }

  return forres;
}

int main(void) {

  int n = 3;
  int m = 4;

  struct ListNode *list1_n =
      (struct ListNode *)malloc(n * sizeof(struct ListNode));

  struct ListNode *list2_n =
      (struct ListNode *)malloc(m * sizeof(struct ListNode));

  for (int i = 0; i < n; i++) {
    scanf("%d", &list1_n[i].val);
    if (i < n - 1) {
      list1_n[i].next = &list1_n[i + 1];
    } else {
      list1_n[i].next = NULL;
    }
  }
  printf("NEW MASSIV:\n");
  for (int i = 0; i < m; i++) {
    scanf("%d", &list2_n[i].val);
    if (i < m - 1) {
      list2_n[i].next = &list2_n[i + 1];
    } else {
      list2_n[i].next = NULL;
    }
  }

  struct ListNode *list1 = &list1_n[0];
  struct ListNode *list2 = &list2_n[0];
  struct ListNode *res = mergeTwoLists(list1, list2);

  while (res != NULL) {
    printf("\n%d\n", res->val);
    res = res->next;
  }
  return 0;
}
