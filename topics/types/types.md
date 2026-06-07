# Types

## Integer types

```c
#include <stdint.h>   // fixed-width types (C99)
#include <limits.h>   // INT_MAX, UINT64_MAX, etc.

// Signed
int8_t   // -128 .. 127
int16_t  // -32768 .. 32767
int32_t  // -2147483648 .. 2147483647
int64_t  // -9223372036854775808 .. 9223372036854775807

// Unsigned
uint8_t  uint16_t  uint32_t  uint64_t

// Platform-dependent (avoid for portability)
char        // signed or unsigned — implementation defined
short       // at least 16 bits
int         // at least 16 bits (usually 32)
long        // at least 32 bits (32 on Windows, 64 on Linux/macOS)
long long   // at least 64 bits

// For sizes and array indices — use these, not int
size_t      // unsigned, result of sizeof
ptrdiff_t   // signed, result of pointer subtraction
ssize_t     // signed size (POSIX — Portable Operating System Interface,
            // the standard that extends C11 with OS-level APIs on Linux/Unix)
```

### Literal suffixes

```c
42          // int
42U         // unsigned int
42L         // long
42UL        // unsigned long
42LL        // long long
42ULL       // unsigned long long
0x2A        // hex int (42)
052         // octal int (42)
// 0b101010  — binary: GCC extension, not C11 standard; use 0x2A instead
```

## Floating-point types

```c
float       // 32-bit IEEE 754, ~7 significant digits
double      // 64-bit IEEE 754, ~15 significant digits — prefer this
long double // 80-bit on x86, 128-bit on some platforms

3.14        // double literal
3.14f       // float literal
3.14L       // long double literal

// M_PI is not in C11 — define it manually or use POSIX
#define M_PI 3.14159265358979323846
```

### Floating-point pitfalls

```c
// Never compare floats with ==
0.1 + 0.2 == 0.3   // false! (binary representation error)
fabs((0.1 + 0.2) - 0.3) < 1e-9   // use epsilon comparison

// NaN is not equal to anything, including itself
double nan = 0.0 / 0.0;
nan == nan   // false; use isnan(nan) from <math.h>
```

## Character type

```c
char c = 'A';       // 65 in ASCII
int  i = (int)c;    // explicit cast for arithmetic

// char may be signed or unsigned — for byte manipulation use:
unsigned char b = 0xFF;

// For type-safe character classification (<ctype.h>):
isdigit(c)   isalpha(c)   isspace(c)   toupper(c)   tolower(c)
// Pass (unsigned char)c or cast — undefined behaviour (UB: the standard
// imposes no requirement; the program may crash or produce wrong results)
// on negative chars
```

## Boolean

```c
#include <stdbool.h>  // C99

bool flag = true;     // bool, true, false are macros for _Bool, 1, 0
_Bool b   = 0;        // C11 built-in type
```

## Enumerations

```c
typedef enum { MON=1, TUE, WED, THU, FRI, SAT, SUN } Day;
// Underlying type is int (implementation-defined, usually)

// Bit-flag enum — values must be distinct powers of 2
typedef enum {
    FLAG_READ    = 1 << 0,
    FLAG_WRITE   = 1 << 1,
    FLAG_EXECUTE = 1 << 2,
} Permission;

Permission p = FLAG_READ | FLAG_WRITE;
if (p & FLAG_WRITE) { /* ... */ }
```

## sizeof and alignof

```c
sizeof(int)          // bytes — always compile-time except for VLAs
sizeof(arr)          // total bytes of array (not pointer!)
sizeof(arr)/sizeof(*arr)  // element count

#include <stdalign.h>  // C11
alignof(double)      // alignment requirement in bytes

// For portability use the right format specifier:
printf("%zu", sizeof(int));   // size_t → %zu
printf("%td", ptr1 - ptr2);  // ptrdiff_t → %td
```

## Type conversions

### Implicit promotions

```c
// Integer promotion: char, short → int before arithmetic
char a = 200, b = 100;
int r = a + b;   // a and b promoted to int before addition

// Usual arithmetic conversion: narrower type converts to wider
int    + double → double
int    + long   → long
signed + unsigned → unsigned (potential surprise for negative values)
```

### Explicit casts

```c
double d = 3.9;
int    i = (int)d;        // truncates toward zero → 3

// Widening — always safe
int    n  = 42;
long   l  = (long)n;      // no data loss

// Narrowing — may lose data; no warning by default
long   big = 70000;
short  s   = (short)big;  // wrap-around — compile with -Wconversion to catch

// Type punning — use memcpy, not casts
float f = 1.0f;
uint32_t bits;
memcpy(&bits, &f, 4);     // portable; avoids strict-aliasing UB (Undefined Behaviour)
```

## Common sizes (64-bit Linux)

| Type | Size | Range |
|------|------|-------|
| `char` | 1 | -128..127 |
| `short` | 2 | -32768..32767 |
| `int` | 4 | ±2.1 billion |
| `long` | 8 | ±9.2 × 10¹⁸ |
| `long long` | 8 | ±9.2 × 10¹⁸ |
| `float` | 4 | ~7 decimal digits |
| `double` | 8 | ~15 decimal digits |
| `pointer` | 8 | — |
