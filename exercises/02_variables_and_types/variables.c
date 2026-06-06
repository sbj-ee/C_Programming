#include <stdio.h>

int main(void) {

    /* --- Integer types --- */
    char   c  = 'A';          /* 1 byte, -128..127 */
    short  s  = 32000;        /* at least 2 bytes */
    int    i  = -2147483648;  /* at least 4 bytes, most common general-purpose type */
    long   l  = 9876543210L;  /* at least 4 bytes (8 on most 64-bit systems) */

    printf("=== Integer types ===\n");
    printf("char:   %c  (%d)\n", c, c);
    printf("short:  %d\n", s);
    printf("int:    %d\n", i);
    printf("long:   %ld\n", l);

    /* --- Unsigned integers (no negative values, double the positive range) --- */
    unsigned int  ui = 4294967295U;
    unsigned char uc = 255;

    printf("\n=== Unsigned integers ===\n");
    printf("unsigned int:  %u\n", ui);
    printf("unsigned char: %u\n", uc);

    /* --- Floating point types --- */
    float  f  = 3.14f;        /* ~6-7 significant digits */
    double d  = 3.141592653589793; /* ~15-16 significant digits */

    printf("\n=== Floating point ===\n");
    printf("float:  %.7f\n", f);
    printf("double: %.15f\n", d);

    /* --- sizeof: bytes occupied by each type --- */
    printf("\n=== sizeof (bytes) ===\n");
    printf("char:   %zu\n", sizeof(char));
    printf("short:  %zu\n", sizeof(short));
    printf("int:    %zu\n", sizeof(int));
    printf("long:   %zu\n", sizeof(long));
    printf("float:  %zu\n", sizeof(float));
    printf("double: %zu\n", sizeof(double));

    /* --- Type casting --- */

    /* Implicit: int divided by int truncates — no remainder */
    int   a = 7, b = 2;
    int   truncated = a / b;
    printf("\n=== Type casting ===\n");
    printf("7 / 2 (int / int)      = %d  (truncates)\n", truncated);

    /* Explicit cast: promote one operand to double before dividing */
    double exact = (double)a / b;
    printf("(double)7 / 2          = %.1f\n", exact);

    /* Casting double to int truncates toward zero */
    double pi = 3.99;
    int    pi_int = (int)pi;
    printf("(int)3.99              = %d  (truncates, does not round)\n", pi_int);

    /* char arithmetic: chars are just small integers */
    char letter = 'a';
    printf("\n'a' + 1 = '%c' (%d)\n", letter + 1, letter + 1);

    return 0;
}
