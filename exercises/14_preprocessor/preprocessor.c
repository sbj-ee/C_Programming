/* The preprocessor runs before the compiler. It performs text substitution,
   file inclusion, and conditional compilation. None of it is type-checked. */

/* ================================================================
   INCLUDE GUARDS — prevent a header from being processed twice
   if it is #included from multiple translation units.
   (Shown here inline for demonstration; normally live in .h files.)
   ================================================================ */
#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <stdarg.h>   /* va_list, va_start, va_arg, va_end */

/* ================================================================
   OBJECT-LIKE MACROS — simple text substitution
   ================================================================ */
#define PI          3.14159265358979323846
#define MAX_NAME    64
#define KILO        1024
#define MEGA        (KILO * KILO)       /* parentheses prevent precedence bugs */
#define GREETING    "Hello"

/* ================================================================
   FUNCTION-LIKE MACROS — parameterised substitution
   Always parenthesise every parameter and the whole expression.
   ================================================================ */
#define SQ(x)           ((x) * (x))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define ABS(x)          ((x) < 0 ? -(x) : (x))
#define CLAMP(v, lo, hi) (MAX((lo), MIN((v), (hi))))
#define ARRAY_LEN(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define SWAP(T, a, b)   do { T _tmp = (a); (a) = (b); (b) = _tmp; } while (0)

/* ================================================================
   STRINGIFICATION (#) and TOKEN PASTING (##)
   ================================================================ */
#define STRINGIFY(x)    #x
#define TO_STRING(x)    STRINGIFY(x)    /* double expansion resolves macros first */

#define MAKE_PAIR(name) int name##_key; char name##_val[32]

/* ================================================================
   VARIADIC MACRO — __VA_ARGS__ captures any number of arguments
   ================================================================ */
#define LOG(fmt, ...)   printf("[LOG] " fmt "\n", __VA_ARGS__)
#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("ASSERT failed: %s  (%s:%d)\n", \
                   #cond, __FILE__, __LINE__); \
        } \
    } while (0)

/* ================================================================
   PREDEFINED MACROS
   __FILE__   current source filename (string)
   __LINE__   current line number (integer)
   __DATE__   compilation date (string)
   __TIME__   compilation time (string)
   __func__   current function name (C99, not a macro but behaves like one)
   ================================================================ */

/* ================================================================
   CONDITIONAL COMPILATION
   ================================================================ */
#define VERSION_MAJOR 1
#define VERSION_MINOR 3

#ifdef _WIN32
    #define PLATFORM "Windows"
#elif defined(__APPLE__)
    #define PLATFORM "macOS"
#elif defined(__linux__)
    #define PLATFORM "Linux"
#else
    #define PLATFORM "Unknown"
#endif

/* Debug logging: compiled out entirely in release builds.
   Compile with -DDEBUG to enable: gcc -DDEBUG ... */
#ifdef DEBUG
    #define DBG(fmt, ...) printf("[DBG %s:%d] " fmt "\n", \
                                 __func__, __LINE__, __VA_ARGS__)
#else
    #define DBG(fmt, ...)   /* expands to nothing */
#endif

#endif /* PREPROCESSOR_H */

/* ================================================================
   MAIN
   ================================================================ */
static void show_function_info(void);
static void demo_multiline_macro(int *arr, int len);

int main(void) {

    /* --- Object-like macros --- */
    printf("=== Object-like macros ===\n");
    printf("PI      = %.10f\n", PI);
    printf("KILO    = %d\n",    KILO);
    printf("MEGA    = %d\n",    MEGA);
    printf("GREETING= %s\n",    GREETING);
    char name[MAX_NAME];
    snprintf(name, sizeof(name), "Alice");
    printf("name    = %s  (MAX_NAME=%d)\n", name, MAX_NAME);

    /* --- Function-like macros --- */
    printf("\n=== Function-like macros ===\n");
    printf("SQ(5)            = %d\n", SQ(5));
    printf("MAX(3, 7)        = %d\n", MAX(3, 7));
    printf("MIN(3, 7)        = %d\n", MIN(3, 7));
    printf("ABS(-42)         = %d\n", ABS(-42));
    printf("CLAMP(15, 0, 10) = %d\n", CLAMP(15, 0, 10));
    printf("CLAMP(-3, 0, 10) = %d\n", CLAMP(-3, 0, 10));

    int nums[] = {10, 20, 30, 40, 50};
    printf("ARRAY_LEN(nums)  = %zu\n", ARRAY_LEN(nums));

    /* SWAP uses a temporary via a compound statement */
    int a = 3, b = 9;
    printf("before SWAP: a=%d b=%d\n", a, b);
    SWAP(int, a, b);
    printf("after  SWAP: a=%d b=%d\n", a, b);

    /* --- Precedence hazard: why macros need parens --- */
    printf("\n=== Precedence hazard ===\n");
    /* If SQ were defined as  x*x  (no parens), SQ(1+2) would expand to
       1+2*1+2 = 5, not 9.  With ((x)*(x)) it expands to ((1+2)*(1+2)) = 9. */
    printf("SQ(1+2) = %d  (would be 5 without parentheses)\n", SQ(1 + 2));

    /* --- Stringification --- */
    printf("\n=== Stringification (#) ===\n");
    printf("STRINGIFY(hello world) = \"%s\"\n", STRINGIFY(hello world));
    printf("PI as string           = \"%s\"\n", TO_STRING(PI));
    /* TO_STRING expands PI first, then stringifies the number */

    /* --- Token pasting --- */
    printf("\n=== Token pasting (##) ===\n");
    struct { MAKE_PAIR(user); } record;
    record.user_key = 42;
    snprintf(record.user_val, sizeof(record.user_val), "Alice");
    printf("user_key=%d  user_val=%s\n", record.user_key, record.user_val);

    /* --- Variadic macro --- */
    printf("\n=== Variadic macro (LOG) ===\n");
    LOG("server started on port %d", 8080);
    LOG("connected: %s", "192.168.1.1");
    LOG("%s", "no variadic args — format string is the only required arg");

    /* --- ASSERT macro --- */
    printf("\n=== ASSERT macro ===\n");
    ASSERT(1 + 1 == 2);            /* passes silently */
    ASSERT(SQ(3) == 9);            /* passes silently */
    ASSERT(MAX(2, 5) == 10);       /* fails — prints diagnostic with expression text */

    /* --- Predefined macros --- */
    printf("\n=== Predefined macros ===\n");
    printf("__FILE__ = %s\n", __FILE__);
    printf("__LINE__ = %d\n", __LINE__);
    printf("__DATE__ = %s\n", __DATE__);
    printf("__TIME__ = %s\n", __TIME__);
    show_function_info();

    /* --- Conditional compilation --- */
    printf("\n=== Conditional compilation ===\n");
    printf("PLATFORM        = %s\n", PLATFORM);
    printf("VERSION         = %s.%s\n", TO_STRING(VERSION_MAJOR),
                                        TO_STRING(VERSION_MINOR));
    DBG("%s", "this only appears when compiled with -DDEBUG");

    /* --- do-while(0) wrapper --- why macros use it --- */
    printf("\n=== do-while(0) wrapper ===\n");
    /* Wrapping in do { ... } while (0) makes the macro safe inside an
       if-else without braces, because it forms a single statement. */
    if (1)
        SWAP(int, a, b);   /* safe: do-while(0) is one statement */
    printf("SWAP result: a=%d b=%d\n", a, b);

    /* --- Multiline macro demonstration --- */
    printf("\n=== Multiline macro ===\n");
    demo_multiline_macro(nums, ARRAY_LEN(nums));

    return 0;
}

static void show_function_info(void) {
    printf("called from function: %s\n", __func__);
}

/* Macro that prints each element — backslash continues the line */
#define PRINT_ARRAY(arr, len, fmt) \
    do { \
        printf("["); \
        for (int _i = 0; _i < (len); _i++) \
            printf(((_i) < (len)-1) ? fmt ", " : fmt, (arr)[_i]); \
        printf("]\n"); \
    } while (0)

static void demo_multiline_macro(int *arr, int len) {
    printf("PRINT_ARRAY: ");
    PRINT_ARRAY(arr, len, "%d");
}
