# Memory Management

## Storage classes

| Region | Lifetime | Size known at? | Example |
|--------|----------|----------------|---------|
| Stack | Until function returns | Compile time | `int x;` |
| Heap | Until `free()` | Runtime | `malloc(n)` |
| Static / BSS (Block Started by Symbol) | Entire program | Compile time | `static int x;` `int y;` (file-scope) |
| Read-only | Entire program | Compile time | `"string literal"` |

The stack is fast but limited (~1–8 MB). Use the heap for large or
variable-length data.

## Allocation functions

```c
#include <stdlib.h>

// Allocate n bytes — contents are uninitialised
void *malloc(size_t n);

// Allocate count×size bytes, zero-initialised
void *calloc(size_t count, size_t size);

// Resize an existing allocation — may move it
// Returns NULL on failure; old pointer still valid if NULL returned
void *realloc(void *ptr, size_t new_size);

// Release memory — ptr must be malloc'd; free(NULL) is safe
void free(void *ptr);
```

### Patterns

```c
// Single object
Node *n = malloc(sizeof *n);   // sizeof *n avoids repeating the type
if (!n) { /* handle OOM */ }

// Array of n elements — calloc zeroes them
int *arr = calloc(n, sizeof *arr);

// Growing buffer
size_t cap = 4;
int *buf = malloc(cap * sizeof *buf);
// ... later:
if (len == cap) {
    cap *= 2;
    int *tmp = realloc(buf, cap * sizeof *buf);
    if (!tmp) { free(buf); return -1; }  // keep old buf valid
    buf = tmp;
}

// Always set pointer to NULL after free to catch use-after-free early
free(buf);
buf = NULL;
```

## Ownership rules

C has no garbage collector. Every `malloc` must have exactly one `free`.

1. **Allocator owns** — function that calls malloc is responsible for free.
2. **Transfer** — document when a function takes ownership of a pointer.
3. **Borrow** — a function that receives a pointer but doesn't free it.

```c
// Allocator: caller must free the result
char *dup_str(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (p) memcpy(p, s, strlen(s) + 1);
    return p;   // caller owns this
}

// Consumer: borrows the pointer, does not free
void print_str(const char *s) {
    printf("%s\n", s);
}
```

## goto cleanup pattern

Avoids leaks when multiple resources are acquired:

```c
int process(const char *path) {
    int   rc  = -1;
    FILE *f   = NULL;
    char *buf = NULL;

    f = fopen(path, "r");
    if (!f)   goto done;

    buf = malloc(4096);
    if (!buf) goto done;

    // ... do work ...
    rc = 0;

done:
    free(buf);
    if (f) fclose(f);
    return rc;
}
```

## Common memory errors

| Error | Description | Detection |
|-------|-------------|-----------|
| Buffer overflow | Write past end of allocation | Valgrind, ASan |
| Use after free | Read/write freed memory | Valgrind, ASan |
| Double free | `free` same pointer twice | Valgrind, ASan |
| Memory leak | Allocation never freed | Valgrind |
| Uninitialised read | Use `malloc`'d memory before writing | Valgrind |
| Stack overflow | Deep recursion or large stack array | Crash (no stack guard) |

## Valgrind

```bash
valgrind --leak-check=full --track-origins=yes ./prog

# Key output lines:
# "ERROR SUMMARY: 0 errors" — clean
# "definitely lost: N bytes" — leak
# "Invalid read/write of size N" — buffer overflow / use-after-free
```

## AddressSanitizer (faster than Valgrind)

```bash
gcc -Wall -g -fsanitize=address,undefined prog.c -o prog
./prog
# Reports at the point of the error with a stack trace
```

## Stack vs heap decision guide

| Use stack when | Use heap when |
|---------------|---------------|
| Size known at compile time | Size known only at runtime |
| Small (≤ a few KB) | Large (> a few KB) |
| Lifetime ≤ function call | Lifetime crosses function calls |
| No returning address of it | Result must outlive the call |
