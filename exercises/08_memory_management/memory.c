#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Dynamic array that grows on demand */
typedef struct {
    int   *data;
    int    size;     /* elements currently stored */
    int    capacity; /* elements allocated */
} IntVec;

IntVec *vec_create(int initial_capacity);
void    vec_push(IntVec *v, int value);
void    vec_free(IntVec *v);
char   *dup_string(const char *src);

/* ------------------------------------------------------------------ */

int main(void) {

    /* --- malloc: allocate uninitialized memory ---
       Returns a void* to the block, or NULL on failure.
       Always check the return value before using the pointer. */
    printf("=== malloc ===\n");

    int n = 5;
    int *arr = malloc(n * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    /* Contents are uninitialized — write before reading */
    for (int i = 0; i < n; i++) arr[i] = (i + 1) * 10;
    for (int i = 0; i < n; i++) printf("arr[%d] = %d\n", i, arr[i]);

    free(arr);
    arr = NULL;   /* NULL-ing after free prevents accidental use of a dangling pointer */

    /* --- calloc: allocate and zero-initialize ---
       calloc(count, size) — takes element count and element size separately */
    printf("\n=== calloc ===\n");

    int *zeroed = calloc(5, sizeof(int));
    if (zeroed == NULL) { fprintf(stderr, "calloc failed\n"); return 1; }

    printf("fresh calloc (should all be 0): ");
    for (int i = 0; i < 5; i++) printf("%d ", zeroed[i]);
    printf("\n");

    free(zeroed);
    zeroed = NULL;

    /* --- realloc: resize an existing allocation ---
       If the new size fits in place, the same pointer is returned.
       Otherwise a new block is allocated, contents are copied, and the
       old block is freed. Always assign the result to a temporary first —
       if realloc fails it returns NULL and the original pointer is still valid. */
    printf("\n=== realloc ===\n");

    int  cap  = 4;
    int *buf  = malloc(cap * sizeof(int));
    if (buf == NULL) { fprintf(stderr, "malloc failed\n"); return 1; }

    for (int i = 0; i < cap; i++) buf[i] = i;
    printf("before realloc (cap=%d): ", cap);
    for (int i = 0; i < cap; i++) printf("%d ", buf[i]);
    printf("\n");

    cap *= 2;
    int *tmp = realloc(buf, cap * sizeof(int));
    if (tmp == NULL) {
        fprintf(stderr, "realloc failed\n");
        free(buf);   /* original allocation is still valid on failure */
        return 1;
    }
    buf = tmp;

    for (int i = cap / 2; i < cap; i++) buf[i] = i;
    printf("after  realloc (cap=%d): ", cap);
    for (int i = 0; i < cap; i++) printf("%d ", buf[i]);
    printf("\n");

    free(buf);
    buf = NULL;

    /* --- Allocating structs on the heap --- */
    printf("\n=== Heap-allocated struct ===\n");

    typedef struct { char name[32]; int score; } Player;

    Player *p = malloc(sizeof(Player));
    if (p == NULL) { fprintf(stderr, "malloc failed\n"); return 1; }

    strcpy(p->name, "Alice");
    p->score = 9500;
    printf("player: %s  score: %d\n", p->name, p->score);

    free(p);
    p = NULL;

    /* --- Heap-allocated array of structs --- */
    printf("\n=== Heap-allocated array of structs ===\n");

    int num_players = 3;
    Player *team = calloc(num_players, sizeof(Player));
    if (team == NULL) { fprintf(stderr, "calloc failed\n"); return 1; }

    const char *names[] = {"Bob", "Carol", "Dave"};
    for (int i = 0; i < num_players; i++) {
        strcpy(team[i].name, names[i]);
        team[i].score = (i + 1) * 1000;
    }
    for (int i = 0; i < num_players; i++) {
        printf("  %s: %d\n", team[i].name, team[i].score);
    }

    free(team);
    team = NULL;

    /* --- Duplicating a string onto the heap --- */
    printf("\n=== Heap string (strdup pattern) ===\n");

    char *copy = dup_string("Hello, heap!");
    if (copy == NULL) { fprintf(stderr, "dup_string failed\n"); return 1; }
    printf("copy = \"%s\"\n", copy);
    free(copy);
    copy = NULL;

    /* --- Dynamic array that grows automatically --- */
    printf("\n=== Dynamic array (IntVec) ===\n");

    IntVec *v = vec_create(2);
    if (v == NULL) { fprintf(stderr, "vec_create failed\n"); return 1; }

    for (int i = 1; i <= 8; i++) {
        vec_push(v, i * 5);
        printf("pushed %2d  size=%d  cap=%d\n", i * 5, v->size, v->capacity);
    }

    printf("contents: ");
    for (int i = 0; i < v->size; i++) printf("%d ", v->data[i]);
    printf("\n");

    vec_free(v);

    return 0;
}

/* ------------------------------------------------------------------ */

/* Returns a heap-allocated copy of src — caller must free() it */
char *dup_string(const char *src) {
    size_t len = strlen(src) + 1;   /* +1 for '\0' */
    char  *dst = malloc(len);
    if (dst == NULL) return NULL;
    memcpy(dst, src, len);
    return dst;
}

IntVec *vec_create(int initial_capacity) {
    IntVec *v = malloc(sizeof(IntVec));
    if (v == NULL) return NULL;

    v->data = malloc(initial_capacity * sizeof(int));
    if (v->data == NULL) { free(v); return NULL; }

    v->size     = 0;
    v->capacity = initial_capacity;
    return v;
}

/* Doubles capacity via realloc whenever the array is full */
void vec_push(IntVec *v, int value) {
    if (v->size == v->capacity) {
        int new_cap = v->capacity * 2;
        int *tmp    = realloc(v->data, new_cap * sizeof(int));
        if (tmp == NULL) { fprintf(stderr, "vec_push: realloc failed\n"); return; }
        v->data     = tmp;
        v->capacity = new_cap;
    }
    v->data[v->size++] = value;
}

void vec_free(IntVec *v) {
    free(v->data);
    free(v);
}
