# Pointers

A pointer stores a memory address. Every pointer has a type that determines
how dereferencing interprets the bytes at that address.

## Declaration and basic use

```c
int  x  = 42;
int *p  = &x;      // & gives the address of x
int  y  = *p;      // * dereferences: reads the value at the address
*p = 99;           // write through the pointer — x is now 99

int *uninit;       // NOT initialised — reading *uninit is UB
int *null = NULL;  // safe "no value" sentinel; dereferencing is a crash
```

## Pointer arithmetic

Arithmetic on a pointer steps by `sizeof(T)`, not by bytes.

```c
int arr[5] = {10, 20, 30, 40, 50};
int *p = arr;       // arr decays to &arr[0]

p + 2;              // points to arr[2]  (+2 * sizeof(int) bytes)
*(p + 2);           // 30
p[2];               // same thing — indexing is syntactic sugar

// Iterate with pointer arithmetic
for (int *q = arr; q < arr + 5; q++)
    printf("%d ", *q);

// Pointer difference gives element count (not bytes)
int *end = arr + 5;
ptrdiff_t n = end - arr;   // 5
```

## Pointers and arrays

An array name decays to a pointer to its first element in most expressions.
The exceptions are `sizeof`, `&`, and string-literal initialisation.

```c
int a[3] = {1, 2, 3};
int *p = a;          // p == &a[0]; no & needed

// These are equivalent:
a[i]   ==  *(a + i)
p[i]   ==  *(p + i)

// sizeof difference — know this
sizeof(a)   // 12 (3 ints × 4 bytes)
sizeof(p)   // 8  (pointer size on 64-bit)
```

## Pointer to pointer

```c
int  x  = 7;
int *p  = &x;
int **pp = &p;      // pointer to the pointer

**pp = 99;          // modifies x
*pp  = NULL;        // modifies p (sets it to NULL)
```

Common uses: out-parameters that return a pointer, argv, 2-D arrays.

```c
void alloc_buf(char **out, size_t n) {
    *out = malloc(n);
}

char *buf;
alloc_buf(&buf, 128);
// buf is now allocated
free(buf);
```

## const correctness

Read right-to-left: "pointer to const int" vs "const pointer to int".

```c
const int *p;         // pointer to const int — cannot write *p
int *const p;         // const pointer to int — cannot change p
const int *const p;   // both const

// Passing read-only data — prevents accidental writes
size_t strlen(const char *s);
```

## void pointers

`void *` is compatible with every pointer type. You cannot dereference it
directly — cast to a concrete type first. Used for generic interfaces.

```c
void *generic_alloc(size_t n) { return malloc(n); }

int *arr = generic_alloc(10 * sizeof(int));  // implicit in C
arr[0] = 42;

// Byte-level traversal
void *base = arr;
int *elem = (int *)((char *)base + 2 * sizeof(int));  // arr[2]
```

## Function pointers

```c
// Declare: return_type (*name)(param_types)
int (*cmp)(const void *, const void *);

// Assign
cmp = strcmp;          // function name decays to a pointer
int r = cmp("a","b");  // call through the pointer

// In a struct — dispatch table (vtable-like)
typedef struct {
    int (*compare)(int, int);
    void (*print)(int);
} Ops;

// typedef makes the syntax readable
typedef int (*Comparator)(const void *, const void *);
Comparator c = cmp_int;
qsort(arr, n, sizeof(int), c);
```

## Common pitfalls

| Pitfall | Consequence | Fix |
|---------|-------------|-----|
| Dereferencing NULL | Segfault | Check `if (p != NULL)` |
| Returning a pointer to a local variable | Dangling pointer (UB) | Heap-allocate or use out-param |
| Pointer arithmetic beyond array bounds | UB | Stay within `[arr, arr+n]` |
| Casting `int *` to `char *` and back | Alignment UB on strict platforms | Use `memcpy` for type-punning |
| Using `==` to compare strings | Compares addresses, not content | Use `strcmp` |
| Forgetting that array decay loses size | `sizeof(p)` is pointer size | Track length separately |
