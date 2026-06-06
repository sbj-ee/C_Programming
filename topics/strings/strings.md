# Strings

C strings are null-terminated arrays of `char`. There is no string type —
only a convention that a `'\0'` byte marks the end.

## Representation

```c
char s1[] = "hello";       // array: {'h','e','l','l','o','\0'}, size 6
char *s2  = "hello";       // pointer to read-only literal — do not modify
const char *s3 = "hello";  // correct type for string literals

// String literal in memory:
// [ h | e | l | l | o | \0 ]
//   0   1   2   3   4   5     indices
```

## Key functions (`<string.h>`)

### Length and copy

```c
size_t strlen(const char *s);           // length excluding '\0'
char  *strcpy(char *dst, const char *src);   // copy src into dst
char  *strncpy(char *dst, const char *src, size_t n);  // copy at most n bytes
// strncpy does NOT guarantee null-termination if src is longer than n:
strncpy(dst, src, sizeof dst - 1);
dst[sizeof dst - 1] = '\0';            // always terminate manually
```

### Concatenation and comparison

```c
char  *strcat(char *dst, const char *src);         // append src to dst
char  *strncat(char *dst, const char *src, size_t n); // append at most n chars
int    strcmp(const char *a, const char *b);       // 0 if equal
int    strncmp(const char *a, const char *b, size_t n); // first n chars
// Return: <0 if a<b, 0 if equal, >0 if a>b
```

### Search

```c
char *strchr(const char *s, int c);      // first occurrence of c
char *strrchr(const char *s, int c);     // last occurrence of c
char *strstr(const char *haystack, const char *needle);
size_t strspn(const char *s, const char *accept);   // leading accepted chars
size_t strcspn(const char *s, const char *reject);  // leading non-rejected chars
```

### Tokenising

```c
// strtok modifies the string in place — copy first if you need the original
char src[] = "one,two,three";
char *tok = strtok(src, ",");
while (tok) {
    printf("%s\n", tok);
    tok = strtok(NULL, ",");   // NULL continues from last position
}
// Not re-entrant: use strtok_r (POSIX) in multi-threaded code
```

### Formatted I/O

```c
// Build a string into a buffer
char buf[64];
snprintf(buf, sizeof buf, "%s=%d", key, value);  // always use snprintf, not sprintf

// Parse a string
int n; float f;
sscanf("42 3.14", "%d %f", &n, &f);

// Safe alternative to atoi (handles errors):
char *end;
long val = strtol("123abc", &end, 10);  // end points to "abc"
if (*end != '\0') { /* parse error */ }
```

## Safe string copy (no strncpy confusion)

```c
// Idiom: copy at most sizeof(dst)-1 chars, always null-terminate
void safe_copy(char *dst, const char *src, size_t dstsize) {
    size_t n = dstsize - 1;
    strncpy(dst, src, n);
    dst[n] = '\0';
}

// Even cleaner with memcpy (when you know src length):
size_t srclen = strlen(src);
size_t cpylen = srclen < dstsize - 1 ? srclen : dstsize - 1;
memcpy(dst, src, cpylen);
dst[cpylen] = '\0';
```

## Heap-allocated strings

```c
// Duplicate a string (strdup is POSIX; implement in strict C11):
char *dup_str(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;   // caller must free
}

// Build a string of unknown final length:
char   *buf  = NULL;
size_t  cap  = 0;
size_t  len  = 0;

void append(const char *s) {
    size_t add = strlen(s);
    if (len + add + 1 > cap) {
        cap = (len + add + 1) * 2;
        buf = realloc(buf, cap);
    }
    memcpy(buf + len, s, add + 1);
    len += add;
}
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| `sprintf` with no size limit | Use `snprintf(buf, sizeof buf, ...)` |
| `strcpy` into undersized buffer | Check `strlen(src) < sizeof(dst)` first |
| Modifying a string literal (`char *s = "hi"; s[0]='x'`) | Use `char s[] = "hi"` |
| `strncpy` missing null terminator | Always set `dst[n-1] = '\0'` explicitly |
| Comparing strings with `==` | Use `strcmp(a, b) == 0` |
| `strtol` with no error check | Check `errno` and `*end` after the call |
