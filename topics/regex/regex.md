# POSIX Regular Expressions

Requires `#define _POSIX_C_SOURCE 200809L` and `<regex.h>`.
No extra link flags needed (part of libc).

## Lifecycle

```c
#include <regex.h>

regex_t re;

/* 1. Compile */
int rc = regcomp(&re, pattern, cflags);
if (rc) {
    char buf[256];
    regerror(rc, &re, buf, sizeof buf);
    fprintf(stderr, "regcomp: %s\n", buf);
}

/* 2. Match */
regmatch_t pmatch[10];   /* [0] = whole match; [1..] = capture groups */
rc = regexec(&re, string, 10, pmatch, eflags);
/* rc == 0: match; rc == REG_NOMATCH: no match; other: error */

/* 3. Free */
regfree(&re);            /* call only after successful regcomp; content is undefined on failure */
```

## Compile flags (cflags)

| Flag | Meaning |
|------|---------|
| `REG_EXTENDED` | Extended RE (ERE) — `+` `?` `|` `()` are metacharacters |
| `REG_ICASE` | Case-insensitive matching |
| `REG_NEWLINE` | `^`/`$` match line boundaries; `.` does not match `\n` |
| `REG_NOSUB` | Don't track subexpressions (faster; pmatch ignored) |

Without `REG_EXTENDED` the engine uses BRE (Basic RE): `\(`, `\)`, `\+`, `\?`, `\|` required.

## Execute flags (eflags)

| Flag | Meaning |
|------|---------|
| `REG_NOTBOL` | `^` does not match at the start of this string |
| `REG_NOTEOL` | `$` does not match at the end of this string |

Pass `REG_NOTBOL` on all iterations after the first in a scan loop.

## Reading match positions

```c
if (rc == 0) {
    /* pmatch[0].rm_so = start offset (inclusive)
     * pmatch[0].rm_eo = end   offset (exclusive)
     * pmatch[n].rm_so == -1 if group n did not participate */
    int start = (int)pmatch[0].rm_so;
    int end   = (int)pmatch[0].rm_eo;
    printf("match: \"%.*s\"\n", end - start, string + start);
}
```

## Capture groups

```c
/* Pattern with 2 capture groups */
regcomp(&re, "^([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*(.+)$", REG_EXTENDED);

regmatch_t pm[3];   /* [0]=whole, [1]=key, [2]=value */
if (regexec(&re, line, 3, pm, 0) == 0) {
    printf("key:   %.*s\n", (int)(pm[1].rm_eo - pm[1].rm_so), line + pm[1].rm_so);
    printf("value: %.*s\n", (int)(pm[2].rm_eo - pm[2].rm_so), line + pm[2].rm_so);
}
```

`nmatch` must be at least `n_groups + 1` (for `pmatch[0]` = whole match).

## Scan loop — all matches in a string

```c
const char *cursor = text;
int eflags = 0;
regmatch_t m;

while (regexec(&re, cursor, 1, &m, eflags) == 0) {
    printf("found: \"%.*s\"\n",
           (int)(m.rm_eo - m.rm_so), cursor + m.rm_so);

    cursor += m.rm_eo;
    eflags  = REG_NOTBOL;   /* ^ no longer matches mid-string */

    if (m.rm_eo == m.rm_so) cursor++;  /* prevent infinite loop on ε-match */
}
```

## ERE syntax reference

| Syntax | Meaning |
|--------|---------|
| `.` | Any character except `\n` (unless `REG_NEWLINE`) |
| `*` | Zero or more of the preceding |
| `+` | One or more |
| `?` | Zero or one |
| `{n,m}` | Between n and m repetitions |
| `^` | Start of string (or line with `REG_NEWLINE`) |
| `$` | End of string (or line with `REG_NEWLINE`) |
| `[abc]` | Character class |
| `[^abc]` | Negated character class |
| `[a-z]` | Range |
| `(abc)` | Capture group |
| `a|b` | Alternation (`a\|b` in BRE (Basic RE) — backslash required) |
| `\` | Escape next metacharacter |

**POSIX ERE has no `\d`, `\w`, `\s`** — use `[0-9]`, `[A-Za-z0-9_]`, `[ \t]`.

## Common patterns

```c
/* IPv4 address */
"^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
"(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"

/* ISO date YYYY-MM-DD */
"^[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$"

/* C identifier */
"^[A-Za-z_][A-Za-z0-9_]*$"

/* Floating-point number */
"^-?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$"

/* Hex colour #rrggbb */
"^#[0-9A-Fa-f]{6}$"

/* Non-empty whitespace-delimited token */
"[^ \t\n]+"
```

Always anchor validation patterns with `^` and `$` to match the whole input.

## Helper: check if string matches whole pattern

```c
int matches_all(const char *pattern, const char *str) {
    regex_t re;
    regmatch_t m;
    if (regcomp(&re, pattern, REG_EXTENDED)) return 0;
    int ok = (regexec(&re, str, 1, &m, 0) == 0
              && m.rm_so == 0
              && m.rm_eo == (regoff_t)strlen(str));
    regfree(&re);
    return ok;
}
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Forgetting `regfree` | Always call, even after `regcomp` failure returns partial state |
| Using `\d`, `\w`, `\s` in POSIX ERE | Use `[0-9]`, `[A-Za-z0-9_]`, `[ \t\r\n]` |
| Pattern validates substring, not whole string | Anchor with `^` and `$` |
| Not passing `REG_NOTBOL` after first iteration | `^` incorrectly matches mid-string positions |
| Infinite loop on zero-length match | Advance `cursor++` when `rm_eo == rm_so` |
| Recompiling the same pattern in a loop | Compile once outside the loop; `regfree` after |
| Forgetting to check `pmatch[n].rm_so != -1` | Group may not have participated in the match |

## POSIX vs PCRE2

POSIX `<regex.h>` is always available but limited. For richer features:

```c
/* PCRE2 (install libpcre2-dev; link -lpcre2-8) */
#include <pcre2.h>
/* Supports: \d \w \s \b, lookahead/behind, named groups, Unicode */
```
