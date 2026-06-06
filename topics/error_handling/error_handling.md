# Error Handling

C has no exceptions. Errors are communicated through return values,
output parameters, and the global `errno` variable.

## Return-code convention

```c
// Convention: return 0 on success, -1 (or negative errno) on failure
int connect_db(const char *url) {
    if (!url) { errno = EINVAL; return -1; }
    // ...
    return 0;
}

// Convention: return valid pointer or NULL on failure
char *load_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;        // errno set by fopen
    // ...
}
```

## errno, perror, strerror

```c
#include <errno.h>
#include <string.h>   // strerror
#include <stdio.h>    // perror

// errno is set by failing system/library calls
// Always check immediately — the next call may overwrite it
FILE *f = fopen("missing.txt", "r");
if (!f) {
    int saved = errno;                  // save before any other call
    fprintf(stderr, "fopen: %s\n", strerror(saved));
    // or just:
    perror("fopen");                    // prints "fopen: No such file or directory"
}

// Common values
EPERM    // 1  — operation not permitted
ENOENT   // 2  — no such file or directory
EINTR    // 4  — interrupted by signal
EIO      // 5  — I/O error
EACCES   // 13 — permission denied
EINVAL   // 22 — invalid argument
ENOMEM   // 12 — out of memory
ERANGE   // 34 — result out of range
EEXIST   // 17 — file exists
EAGAIN   // 11 — try again (resource temporarily unavailable)
```

## Error propagation

Each function checks its callees and returns early on failure,
preserving the errno set by the deepest failing call.

```c
static int read_record(int fd, Record *r) {
    ssize_t n = read(fd, r, sizeof *r);
    if (n != (ssize_t)sizeof *r) return -1;  // errno set by read()
    return 0;
}

static int load_records(const char *path, Record *recs, size_t n) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;                  // errno from open()

    for (size_t i = 0; i < n; i++) {
        if (read_record(fd, &recs[i]) != 0) {
            close(fd);
            return -1;                      // errno from read_record
        }
    }
    close(fd);
    return 0;
}
```

## goto cleanup pattern

The only clean way to handle multiple resources with early exits.

```c
int process(const char *path) {
    int    rc  = -1;
    FILE  *f   = NULL;
    char  *buf = NULL;
    int   *arr = NULL;

    f = fopen(path, "r");
    if (!f) goto done;

    buf = malloc(4096);
    if (!buf) { errno = ENOMEM; goto done; }

    arr = calloc(256, sizeof *arr);
    if (!arr) { errno = ENOMEM; goto done; }

    // ... do work ...
    rc = 0;

done:
    free(arr);          // free(NULL) is safe
    free(buf);
    if (f) fclose(f);
    return rc;
}
```

## Custom error type

Richer than raw errno — typed, extensible, no global state.

```c
typedef enum {
    ERR_OK      = 0,
    ERR_NULL    = 1,
    ERR_PARSE   = 2,
    ERR_RANGE   = 3,
    ERR_ALLOC   = 4,
} ErrCode;

static const char *err_str(ErrCode e) {
    switch (e) {
        case ERR_OK:    return "ok";
        case ERR_NULL:  return "null argument";
        case ERR_PARSE: return "parse error";
        case ERR_RANGE: return "out of range";
        case ERR_ALLOC: return "allocation failed";
        default:        return "unknown error";
    }
}

// Usage
ErrCode parse_port(const char *s, uint16_t *out) {
    if (!s || !out) return ERR_NULL;
    char *end;
    long v = strtol(s, &end, 10);
    if (*end) return ERR_PARSE;
    if (v < 1 || v > 65535) return ERR_RANGE;
    *out = (uint16_t)v;
    return ERR_OK;
}
```

## setjmp / longjmp

Non-local jump: unwind multiple stack frames without calling destructors.
Use sparingly — skips cleanup code between the frames.

```c
#include <setjmp.h>

static jmp_buf err_env;

void deep_fn(void) {
    // On fatal error:
    longjmp(err_env, 1);   // jumps back to setjmp, returns 1
}

int main(void) {
    if (setjmp(err_env) != 0) {
        fprintf(stderr, "fatal error caught\n");
        return 1;
    }
    deep_fn();   // may longjmp back here
    return 0;
}
```

**Pitfalls with setjmp:**
- Local variables modified after `setjmp` must be `volatile` to be reliable.
- Heap allocations between `setjmp` and `longjmp` are leaked.
- Does not call C++ destructors — avoid mixing with C++ code.

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Not checking return values | Check every call that can fail |
| Overwriting errno before reading it | Save to a local immediately |
| Returning error code from a function that also needs a pointer | Use an out-parameter: `int fn(T **out)` |
| Leaking resources on early return | Use goto cleanup or RAII-style wrappers |
| Using `exit()` in library code | Return an error code instead |
