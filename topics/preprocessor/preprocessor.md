# Preprocessor

The preprocessor runs before compilation. It performs text substitution,
file inclusion, and conditional compilation — without any knowledge of C
types or scopes.

## Object-like macros

```c
#define PI      3.14159265358979323846
#define BUFSIZE 1024
#define MAX_NODES 256

// Avoid bare macros for constants — prefer typed constants or enums:
static const double PI = 3.14159265358979323846;  // better: has a type
enum { BUFSIZE = 1024 };                           // better: scoped
```

## Function-like macros

The body must be fully parenthesised to avoid precedence surprises.

```c
// BAD — operator precedence trap
#define SQ(x)   x * x         // SQ(1+2) → 1+2*1+2 = 5, not 9

// GOOD — wrap args and whole expression
#define SQ(x)   ((x) * (x))   // SQ(1+2) → ((1+2)*(1+2)) = 9

// Multi-statement macro: wrap in do { } while(0)
#define SWAP(a, b, T) do { T _tmp = (a); (a) = (b); (b) = _tmp; } while(0)
SWAP(x, y, int);   // safe in if/else without braces

// MIN/MAX with one evaluation each (use a statement expression — GCC ext)
// Portable alternative: inline function
static inline int max_int(int a, int b) { return a > b ? a : b; }
```

## Stringification (`#`) and token-pasting (`##`)

```c
// # converts the argument to a string literal
#define STRINGIFY(x)   #x
STRINGIFY(hello)     →  "hello"
STRINGIFY(1 + 2)     →  "1 + 2"

// ## concatenates two tokens
#define FIELD(prefix, name)  prefix##_##name
FIELD(my, value)     →  my_value

// Common use: enum ↔ string table via X-macro
#define COLORS  X(RED) X(GREEN) X(BLUE)
typedef enum { COLORS } Color;          // RED=0, GREEN=1, BLUE=2
const char *color_names[] = {
#define X(c) #c,
    COLORS
#undef X
};
```

## Variadic macros

```c
// __VA_ARGS__ receives everything after the last named param
#define LOG(fmt, ...)   fprintf(stderr, "[LOG] " fmt "\n", __VA_ARGS__)
LOG("x=%d y=%d", x, y);

// If the variadic part might be empty, at least one arg is safer:
#define LOGF(fmt, ...)  fprintf(stderr, fmt "\n", __VA_ARGS__)
// On some compilers ##__VA_ARGS__ swallows the preceding comma when empty
// (GCC extension — not standard C11)
```

## Include guards

Prevent a header from being processed more than once per translation unit.

```c
// mylib.h
#ifndef MYLIB_H
#define MYLIB_H

// ... declarations ...

#endif /* MYLIB_H */

// Alternative (non-standard but widely supported):
#pragma once
```

## Conditional compilation

```c
#ifdef DEBUG
    printf("debug: x=%d\n", x);
#endif

#if defined(PLATFORM_LINUX)
    // Linux-specific code
#elif defined(PLATFORM_MACOS)
    // macOS-specific code
#else
    #error "Unsupported platform"
#endif

// Check C standard version
#if __STDC_VERSION__ >= 201112L
    // C11 feature
#endif

// Compile with -DDEBUG to enable debug output
// gcc -DDEBUG -DPLATFORM_LINUX prog.c -o prog
```

## Predefined macros

```c
__FILE__      // string: current source file name
__LINE__      // int: current line number
__func__      // string: current function name (C99)
__DATE__      // string: compilation date "Mmm dd yyyy"
__TIME__      // string: compilation time "hh:mm:ss"
__STDC_VERSION__  // long: C standard (201112L for C11)
```

## Assertions

```c
#include <assert.h>

assert(ptr != NULL);   // aborts with message if false; disabled by -DNDEBUG

// Static assert — compile-time check
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Unparenthesised macro arguments | Wrap every use of every arg in `()` |
| Macro argument evaluated twice | `SQ(x++)` expands `x++` twice — use inline function |
| Missing `do { } while(0)` in multi-statement macros | Breaks `if/else` without braces |
| Using `#ifdef` for values | Use `#if VALUE` or `#if defined(X)` |
| No include guard | Header processed multiple times → redefinition errors |
