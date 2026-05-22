/* Balanced bracket validator — uses a stack to match (), [], {}. */
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

static void push(Node **head, int val) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (n == NULL) return;
    n->data = val;
    n->next = *head;
    *head = n;
}

static void freeall(Node *head) {
    while (head != NULL) {
        Node *t = head;
        head = head->next;
        free(t);
    }
}

static int pop(Node **head) {
    if (*head == NULL) return -1;
    Node *t = *head;
    int val = (*head)->data;
    *head = (*head)->next;
    free(t);
    return val;
}

int is_balanced(const char *str) {
    Node *head = NULL;
    int ok = 1;
    while (*str != '\0') {
        if (*str == '(' || *str == '{' || *str == '[') {
            push(&head, *str);
        } else if (*str == ')' || *str == '}' || *str == ']') {
            int val = pop(&head);
            if (val == -1) { ok = 0; break; }
            if ((*str == ')' && val != '(') ||
                (*str == '}' && val != '{') ||
                (*str == ']' && val != '[')) { ok = 0; break; }
        }
        str++;
    }
    if (head != NULL) ok = 0;
    freeall(head);
    return ok;
}

int main(void) {
    const char *cases[] = { "([{}])", "([)]", "((()))", "(", "{[a+b]}" };
    for (int i = 0; i < (int)(sizeof(cases)/sizeof(*cases)); i++) {
        printf("%-12s -> %s\n", cases[i], is_balanced(cases[i]) ? "balanced" : "not balanced");
    }
    return 0;
}
