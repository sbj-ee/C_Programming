/*
 * Exercise 21: Generics via void *
 *
 * C has no templates.  Generic code is achieved through three ideas:
 *
 *   1. void *  — a pointer to "untyped" memory; any T* converts to it
 *              and back without a cast
 *   2. size_t element_size — carry the element size at runtime so the
 *              function can move bytes without knowing the type
 *   3. int (*cmp)(const void *, const void *) — a caller-supplied
 *              comparator; the exact mechanism used by qsort / bsearch
 *
 * Together these three let one function body work on ints, doubles,
 * structs, or anything else — at the cost of a little indirection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ──────────────────────────────────────────────────────────
 * Section 1: void * basics
 *
 * void * is assignment-compatible with every pointer type.
 * You cannot dereference it directly — you must cast first.
 * memcpy / memmove are the standard tools for moving bytes
 * through a void * when the type is unknown.
 * ────────────────────────────────────────────────────────── */

/* Swap any two objects of the same size */
static void generic_swap(void *a, void *b, size_t size) {
    /* Use a stack buffer for small sizes, malloc for large */
    uint8_t tmp[256];
    void *buf = (size <= sizeof tmp) ? tmp : malloc(size);
    if (!buf) return;
    memcpy(buf, a,   size);
    memcpy(a,   b,   size);
    memcpy(b,   buf, size);
    if (buf != tmp) free(buf);
}

static void section_void_ptr(void) {
    printf("=== 1. void * basics ===\n");

    /* Any T* converts to void* without a cast */
    int   x = 10, y = 20;
    double p = 1.5, q = 9.9;
    char  s[8] = "hello", t[8] = "world";

    generic_swap(&x, &y, sizeof x);
    printf("int swap:    x=%d y=%d\n", x, y);

    generic_swap(&p, &q, sizeof p);
    printf("double swap: p=%.1f q=%.1f\n", p, q);

    generic_swap(s, t, sizeof s);
    printf("char[] swap: s=%s t=%s\n", s, t);

    /* Pointer arithmetic on void* is illegal in strict C11.
     * Cast to char* (1-byte pointer) to step through raw bytes. */
    int arr[4] = {10, 20, 30, 40};
    void *base = arr;
    for (int i = 0; i < 4; i++) {
        /* cast void* -> char* to do byte arithmetic, then to int* */
        int *elem = (int *)((char *)base + i * sizeof(int));
        printf("  arr[%d] = %d\n", i, *elem);
    }

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: Generic comparators
 *
 * qsort / bsearch require:  int cmp(const void *a, const void *b)
 * Return < 0 if a < b, 0 if equal, > 0 if a > b.
 *
 * The pattern: cast the void* arguments to the concrete type,
 * dereference, compare.
 * ────────────────────────────────────────────────────────── */

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    /* Avoid overflow: don't use ia - ib */
    return (ia > ib) - (ia < ib);
}

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

static int cmp_str(const void *a, const void *b) {
    /* Array of strings: a and b point to char* elements */
    return strcmp(*(const char **)a, *(const char **)b);
}

/* Generic min/max: walk an array of `n` elements of `size` bytes */
static const void *generic_min(const void *arr, size_t n, size_t size,
                                int (*cmp)(const void *, const void *)) {
    const char *best = arr;
    for (size_t i = 1; i < n; i++) {
        const char *cur = (const char *)arr + i * size;
        if (cmp(cur, best) < 0) best = cur;
    }
    return best;
}

static void section_comparators(void) {
    printf("=== 2. Generic comparators ===\n");

    int    ints[]    = {5, 2, 8, 1, 9, 3};
    double doubles[] = {3.14, 1.41, 2.71, 0.57};
    const char *words[] = {"banana", "apple", "cherry", "date"};

    /* qsort is the stdlib generic sort */
    qsort(ints,    6, sizeof *ints,    cmp_int);
    qsort(doubles, 4, sizeof *doubles, cmp_double);
    qsort(words,   4, sizeof *words,   cmp_str);

    printf("sorted ints:    ");
    for (int i = 0; i < 6; i++) printf("%d ", ints[i]);

    printf("\nsorted doubles: ");
    for (int i = 0; i < 4; i++) printf("%.2f ", doubles[i]);

    printf("\nsorted words:   ");
    for (int i = 0; i < 4; i++) printf("%s ", words[i]);

    /* generic_min on the original (unsorted) array */
    int raw[] = {5, 2, 8, 1, 9, 3};
    const int *m = generic_min(raw, 6, sizeof *raw, cmp_int);
    printf("\nmin of {5,2,8,1,9,3} = %d\n", *m);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: bsearch — generic binary search
 *
 * bsearch scans a sorted array for a key.
 * Returns a void* to the matching element, or NULL.
 * ────────────────────────────────────────────────────────── */

static void section_bsearch(void) {
    printf("=== 3. bsearch ===\n");

    int arr[] = {1, 3, 5, 7, 9, 11, 13, 17, 19, 23};
    int n = (int)(sizeof arr / sizeof *arr);

    int keys[] = {7, 4, 23, 0};
    for (int i = 0; i < 4; i++) {
        int *hit = bsearch(&keys[i], arr, (size_t)n, sizeof *arr, cmp_int);
        if (hit)
            printf("bsearch(%2d): found at index %ld\n",
                   keys[i], (long)(hit - arr));
        else
            printf("bsearch(%2d): not found\n", keys[i]);
    }

    /* Works identically with strings */
    const char *words[] = {"apple", "banana", "cherry", "date", "fig"};
    const char *key = "cherry";
    const char **wp = bsearch(&key, words, 5, sizeof *words, cmp_str);
    printf("bsearch(\"%s\"): %s\n", key, wp ? "found" : "not found");

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: Generic dynamic array (GenVec)
 *
 * A single implementation that works for any element type,
 * parameterised only by element_size at construction time.
 * ────────────────────────────────────────────────────────── */

typedef struct {
    void   *data;
    size_t  len;
    size_t  cap;
    size_t  esize;   /* bytes per element */
} GenVec;

static GenVec genvec_new(size_t element_size) {
    return (GenVec){ .data = NULL, .len = 0, .cap = 0, .esize = element_size };
}

/* Returns 0 on success, -1 on allocation failure */
static int genvec_push(GenVec *v, const void *elem) {
    if (v->len == v->cap) {
        size_t newcap = v->cap ? v->cap * 2 : 4;
        void *p = realloc(v->data, newcap * v->esize);
        if (!p) return -1;
        v->data = p;
        v->cap  = newcap;
    }
    memcpy((char *)v->data + v->len * v->esize, elem, v->esize);
    v->len++;
    return 0;
}

/* Returns a pointer into the backing array — valid until the next push */
static void *genvec_get(const GenVec *v, size_t i) {
    if (i >= v->len) return NULL;
    return (char *)v->data + i * v->esize;
}

static void genvec_sort(GenVec *v, int (*cmp)(const void *, const void *)) {
    qsort(v->data, v->len, v->esize, cmp);
}

static void genvec_free(GenVec *v) {
    free(v->data);
    v->data = NULL;
    v->len = v->cap = 0;
}

typedef struct { char name[32]; int score; } Player;

static int cmp_player_score(const void *a, const void *b) {
    int sa = ((const Player *)a)->score;
    int sb = ((const Player *)b)->score;
    return (sb > sa) - (sb < sa);   /* descending */
}

static void section_genvec(void) {
    printf("=== 4. Generic dynamic array (GenVec) ===\n");

    /* --- integer GenVec --- */
    GenVec vi = genvec_new(sizeof(int));
    int nums[] = {42, 7, 99, 3, 55};
    for (int i = 0; i < 5; i++) genvec_push(&vi, &nums[i]);

    printf("int GenVec (before sort): ");
    for (size_t i = 0; i < vi.len; i++)
        printf("%d ", *(int *)genvec_get(&vi, i));

    genvec_sort(&vi, cmp_int);
    printf("\nint GenVec (after sort):  ");
    for (size_t i = 0; i < vi.len; i++)
        printf("%d ", *(int *)genvec_get(&vi, i));
    printf("\n");
    genvec_free(&vi);

    /* --- struct GenVec --- */
    GenVec vp = genvec_new(sizeof(Player));
    Player players[] = {
        {"Alice",  88}, {"Bob",   95}, {"Carol", 72},
        {"Dave",   88}, {"Eve",  100}
    };
    for (int i = 0; i < 5; i++) genvec_push(&vp, &players[i]);

    genvec_sort(&vp, cmp_player_score);
    printf("Players by score (desc):\n");
    for (size_t i = 0; i < vp.len; i++) {
        const Player *p = genvec_get(&vp, i);
        printf("  %2zu. %-8s %d\n", i+1, p->name, p->score);
    }
    genvec_free(&vp);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: Type tags — safe generic unions
 *
 * void * discards type information.  A type tag restores it:
 * store an enum alongside the pointer so the receiver can
 * branch on the actual type at runtime.
 * ────────────────────────────────────────────────────────── */

typedef enum { TAG_INT, TAG_DOUBLE, TAG_STR } Tag;

typedef struct {
    Tag   tag;
    union {
        int         i;
        double      d;
        const char *s;
    } val;
} Variant;

static Variant var_int(int i)         { return (Variant){ TAG_INT,    .val.i = i }; }
static Variant var_double(double d)   { return (Variant){ TAG_DOUBLE, .val.d = d }; }
static Variant var_str(const char *s) { return (Variant){ TAG_STR,    .val.s = s }; }

static void print_variant(const Variant *v) {
    switch (v->tag) {
        case TAG_INT:    printf("int(%d)",    v->val.i); break;
        case TAG_DOUBLE: printf("double(%.2f)", v->val.d); break;
        case TAG_STR:    printf("str(\"%s\")", v->val.s); break;
    }
}

static void section_type_tags(void) {
    printf("=== 5. Type tags (safe variants) ===\n");

    Variant items[] = {
        var_int(42),
        var_double(3.14),
        var_str("hello"),
        var_int(-7),
        var_str("world"),
    };
    size_t n = sizeof items / sizeof *items;

    for (size_t i = 0; i < n; i++) {
        printf("  items[%zu] = ", i);
        print_variant(&items[i]);
        printf("\n");
    }

    /* Compute sum of all numeric items */
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        if      (items[i].tag == TAG_INT)    sum += items[i].val.i;
        else if (items[i].tag == TAG_DOUBLE) sum += items[i].val.d;
    }
    printf("  sum of numeric items = %.2f\n", sum);

    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    section_void_ptr();
    section_comparators();
    section_bsearch();
    section_genvec();
    section_type_tags();
    return 0;
}
