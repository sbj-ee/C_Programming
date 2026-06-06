#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
   Both structures share the same node shape.
   The difference is purely in which end is used for insertion
   and which for removal.

   STACK — Last In, First Out (LIFO)
     push → top     pop ← top       (one end only)

   QUEUE — First In, First Out (FIFO)
     enqueue → tail     dequeue ← head   (two ends)
   ================================================================ */

typedef struct Node {
    int          value;
    struct Node *next;
} Node;

/* ================================================================
   STACK
   ================================================================ */
typedef struct {
    Node *top;
    int   size;
} Stack;

Stack *stack_create(void);
void   stack_free(Stack *s);
void   stack_push(Stack *s, int value);
int    stack_pop(Stack *s);
int    stack_peek(const Stack *s);
int    stack_is_empty(const Stack *s);
void   stack_print(const Stack *s);

/* ================================================================
   QUEUE
   ================================================================ */
typedef struct {
    Node *head;   /* dequeue end */
    Node *tail;   /* enqueue end — O(1) enqueue without walking the list */
    int   size;
} Queue;

Queue *queue_create(void);
void   queue_free(Queue *q);
void   queue_enqueue(Queue *q, int value);
int    queue_dequeue(Queue *q);
int    queue_peek(const Queue *q);
int    queue_is_empty(const Queue *q);
void   queue_print(const Queue *q);

/* ================================================================
   APPLICATIONS
   ================================================================ */
/* Check whether a string of brackets is balanced — classic stack use */
int  brackets_balanced(const char *s);

/* Print integers 1..n in the order a BFS would visit a complete binary tree */
void bfs_order(int n);

/* ------------------------------------------------------------------ */

int main(void) {

    /* ---- STACK ---- */
    printf("=== Stack (LIFO) ===\n");

    Stack *s = stack_create();
    printf("empty? %d\n", stack_is_empty(s));

    for (int i = 1; i <= 5; i++) {
        stack_push(s, i * 10);
        printf("push %d  → ", i * 10);
        stack_print(s);
    }

    printf("peek   = %d\n", stack_peek(s));

    while (!stack_is_empty(s)) {
        printf("pop    = %d  → ", stack_pop(s));
        stack_print(s);
    }

    printf("empty? %d\n", stack_is_empty(s));
    stack_free(s);

    /* ---- QUEUE ---- */
    printf("\n=== Queue (FIFO) ===\n");

    Queue *q = queue_create();
    printf("empty? %d\n", queue_is_empty(q));

    for (int i = 1; i <= 5; i++) {
        queue_enqueue(q, i * 10);
        printf("enqueue %d  → ", i * 10);
        queue_print(q);
    }

    printf("peek   = %d\n", queue_peek(q));

    while (!queue_is_empty(q)) {
        printf("dequeue = %d  → ", queue_dequeue(q));
        queue_print(q);
    }

    printf("empty? %d\n", queue_is_empty(q));
    queue_free(q);

    /* ---- APPLICATION: bracket balancing with a stack ---- */
    printf("\n=== Application: bracket balancing ===\n");
    const char *exprs[] = {
        "(a + b) * (c - d)",
        "{[()]}",
        "([)]",
        "(((",
        "",
        "no brackets at all",
    };
    for (int i = 0; i < 6; i++) {
        printf("  %-26s  %s\n", exprs[i],
               brackets_balanced(exprs[i]) ? "balanced" : "UNBALANCED");
    }

    /* ---- APPLICATION: BFS level-order traversal with a queue ---- */
    printf("\n=== Application: BFS level-order (complete binary tree, n=7) ===\n");
    bfs_order(7);

    return 0;
}

/* ================================================================
   STACK IMPLEMENTATION
   ================================================================ */

static Node *node_new(int value) {
    Node *n = malloc(sizeof(Node));
    if (!n) { fprintf(stderr, "malloc failed\n"); exit(1); }
    n->value = value;
    n->next  = NULL;
    return n;
}

Stack *stack_create(void) {
    Stack *s = calloc(1, sizeof(Stack));
    if (!s) { fprintf(stderr, "calloc failed\n"); exit(1); }
    return s;
}

void stack_free(Stack *s) {
    while (!stack_is_empty(s)) stack_pop(s);
    free(s);
}

/* Push onto the top in O(1) */
void stack_push(Stack *s, int value) {
    Node *n  = node_new(value);
    n->next  = s->top;
    s->top   = n;
    s->size++;
}

/* Pop from the top in O(1) */
int stack_pop(Stack *s) {
    if (stack_is_empty(s)) { fprintf(stderr, "pop on empty stack\n"); exit(1); }
    Node *old = s->top;
    int   val = old->value;
    s->top    = old->next;
    free(old);
    s->size--;
    return val;
}

int stack_peek(const Stack *s) {
    if (stack_is_empty(s)) { fprintf(stderr, "peek on empty stack\n"); exit(1); }
    return s->top->value;
}

int stack_is_empty(const Stack *s) { return s->size == 0; }

void stack_print(const Stack *s) {
    printf("top[");
    for (Node *cur = s->top; cur; cur = cur->next) {
        printf("%d", cur->value);
        if (cur->next) printf(", ");
    }
    printf("]  size=%d\n", s->size);
}

/* ================================================================
   QUEUE IMPLEMENTATION
   ================================================================ */

Queue *queue_create(void) {
    Queue *q = calloc(1, sizeof(Queue));
    if (!q) { fprintf(stderr, "calloc failed\n"); exit(1); }
    return q;
}

void queue_free(Queue *q) {
    while (!queue_is_empty(q)) queue_dequeue(q);
    free(q);
}

/* Enqueue at the tail in O(1) — the tail pointer avoids walking the list */
void queue_enqueue(Queue *q, int value) {
    Node *n = node_new(value);
    if (q->tail) q->tail->next = n;
    else         q->head       = n;   /* first node: head and tail both point to it */
    q->tail = n;
    q->size++;
}

/* Dequeue from the head in O(1) */
int queue_dequeue(Queue *q) {
    if (queue_is_empty(q)) { fprintf(stderr, "dequeue on empty queue\n"); exit(1); }
    Node *old = q->head;
    int   val = old->value;
    q->head   = old->next;
    if (q->head == NULL) q->tail = NULL;   /* queue is now empty */
    free(old);
    q->size--;
    return val;
}

int queue_peek(const Queue *q) {
    if (queue_is_empty(q)) { fprintf(stderr, "peek on empty queue\n"); exit(1); }
    return q->head->value;
}

int queue_is_empty(const Queue *q) { return q->size == 0; }

void queue_print(const Queue *q) {
    printf("head[");
    for (Node *cur = q->head; cur; cur = cur->next) {
        printf("%d", cur->value);
        if (cur->next) printf(", ");
    }
    printf("]tail  size=%d\n", q->size);
}

/* ================================================================
   APPLICATIONS
   ================================================================ */

int brackets_balanced(const char *s) {
    /* Use a stack of chars — push openers, match closers */
    int   top  = -1;
    char  stk[256];

    for (int i = 0; s[i]; i++) {
        char c = s[i];
        if (c == '(' || c == '[' || c == '{') {
            stk[++top] = c;
        } else if (c == ')' || c == ']' || c == '}') {
            if (top < 0) return 0;   /* closer with no matching opener */
            char open = stk[top--];
            if ((c == ')' && open != '(') ||
                (c == ']' && open != '[') ||
                (c == '}' && open != '{')) return 0;
        }
    }
    return top == -1;   /* stack must be empty at end */
}

/* Simulate BFS on a complete binary tree stored as a 1-indexed array.
   Node i has children 2i (left) and 2i+1 (right).
   A queue naturally produces level-order (breadth-first) output. */
void bfs_order(int n) {
    Queue *q = queue_create();
    queue_enqueue(q, 1);   /* start at root (index 1) */

    int level = 0, in_level = 1, printed = 0;
    printf("level %d: ", level);

    while (!queue_is_empty(q)) {
        int node = queue_dequeue(q);
        printf("%d ", node);
        printed++;

        int left  = 2 * node;
        int right = 2 * node + 1;
        if (left  <= n) queue_enqueue(q, left);
        if (right <= n) queue_enqueue(q, right);

        if (printed == in_level && !queue_is_empty(q)) {
            in_level = q->size;
            printf("\nlevel %d: ", ++level);
            printed = 0;
        }
    }
    printf("\n");
    queue_free(q);
}
