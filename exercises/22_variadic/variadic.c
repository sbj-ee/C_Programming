/*
 * Exercise 22: Variadic Functions
 *
 * Every call to printf, scanf, and sprintf uses variadic arguments.
 * This exercise builds the mental model from scratch:
 *
 *   va_list   — opaque object that holds the argument state
 *   va_start  — initialise it, pointing past the last named parameter
 *   va_arg    — retrieve the next argument as a given type
 *   va_end    — release resources (required before return)
 *   va_copy   — duplicate a va_list so you can traverse twice
 *
 * The caller decides the types.  The callee must be told what to
 * expect, either through a count, a format string, or a sentinel.
 * There is no runtime type information — reading the wrong type is
 * undefined behaviour.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

/* ──────────────────────────────────────────────────────────
 * Section 1: va_list basics
 *
 * The simplest pattern: a count tells the callee how many
 * arguments to read.
 * ────────────────────────────────────────────────────────── */

/* Sum n integers */
static int sum(int n, ...) {
    va_list ap;
    va_start(ap, n);       /* ap now points to the arg after 'n' */
    int total = 0;
    for (int i = 0; i < n; i++)
        total += va_arg(ap, int);   /* retrieve one int, advance ap */
    va_end(ap);
    return total;
}

/* Maximum of n doubles */
static double maxd(int n, ...) {
    va_list ap;
    va_start(ap, n);
    double best = va_arg(ap, double);
    for (int i = 1; i < n; i++) {
        double v = va_arg(ap, double);
        if (v > best) best = v;
    }
    va_end(ap);
    return best;
}

/* Concatenate n strings into a freshly malloc'd buffer.
 * Caller owns the returned string. */
static char *join(int n, ...) {
    /* First pass: measure total length */
    va_list ap;
    va_start(ap, n);
    size_t total = 1;   /* null terminator */
    for (int i = 0; i < n; i++)
        total += strlen(va_arg(ap, const char *));
    va_end(ap);

    char *out = malloc(total);
    if (!out) return NULL;
    out[0] = '\0';

    /* Second pass: copy strings — needs a fresh va_list */
    va_start(ap, n);
    for (int i = 0; i < n; i++)
        strcat(out, va_arg(ap, const char *));
    va_end(ap);

    return out;
}

static void section_basics(void) {
    printf("=== 1. va_list basics ===\n");

    printf("sum(3, 1,2,3)          = %d\n",  sum(3, 1, 2, 3));
    printf("sum(5, 10,20,30,40,50) = %d\n",  sum(5, 10, 20, 30, 40, 50));
    printf("sum(0)                 = %d\n",  sum(0));

    printf("maxd(4, 1.1,9.9,3.3,7.7) = %.1f\n", maxd(4, 1.1, 9.9, 3.3, 7.7));

    char *s = join(4, "Hello", ", ", "world", "!");
    printf("join(4, ...)           = \"%s\"\n", s);
    free(s);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: Sentinel-terminated variadics
 *
 * Instead of a count, signal the end with a sentinel value.
 * NULL is the standard sentinel for pointer lists.
 * ────────────────────────────────────────────────────────── */

/* Count the strings passed, terminated by NULL */
static int count_args(const char *first, ...) {
    if (!first) return 0;
    int n = 1;
    va_list ap;
    va_start(ap, first);
    while (va_arg(ap, const char *) != NULL) n++;
    va_end(ap);
    return n;
}

/* Print every string, one per line, terminated by NULL */
static void print_all(const char *first, ...) {
    if (!first) return;
    printf("  %s\n", first);
    va_list ap;
    va_start(ap, first);
    const char *s;
    while ((s = va_arg(ap, const char *)) != NULL)
        printf("  %s\n", s);
    va_end(ap);
}

static void section_sentinel(void) {
    printf("=== 2. Sentinel-terminated ===\n");

    printf("count_args(\"a\",\"b\",\"c\",NULL) = %d\n",
           count_args("a", "b", "c", NULL));

    print_all("alpha", "beta", "gamma", "delta", NULL);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: va_copy
 *
 * va_copy duplicates a va_list so you can traverse the
 * argument list a second time without re-calling va_start
 * (which is impossible once the function is entered).
 *
 * Always pair va_copy with va_end on the copy.
 * ────────────────────────────────────────────────────────── */

/* Return mean and variance of n doubles in one pass */
static void stats(int n, double *mean_out, double *var_out, ...) {
    va_list ap, ap2;
    va_start(ap, var_out);
    va_copy(ap2, ap);     /* save a copy before first traversal */

    /* Pass 1: mean */
    double sum_v = 0.0;
    for (int i = 0; i < n; i++) sum_v += va_arg(ap, double);
    va_end(ap);
    *mean_out = sum_v / n;

    /* Pass 2: variance (using the copy) */
    double sq = 0.0;
    for (int i = 0; i < n; i++) {
        double d = va_arg(ap2, double) - *mean_out;
        sq += d * d;
    }
    va_end(ap2);
    *var_out = sq / n;
}

static void section_va_copy(void) {
    printf("=== 3. va_copy (two-pass) ===\n");

    double mean, var;
    stats(5, &mean, &var, 2.0, 4.0, 4.0, 4.0, 6.0);
    printf("data: {2, 4, 4, 4, 6}\n");
    printf("mean     = %.2f\n", mean);
    printf("variance = %.2f\n", var);
    printf("stddev   = %.4f\n", sqrt(var));   /* sqrt(1.6) ≈ 1.2649 */

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: Building printf-like functions
 *
 * vsnprintf / vfprintf accept a va_list instead of '...',
 * allowing you to wrap printf in your own functions.
 *
 * The pattern:
 *   void my_fn(const char *fmt, ...) {
 *       va_list ap;
 *       va_start(ap, fmt);
 *       vsnprintf(buf, size, fmt, ap);   // or vfprintf
 *       va_end(ap);
 *   }
 * ────────────────────────────────────────────────────────── */

typedef enum { LOG_INFO, LOG_WARN, LOG_ERROR } LogLevel;

static const char *level_name(LogLevel l) {
    switch (l) {
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        default:        return "?????";
    }
}

/* Printf-like logger with level prefix */
static void log_msg(LogLevel level, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    printf("[%s] %s\n", level_name(level), buf);
}

/* Builds a heap string from a format — like asprintf */
static char *fmt_alloc(const char *fmt, ...) {
    /* First call: measure the length */
    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (needed < 0) { va_end(ap2); return NULL; }
    char *buf = malloc((size_t)needed + 1);
    if (buf) vsnprintf(buf, (size_t)needed + 1, fmt, ap2);
    va_end(ap2);
    return buf;
}

static void section_printf_like(void) {
    printf("=== 4. printf-like wrappers ===\n");

    log_msg(LOG_INFO,  "Server started on port %d", 8080);
    log_msg(LOG_WARN,  "Connection from %s timed out after %ds", "10.0.0.5", 30);
    log_msg(LOG_ERROR, "Failed to open %s: %s", "/etc/secret", "Permission denied");

    char *greeting = fmt_alloc("Hello, %s! You have %d message%s.",
                               "Alice", 3, "s");
    printf("fmt_alloc -> \"%s\"\n", greeting);
    free(greeting);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: Type-safe variadic via tagged union + _Generic
 *
 * The danger of va_arg is that reading the wrong type is UB.
 * Wrap each argument in a tagged union before passing it, so
 * the callee always knows the real type.
 *
 * _Generic (C11) dispatches to a typed constructor at compile
 * time with no runtime overhead, giving a printf-like
 * ergonomic but with full type safety.
 * ────────────────────────────────────────────────────────── */

typedef enum { VTAG_INT, VTAG_DOUBLE, VTAG_STR } VTag;

typedef struct {
    VTag tag;
    union { long i; double d; const char *s; } v;
} VArg;

/* Typed constructors — one per accepted type, fully type-safe */
static VArg varg_int(long i)         { return (VArg){ VTAG_INT,    .v.i = i }; }
static VArg varg_double(double d)    { return (VArg){ VTAG_DOUBLE, .v.d = d }; }
static VArg varg_str(const char *s)  { return (VArg){ VTAG_STR,    .v.s = s }; }

/* Convenience shorthands */
#define VI(x) varg_int(x)
#define VD(x) varg_double(x)
#define VS(x) varg_str(x)

/* _Generic: compile-time type dispatch with no runtime cost.
 * Here it maps a value to a human-readable type-name string.
 * (Using _Generic to call typed constructors like varg_int is
 * blocked by GCC's non-standard behaviour of type-checking all
 * branches, not just the selected one — so use explicit macros.) */
#define TYPE_NAME(x) _Generic((x),          \
    int:          "int",                    \
    long:         "long",                   \
    double:       "double",                 \
    float:        "float",                  \
    char *:       "char *",                 \
    const char *: "const char *",           \
    default:      "other"                   \
)

static void print_vargs(int n, ...) {
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; i++) {
        VArg a = va_arg(ap, VArg);
        switch (a.tag) {
            case VTAG_INT:    printf("  [int]    %ld\n",    a.v.i); break;
            case VTAG_DOUBLE: printf("  [double] %.4f\n",   a.v.d); break;
            case VTAG_STR:    printf("  [str]    \"%s\"\n", a.v.s); break;
        }
    }
    va_end(ap);
}

static void section_type_safe(void) {
    printf("=== 5. Type-safe variadic (VArg tagged union) ===\n");

    /* Explicit typed constructors wrap each arg before passing.
     * The callee reads the tag — never touches the wrong union field. */
    print_vargs(5,
        VI(42),
        VD(3.14),
        VS("hello"),
        VI(-7),
        VD(2.71828)
    );

    /* _Generic: zero-cost compile-time type introspection */
    int    a = 0;
    double b = 0.0;
    const char *c = "";
    printf("TYPE_NAME(int a)         = %s\n", TYPE_NAME(a));
    printf("TYPE_NAME(double b)      = %s\n", TYPE_NAME(b));
    printf("TYPE_NAME(const char *c) = %s\n", TYPE_NAME(c));
    printf("TYPE_NAME(3.14f)         = %s\n", TYPE_NAME(3.14f));

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 6: Pitfalls
 * ────────────────────────────────────────────────────────── */

static void section_pitfalls(void) {
    printf("=== 6. Pitfalls ===\n");

    /* Pitfall 1: integer promotions
     * char and short are promoted to int before being passed.
     * Always va_arg(ap, int), never va_arg(ap, char). */
    printf("P1: char/short promote to int — use va_arg(ap, int)\n");

    /* Pitfall 2: float promotes to double.
     * Always va_arg(ap, double), never va_arg(ap, float). */
    printf("P2: float promotes to double — use va_arg(ap, double)\n");

    /* Pitfall 3: reading more args than were passed is UB. */
    printf("P3: reading past the end of the argument list is UB\n");

    /* Pitfall 4: forgetting va_end before return leaks resources
     * on implementations that allocate memory for va_list. */
    printf("P4: always call va_end before each return path\n");

    /* Pitfall 5: passing a va_list by value copies the state.
     * After the callee calls va_arg, the caller's list is not
     * advanced.  Use a pointer (&ap) or va_copy instead. */
    printf("P5: pass va_list by pointer or use va_copy, not by value\n");

    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    section_basics();
    section_sentinel();
    section_va_copy();
    section_printf_like();
    section_type_safe();
    section_pitfalls();
    return 0;
}
