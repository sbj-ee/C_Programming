/*
 * Exercise 32 — POSIX Regular Expressions
 *
 * <regex.h> provides a compiled-pattern API: regcomp() compiles a
 * pattern into a regex_t; regexec() tests a string against it and
 * returns match positions; regfree() releases memory.
 *
 * Build: make
 * Run:   ./regex_demo
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/* ── helper ──────────────────────────────────────────────────────────────── */

/* Compile a regex; abort on error */
static void compile(regex_t *re, const char *pattern, int cflags) {
    int rc = regcomp(re, pattern, cflags);
    if (rc) {
        char buf[256];
        regerror(rc, re, buf, sizeof buf);
        fprintf(stderr, "regcomp(\"%s\"): %s\n", pattern, buf);
        exit(1);
    }
}

/* Print the substring matched by pmatch[idx] within src */
static void print_match(const char *src, const regmatch_t *pmatch, int idx) {
    int start = (int)pmatch[idx].rm_so;
    int end   = (int)pmatch[idx].rm_eo;
    if (start < 0) { printf("<no match>"); return; }
    printf("\"%.*s\"", end - start, src + start);
}

/* ── 1. Concepts ─────────────────────────────────────────────────────────── */

static void section1_concepts(void) {
    puts("═══ 1. POSIX Regex Concepts ════\n");

    puts("  Two flavours:");
    puts("    BRE (Basic):    default; ( ) { } + ? | must be escaped: \\( \\)");
    puts("    ERE (Extended): REG_EXTENDED flag; (, ), +, ?, | are metacharacters\n");

    puts("  Lifecycle:");
    puts("    regcomp(re, pattern, cflags) — compile pattern into regex_t");
    puts("    regexec(re, str, nmatch, pmatch, eflags) — match against string");
    puts("    regfree(re)                  — free resources (always call)\n");

    puts("  cflags (compile flags):");
    puts("    REG_EXTENDED  ERE syntax");
    puts("    REG_ICASE     case-insensitive");
    puts("    REG_NEWLINE   ^ and $ match line boundaries; . does not match \\n\n");

    puts("  eflags (execute flags):");
    puts("    REG_NOTBOL    ^ does not match at the start of this string");
    puts("    REG_NOTEOL    $ does not match at the end of this string\n");

    puts("  Return codes:");
    puts("    0         match found");
    puts("    REG_NOMATCH  no match");
    puts("    other     error — decode with regerror()\n");
}

/* ── 2. Basic match / no-match ───────────────────────────────────────────── */

static void section2_basic(void) {
    puts("═══ 2. Basic Match / No-match ════\n");

    regex_t re;
    compile(&re, "[0-9]+\\.[0-9]+", REG_EXTENDED);

    const char *inputs[] = { "price: 3.14", "no numbers here", "v2 and 1.0.0" };
    for (int i = 0; i < 3; i++) {
        regmatch_t m;
        int rc = regexec(&re, inputs[i], 1, &m, 0);
        printf("  \"%-20s  →  ", inputs[i]);
        if (rc == 0) {
            printf("match at [%d,%d): ", (int)m.rm_so, (int)m.rm_eo);
            print_match(inputs[i], &m, 0);
        } else {
            printf("no match");
        }
        puts("");
    }
    regfree(&re);
    puts("");
}

/* ── 3. Capture groups ───────────────────────────────────────────────────── */

static void section3_groups(void) {
    puts("═══ 3. Capture Groups ════\n");

    puts("  Pattern: ^([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(.+)$");
    puts("  Matches  key = value  lines; groups 1=key, 2=value\n");

    regex_t re;
    compile(&re, "^([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*(.+)$", REG_EXTENDED);

    const char *lines[] = {
        "host = localhost",
        "port = 8080",
        "debug=true",
        "# comment line",
        "bad line",
    };

    for (int i = 0; i < 5; i++) {
        regmatch_t pm[3];   /* pm[0]=whole, pm[1]=key, pm[2]=value */
        int rc = regexec(&re, lines[i], 3, pm, 0);
        printf("  %-22s  →  ", lines[i]);
        if (rc == 0) {
            printf("key="); print_match(lines[i], pm, 1);
            printf("  val="); print_match(lines[i], pm, 2);
        } else {
            printf("no match");
        }
        puts("");
    }
    regfree(&re);
    puts("");
}

/* ── 4. Flags: REG_ICASE, REG_NEWLINE ───────────────────────────────────── */

static void section4_flags(void) {
    puts("═══ 4. Flags: REG_ICASE and REG_NEWLINE ════\n");

    /* REG_ICASE */
    regex_t re;
    compile(&re, "hello", REG_EXTENDED | REG_ICASE);
    const char *tests[] = { "Hello World", "HELLO!", "goodbye" };
    for (int i = 0; i < 3; i++) {
        int rc = regexec(&re, tests[i], 0, NULL, 0);
        printf("  REG_ICASE  \"hello\" vs \"%s\": %s\n",
               tests[i], rc == 0 ? "match" : "no match");
    }
    regfree(&re);
    puts("");

    /* REG_NEWLINE: ^ and $ match line ends */
    compile(&re, "^[0-9]+$", REG_EXTENDED | REG_NEWLINE);
    const char *ml = "abc\n42\nxyz";
    regmatch_t m;
    const char *p = ml;
    printf("  REG_NEWLINE  ^[0-9]+$  in %s:\n", "\"abc\\n42\\nxyz\"");
    while (regexec(&re, p, 1, &m, p == ml ? 0 : REG_NOTBOL) == 0) {
        printf("    matched: \"%.*s\"\n", (int)(m.rm_eo - m.rm_so), p + m.rm_so);
        p += m.rm_eo;
        if (*p == '\0') break;
    }
    regfree(&re);
    puts("");
}

/* ── 5. Scan loop: all matches in a string ───────────────────────────────── */

static void section5_scan(void) {
    puts("═══ 5. Scan Loop: All Matches ════\n");

    regex_t re;
    compile(&re, "[A-Z][a-z]+", REG_EXTENDED);

    const char *text = "The Quick Brown Fox jumped over the Lazy Dog";
    printf("  text: \"%s\"\n  capitalised words: ", text);

    const char *cursor = text;
    int eflags = 0;
    int first = 1;
    regmatch_t m;

    while (regexec(&re, cursor, 1, &m, eflags) == 0) {
        if (!first) printf(", ");
        printf("\"%.*s\"", (int)(m.rm_eo - m.rm_so), cursor + m.rm_so);
        first = 0;
        cursor += m.rm_eo;
        eflags  = REG_NOTBOL;   /* ^ no longer matches mid-string */
        if (m.rm_eo == m.rm_so) { cursor++; }   /* avoid infinite loop on ε-match */
    }
    puts("\n");
    regfree(&re);

    puts("  Scan loop pattern:");
    puts("    cursor = start of text");
    puts("    while regexec(re, cursor, 1, &m, eflags) == 0:");
    puts("      process cursor[m.rm_so .. m.rm_eo)");
    puts("      cursor += m.rm_eo;  eflags |= REG_NOTBOL;\n");
}

/* ── 6. Common patterns ──────────────────────────────────────────────────── */

static void check(const char *label, regex_t *re, const char *input) {
    regmatch_t m;
    int rc = regexec(re, input, 1, &m, 0);
    /* Anchored pattern: full match only if rm_so==0 and rm_eo==strlen */
    int full = (rc == 0 && m.rm_so == 0 && m.rm_eo == (regoff_t)strlen(input));
    printf("  %-12s  %-28s  %s\n", label, input, full ? "valid" : "INVALID");
}

static void section6_patterns(void) {
    puts("═══ 6. Common Patterns ════\n");

    /* IPv4 address */
    regex_t ipv4;
    compile(&ipv4,
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$",
        REG_EXTENDED);
    check("IPv4", &ipv4, "192.168.1.1");
    check("IPv4", &ipv4, "10.0.0.255");
    check("IPv4", &ipv4, "999.1.1.1");
    check("IPv4", &ipv4, "1.2.3");
    regfree(&ipv4);
    puts("");

    /* ISO date YYYY-MM-DD */
    regex_t date;
    compile(&date, "^[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$",
            REG_EXTENDED);
    check("ISO date", &date, "2026-06-06");
    check("ISO date", &date, "1999-12-31");
    check("ISO date", &date, "2026-13-01");
    check("ISO date", &date, "26-1-1");
    regfree(&date);
    puts("");

    /* C identifier */
    regex_t ident;
    compile(&ident, "^[A-Za-z_][A-Za-z0-9_]*$", REG_EXTENDED);
    check("C ident", &ident, "my_var");
    check("C ident", &ident, "_internal");
    check("C ident", &ident, "2bad");
    check("C ident", &ident, "has space");
    regfree(&ident);
    puts("");

    puts("  Notes:");
    puts("  - Anchor with ^ and $ to validate the whole string");
    puts("  - POSIX ERE has no \\d \\w \\s — use [0-9] [A-Za-z0-9_] [ \\t]");
    puts("  - For full Unicode/PCRE features, use the PCRE2 library (pcre2.h)\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    section1_concepts();
    section2_basic();
    section3_groups();
    section4_flags();
    section5_scan();
    section6_patterns();
    return 0;
}
