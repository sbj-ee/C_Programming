/*
 * Exercise 20: Error Handling
 *
 * Topics:
 *   - Return codes and sentinel values (-1, NULL)
 *   - errno, perror, strerror
 *   - Error propagation through a call stack
 *   - goto cleanup pattern for resource teardown
 *   - setjmp / longjmp for non-local jumps
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>

/* ──────────────────────────────────────────────────────────
 * Section 1: Return codes and sentinel values
 *
 * The canonical C idiom: return 0 (or a valid pointer) on
 * success; return -1 / NULL on failure, and set errno.
 * ────────────────────────────────────────────────────────── */

/* Parses an integer from a string.  Returns 0 on success,
 * -1 if the string is NULL, empty, or contains non-digits. */
int parse_int(const char *s, int *out) {
    if (s == NULL || *s == '\0') { errno = EINVAL; return -1; }

    char *end;
    errno = 0;
    long val = strtol(s, &end, 10);
    if (errno != 0)   return -1;   /* overflow / underflow */
    if (*end != '\0') { errno = EINVAL; return -1; }  /* trailing garbage */

    *out = (int)val;
    return 0;
}

static void section_return_codes(void) {
    printf("=== 1. Return codes ===\n");

    int result;
    if (parse_int("42", &result) == 0)
        printf("parse_int(\"42\")  -> %d\n", result);

    if (parse_int("abc", &result) == -1)
        printf("parse_int(\"abc\") -> error: %s\n", strerror(errno));

    if (parse_int("", &result) == -1)
        printf("parse_int(\"\")    -> error: %s\n", strerror(errno));

    if (parse_int("3x7", &result) == -1)
        printf("parse_int(\"3x7\") -> error: %s\n", strerror(errno));

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: errno, perror, strerror
 *
 * errno is a global (thread-local in modern C) int set by
 * failing system calls and C library functions.
 *
 *   perror(msg)        prints "msg: <error string>\n" to stderr
 *   strerror(errno)    returns a pointer to the error string
 *   strerror_r(...)    thread-safe variant (POSIX)
 * ────────────────────────────────────────────────────────── */

static void section_errno(void) {
    printf("=== 2. errno / perror / strerror ===\n");

    /* Provoke a real OS error: open a file that doesn't exist */
    FILE *f = fopen("/tmp/does_not_exist_xyzzy_12345.txt", "r");
    if (f == NULL) {
        /* errno is set to ENOENT */
        printf("errno value : %d\n", errno);
        printf("strerror    : %s\n", strerror(errno));
        /* perror prints to stderr; we replicate to stdout for clarity */
        printf("(perror would print): fopen failed: %s\n", strerror(errno));
    }

    /* errno is NOT automatically reset between calls.
     * Always check immediately after the failing call. */
    errno = 0;   /* manual reset before the next operation */

    /* Common errno values */
    printf("\nCommon errno constants:\n");
    printf("  EPERM  = %d  (operation not permitted)\n", EPERM);
    printf("  ENOENT = %d  (no such file or directory)\n", ENOENT);
    printf("  EINVAL = %d  (invalid argument)\n",          EINVAL);
    printf("  ENOMEM = %d  (out of memory)\n",             ENOMEM);
    printf("  ERANGE = %d  (result out of range)\n",       ERANGE);
    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: Error propagation
 *
 * Each function in a call stack checks its callee and returns
 * -1 upward on failure, preserving the errno that was set.
 * ────────────────────────────────────────────────────────── */

static int read_config_value(const char *key, int *out) {
    /* Simulate a config lookup that can fail */
    if (strcmp(key, "port") == 0) { *out = 8080; return 0; }
    if (strcmp(key, "timeout") == 0) { *out = 30;   return 0; }
    errno = ENOENT;
    return -1;
}

static int load_config(int *port, int *timeout) {
    if (read_config_value("port",    port)    != 0) return -1;
    if (read_config_value("timeout", timeout) != 0) return -1;
    return 0;
}

static int start_server(void) {
    int port, timeout;
    if (load_config(&port, &timeout) != 0) return -1;  /* propagate */
    printf("Server started on port %d (timeout=%ds)\n", port, timeout);
    return 0;
}

static void section_propagation(void) {
    printf("=== 3. Error propagation ===\n");

    if (start_server() == 0)
        printf("start_server(): OK\n");
    else
        printf("start_server(): failed — %s\n", strerror(errno));

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: goto cleanup pattern
 *
 * When a function acquires multiple resources, early-exit on
 * error risks leaks.  goto lets us jump to a single cleanup
 * block, freeing exactly what was allocated.
 * ────────────────────────────────────────────────────────── */

static int process_file(const char *path) {
    int   rc  = -1;
    FILE *in  = NULL;
    char *buf = NULL;

    in = fopen(path, "r");
    if (in == NULL) { goto cleanup; }          /* errno already set */

    buf = malloc(256);
    if (buf == NULL) { errno = ENOMEM; goto cleanup; }

    if (fgets(buf, 256, in) == NULL) {
        errno = EIO;
        goto cleanup;
    }

    /* Strip trailing newline for clean output */
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';

    printf("First line: \"%s\"\n", buf);
    rc = 0;   /* success — fall through to cleanup */

cleanup:
    free(buf);          /* free(NULL) is a safe no-op */
    if (in) fclose(in);
    return rc;
}

static void section_goto_cleanup(void) {
    printf("=== 4. goto cleanup pattern ===\n");

    /* Write a temp file, process it, then try a bad path */
    const char *tmp = "/tmp/ex20_sample.txt";
    FILE *f = fopen(tmp, "w");
    if (f) { fprintf(f, "hello from exercise 20\n"); fclose(f); }

    if (process_file(tmp) != 0)
        printf("process_file(%s): %s\n", tmp, strerror(errno));

    if (process_file("/tmp/no_such_file_ex20.txt") != 0)
        printf("process_file(missing): %s\n", strerror(errno));

    remove(tmp);
    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: setjmp / longjmp
 *
 * setjmp() saves the current execution context into a jmp_buf.
 * longjmp() restores that context — unwinding the stack without
 * calling destructors.
 *
 * Use cases: propagating errors across many stack frames,
 * simple exception-like control flow, signal handlers.
 *
 * Caveats:
 *   - local variables modified after setjmp must be volatile
 *   - any heap allocations between setjmp/longjmp are leaked
 *   - does NOT call C++ destructors — avoid in C++ code
 * ────────────────────────────────────────────────────────── */

static jmp_buf g_err_ctx;   /* save point for this example */

#define ERR_OK       0
#define ERR_OVERFLOW 1
#define ERR_UNDERFLOW 2

static void risky_divide(int a, int b) {
    if (b == 0) longjmp(g_err_ctx, ERR_OVERFLOW);
    printf("  %d / %d = %d\n", a, b, a / b);
}

static void risky_sqrt_approx(int x) {
    if (x < 0) longjmp(g_err_ctx, ERR_UNDERFLOW);
    /* integer square root by Newton's method */
    if (x == 0) { printf("  sqrt(0) = 0\n"); return; }
    int r = x;
    for (int i = 0; i < 20; i++) r = (r + x/r) / 2;
    printf("  isqrt(%d) = %d\n", x, r);
}

static void section_setjmp(void) {
    printf("=== 5. setjmp / longjmp ===\n");

    int err;
    /* setjmp returns 0 on direct call, non-zero when longjmp jumps here */
    if ((err = setjmp(g_err_ctx)) != 0) {
        const char *msg = (err == ERR_OVERFLOW)  ? "division by zero" :
                          (err == ERR_UNDERFLOW) ? "negative sqrt"    :
                          "unknown error";
        printf("  [longjmp caught] error %d: %s\n", err, msg);
        printf("\n");
        return;
    }

    risky_divide(10, 2);
    risky_sqrt_approx(144);
    risky_divide(7, 0);        /* triggers longjmp — jumps back to setjmp */
    printf("  (never reached)\n");
}

static void section_setjmp_neg(void) {
    printf("=== 5b. setjmp / longjmp (negative sqrt) ===\n");

    int err;
    if ((err = setjmp(g_err_ctx)) != 0) {
        printf("  [longjmp caught] error %d: %s\n", err,
               err == ERR_UNDERFLOW ? "negative sqrt" : "unknown");
        printf("\n");
        return;
    }

    risky_sqrt_approx(25);
    risky_sqrt_approx(-4);    /* triggers longjmp */
    printf("  (never reached)\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 6: Custom error type
 *
 * Real projects often define their own error enum so callers
 * get structured errors instead of raw errno ints.
 * ────────────────────────────────────────────────────────── */

typedef enum {
    OK = 0,
    ERR_NULL_ARG,
    ERR_PARSE,
    ERR_RANGE,
    ERR_ALLOC,
} ErrCode;

static const char *err_str(ErrCode e) {
    switch (e) {
        case OK:           return "OK";
        case ERR_NULL_ARG: return "null argument";
        case ERR_PARSE:    return "parse error";
        case ERR_RANGE:    return "out of range";
        case ERR_ALLOC:    return "allocation failed";
        default:           return "unknown error";
    }
}

/* Parse a percentage string like "75%" into an integer 0-100 */
static ErrCode parse_percent(const char *s, int *out) {
    if (s == NULL || out == NULL) return ERR_NULL_ARG;

    char *end;
    errno = 0;
    long val = strtol(s, &end, 10);
    if (errno != 0 || end == s)          return ERR_PARSE;
    if (*end != '%' || *(end+1) != '\0') return ERR_PARSE;
    if (val < 0 || val > 100)            return ERR_RANGE;

    *out = (int)val;
    return OK;
}

static void section_custom_errors(void) {
    printf("=== 6. Custom error type ===\n");

    struct { const char *s; } cases[] = {
        {"75%"}, {"0%"}, {"100%"}, {"101%"}, {"abc"}, {"50"}, {NULL}
    };

    for (int i = 0; cases[i].s != NULL; i++) {
        int v;
        ErrCode e = parse_percent(cases[i].s, &v);
        if (e == OK)
            printf("  parse_percent(\"%-5s\") = %d\n", cases[i].s, v);
        else
            printf("  parse_percent(\"%-5s\") -> %s\n", cases[i].s, err_str(e));
    }
    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    section_return_codes();
    section_errno();
    section_propagation();
    section_goto_cleanup();
    section_setjmp();
    section_setjmp_neg();
    section_custom_errors();
    return 0;
}
