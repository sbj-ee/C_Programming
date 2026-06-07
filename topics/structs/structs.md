# Structs

A struct is a named collection of fields stored contiguously in memory
(with padding for alignment). Unlike arrays, fields can have different types.

## Declaration and typedef

```c
// Plain struct — must write "struct Point" everywhere
struct Point { double x, y; };
struct Point p = {1.0, 2.0};

// typedef — write "Point" everywhere
typedef struct {
    double x, y;
} Point;
Point p = {1.0, 2.0};

// Named tag + typedef — allows self-reference (linked lists, trees)
typedef struct Node {
    int          value;
    struct Node *next;   // must use "struct Node" here, not "Node"
} Node;
```

## Member access

```c
Point p = {3.0, 4.0};

p.x;       // direct access (value)
p.x = 9;   // assignment

Point *pp = &p;
(*pp).x;   // dereference then member
pp->x;     // arrow: shorthand for (*pp).x — prefer this
```

## Initialisation

```c
// Positional (order matters)
Point p1 = {1.0, 2.0};

// Designated initialiser (C99+) — order-independent, clear intent
Point p2 = { .x = 1.0, .y = 2.0 };

// Partial — unspecified fields are zero-initialised
typedef struct { int a, b, c; } Triple;
Triple t = { .b = 5 };   // a=0, b=5, c=0

// Zero-initialise everything
Triple z = {0};

// Compound literal — anonymous temporary struct
Point midpoint(Point a, Point b) {
    return (Point){ (a.x + b.x) / 2, (a.y + b.y) / 2 };
}
```

## Nested structs

```c
typedef struct {
    Point  center;
    double radius;
} Circle;

Circle c = { .center = {0, 0}, .radius = 5.0 };
printf("%.1f\n", c.center.x);
c.center.y = 3.0;
```

## Structs on the heap

```c
Node *new_node(int val) {
    Node *n = malloc(sizeof *n);   // sizeof *n, not sizeof(Node) — avoids type repeat
    if (!n) return NULL;
    n->value = val;
    n->next  = NULL;
    return n;
}
free(n);
```

## Passing structs

Small structs are passed by value (copied). For large structs or mutation,
pass a pointer.

```c
// By value — function gets its own copy
double length(Point p) {
    return sqrt(p.x*p.x + p.y*p.y);
}

// By pointer — efficient for large structs; can mutate
void translate(Point *p, double dx, double dy) {
    p->x += dx;
    p->y += dy;
}

// Const pointer — read-only, efficient
void print_point(const Point *p) {
    printf("(%.2f, %.2f)\n", p->x, p->y);
}
```

## Flexible array member (C99)

A struct can end with an unsized array. The struct is always heap-allocated
with extra space for the array.

```c
typedef struct {
    size_t len;
    int    data[];    // flexible array member — must be last
} IntVec;

IntVec *iv = malloc(sizeof(IntVec) + 10 * sizeof(int));
iv->len = 10;
iv->data[0] = 42;
```

## Bit fields

Pack multiple small integers into one word. Useful for hardware registers
and binary protocol headers.

```c
typedef struct {
    unsigned int valid  : 1;   // 1 bit
    unsigned int flags  : 3;   // 3 bits
    unsigned int value  : 12;  // 12 bits
} Packed;

Packed p = { .valid = 1, .flags = 0b101, .value = 255 };
```

## Memory layout and padding

The compiler inserts padding bytes so each field is aligned to its natural
alignment. Use `offsetof` to find exact positions.

```c
#include <stddef.h>

typedef struct { char a; int b; char c; } Gapped;
// Layout (typical x86-64):
// offset 0: a (1 byte)
// offset 1-3: padding (3 bytes)
// offset 4: b (4 bytes)
// offset 8: c (1 byte)
// offset 9-11: trailing padding (3 bytes)
// sizeof(Gapped) == 12, not 6

printf("%zu\n", offsetof(Gapped, b));   // 4

// Reorder to minimise padding: largest fields first
typedef struct { int b; char a; char c; } Tight;
// sizeof(Tight) == 8 (or even 6 on some ABI — Application Binary Interface,
// the platform-specific rules for struct layout and alignment)
```

## Common patterns

```c
// Tagged union — struct + enum for type-safe variant
typedef enum { INT_VAL, FLOAT_VAL, STR_VAL } Tag;
typedef struct {
    Tag tag;
    union { int i; double d; const char *s; } val;
} Variant;

// X-macro for struct introspection (see preprocessor topic)
// Function pointer in struct — simple vtable
typedef struct {
    void (*draw)(void *self);
    void (*free)(void *self);
} Shape;
```
