#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A singly linked list of integers.
   Each node owns its memory; the list owns its nodes. */
typedef struct Node {
    int          value;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    int   size;
} List;

/* Lifecycle */
List *list_create(void);
void  list_free(List *list);

/* Insertion */
void  list_push_front(List *list, int value);   /* O(1) */
void  list_push_back(List *list,  int value);   /* O(n) */
void  list_insert_at(List *list, int index, int value);

/* Deletion */
int   list_pop_front(List *list);               /* O(1) */
void  list_delete_value(List *list, int value); /* removes first match */
void  list_delete_at(List *list, int index);

/* Query */
int   list_contains(const List *list, int value);
int   list_get(const List *list, int index);
void  list_print(const List *list);

/* Utilities */
void  list_reverse(List *list);

/* ------------------------------------------------------------------ */

int main(void) {

    /* --- Build a list --- */
    printf("=== push_back / push_front ===\n");

    List *list = list_create();

    list_push_back(list, 10);
    list_push_back(list, 20);
    list_push_back(list, 30);
    list_print(list);                    /* [10 -> 20 -> 30] */

    list_push_front(list, 5);
    list_print(list);                    /* [5 -> 10 -> 20 -> 30] */

    /* --- Insert at arbitrary index --- */
    printf("\n=== insert_at ===\n");
    list_insert_at(list, 2, 15);         /* insert 15 between 10 and 20 */
    list_print(list);                    /* [5 -> 10 -> 15 -> 20 -> 30] */
    list_insert_at(list, 0, 1);          /* insert at head */
    list_print(list);                    /* [1 -> 5 -> 10 -> 15 -> 20 -> 30] */

    /* --- Query --- */
    printf("\n=== get / contains ===\n");
    printf("get(0)         = %d\n",  list_get(list, 0));
    printf("get(3)         = %d\n",  list_get(list, 3));
    printf("contains(15)   = %d\n",  list_contains(list, 15));
    printf("contains(99)   = %d\n",  list_contains(list, 99));
    printf("size           = %d\n",  list->size);

    /* --- Delete --- */
    printf("\n=== pop_front ===\n");
    int val = list_pop_front(list);
    printf("popped %d\n", val);
    list_print(list);                    /* [5 -> 10 -> 15 -> 20 -> 30] */

    printf("\n=== delete_value ===\n");
    list_delete_value(list, 15);         /* remove from middle */
    list_print(list);                    /* [5 -> 10 -> 20 -> 30] */
    list_delete_value(list, 5);          /* remove head */
    list_print(list);                    /* [10 -> 20 -> 30] */
    list_delete_value(list, 30);         /* remove tail */
    list_print(list);                    /* [10 -> 20] */
    list_delete_value(list, 99);         /* not found — no crash */
    list_print(list);

    printf("\n=== delete_at ===\n");
    list_push_back(list, 30);
    list_push_back(list, 40);
    list_print(list);                    /* [10 -> 20 -> 30 -> 40] */
    list_delete_at(list, 1);             /* remove index 1 */
    list_print(list);                    /* [10 -> 30 -> 40] */

    /* --- Reverse --- */
    printf("\n=== reverse ===\n");
    list_push_back(list, 50);
    list_push_back(list, 60);
    list_print(list);                    /* [10 -> 30 -> 40 -> 50 -> 60] */
    list_reverse(list);
    list_print(list);                    /* [60 -> 50 -> 40 -> 30 -> 10] */

    list_free(list);
    return 0;
}

/* ------------------------------------------------------------------ */

static Node *node_create(int value) {
    Node *n = malloc(sizeof(Node));
    if (n == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
    n->value = value;
    n->next  = NULL;
    return n;
}

List *list_create(void) {
    List *l = malloc(sizeof(List));
    if (l == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
    l->head = NULL;
    l->size = 0;
    return l;
}

void list_free(List *list) {
    Node *cur = list->head;
    while (cur != NULL) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    free(list);
}

/* --- Insertion --- */

void list_push_front(List *list, int value) {
    Node *n  = node_create(value);
    n->next  = list->head;
    list->head = n;
    list->size++;
}

void list_push_back(List *list, int value) {
    Node *n = node_create(value);
    if (list->head == NULL) {
        list->head = n;
    } else {
        Node *cur = list->head;
        while (cur->next != NULL) cur = cur->next;
        cur->next = n;
    }
    list->size++;
}

void list_insert_at(List *list, int index, int value) {
    if (index <= 0 || list->head == NULL) {
        list_push_front(list, value);
        return;
    }
    Node *cur = list->head;
    for (int i = 0; i < index - 1 && cur->next != NULL; i++) {
        cur = cur->next;
    }
    Node *n   = node_create(value);
    n->next   = cur->next;
    cur->next = n;
    list->size++;
}

/* --- Deletion --- */

int list_pop_front(List *list) {
    if (list->head == NULL) { fprintf(stderr, "pop on empty list\n"); exit(1); }
    Node *old  = list->head;
    int   val  = old->value;
    list->head = old->next;
    free(old);
    list->size--;
    return val;
}

void list_delete_value(List *list, int value) {
    if (list->head == NULL) return;

    /* Special case: target is the head */
    if (list->head->value == value) {
        list_pop_front(list);
        return;
    }

    /* Walk until the node before the target */
    Node *prev = list->head;
    while (prev->next != NULL && prev->next->value != value) {
        prev = prev->next;
    }
    if (prev->next == NULL) return;   /* not found */

    Node *target = prev->next;
    prev->next   = target->next;
    free(target);
    list->size--;
}

void list_delete_at(List *list, int index) {
    if (list->head == NULL || index < 0) return;
    if (index == 0) { list_pop_front(list); return; }

    Node *prev = list->head;
    for (int i = 0; i < index - 1 && prev->next != NULL; i++) {
        prev = prev->next;
    }
    if (prev->next == NULL) return;   /* index out of range */

    Node *target = prev->next;
    prev->next   = target->next;
    free(target);
    list->size--;
}

/* --- Query --- */

int list_contains(const List *list, int value) {
    Node *cur = list->head;
    while (cur != NULL) {
        if (cur->value == value) return 1;
        cur = cur->next;
    }
    return 0;
}

int list_get(const List *list, int index) {
    Node *cur = list->head;
    for (int i = 0; i < index && cur != NULL; i++) cur = cur->next;
    if (cur == NULL) { fprintf(stderr, "index out of range\n"); exit(1); }
    return cur->value;
}

void list_print(const List *list) {
    printf("size=%d  [", list->size);
    Node *cur = list->head;
    while (cur != NULL) {
        printf("%d", cur->value);
        if (cur->next != NULL) printf(" -> ");
        cur = cur->next;
    }
    printf("]\n");
}

/* --- Reverse in place using three pointers --- */
void list_reverse(List *list) {
    Node *prev = NULL;
    Node *cur  = list->head;
    while (cur != NULL) {
        Node *next = cur->next;  /* save next before overwriting */
        cur->next  = prev;       /* reverse the link */
        prev       = cur;        /* advance prev */
        cur        = next;       /* advance cur */
    }
    list->head = prev;
}
