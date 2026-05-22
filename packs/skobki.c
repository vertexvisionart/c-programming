#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
  int data;
  struct Node *next;
} Node;

void push(Node **head, int val) {
  Node *new_node = (Node *)malloc(sizeof(Node));
  if (new_node == NULL)
    return;
  new_node->data = val;
  new_node->next = *head;
  *head = new_node;
}

void freeall(Node *head) {
  while (head != NULL) {
    Node *temp = head;
    head = head->next;
    free(temp);
  }
}

int pop(Node **head) {
  if (*head == NULL) {
    printf("Empty stack\n");
    return -1;
  } else {
    Node *temp = *head;
    int val = (*head)->data;
    *head = (*head)->next;
    free(temp);
    return val;
  }
}

int is_normal(const char *str, Node **head) {
  if (*str == ')' || *str == '}' || *str == ']')
    return 0;
  while (*str != '\0') {
    if (*str == '(' || *str == '{' || *str == '[') {
      push(head, *str);
    }
    if (*str == ')' || *str == '}' || *str == ']') {
      char val = pop(head);
      if (val == -1) {
        return 0;
      }
      if (*str == ')' && val != '(')
        return 0;
      if (*str == '}' && val != '{')
        return 0;
      if (*str == ']' && val != '[')
        return 0;
    }
    str++;
  }
  if (*head != NULL) {
    printf("NETT\n");
    return 0;
  }
  return 1;
}

int main(void) {
  Node *head = NULL;
  char str[] = "([{}])";
  if (is_normal(str, &head)) {
    printf("GOOD!\n");
  } else {
    printf("BAAD!\n");
  }
  freeall(head);
  return 0;
}
