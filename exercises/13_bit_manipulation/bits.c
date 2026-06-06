#include <stdio.h>
#include <stdint.h>   /* uint8_t, uint16_t, uint32_t — exact-width types */

/* Print a byte as 8 binary digits */
static void print_bits8(uint8_t v) {
    for (int i = 7; i >= 0; i--)
        putchar((v >> i) & 1 ? '1' : '0');
}

static void print_bits32(uint32_t v) {
    for (int i = 31; i >= 0; i--) {
        putchar((v >> i) & 1 ? '1' : '0');
        if (i % 8 == 0 && i > 0) putchar('_');
    }
}

/* ================================================================
   BIT OPERATIONS
   Operator  Name         Effect
   &         AND          1 only where both bits are 1
   |         OR           1 where either bit is 1
   ^         XOR          1 where bits differ
   ~         NOT          flip every bit
   <<        left shift   multiply by 2^n
   >>        right shift  divide by 2^n (unsigned)
   ================================================================ */

/* ================================================================
   COMMON IDIOMS
   ================================================================ */

/* Set bit n:   x |=  (1 << n)  */
static uint8_t bit_set(uint8_t x, int n)    { return x |  (1u << n); }

/* Clear bit n: x &= ~(1 << n)  */
static uint8_t bit_clear(uint8_t x, int n)  { return x & ~(1u << n); }

/* Toggle bit n: x ^= (1 << n)  */
static uint8_t bit_toggle(uint8_t x, int n) { return x ^  (1u << n); }

/* Test bit n:  (x >> n) & 1    */
static int     bit_test(uint8_t x, int n)   { return (x >> n) & 1; }

/* Isolate the lowest set bit */
static uint32_t lowest_set_bit(uint32_t x)  { return x & (-x); }

/* Count set bits (popcount) — Brian Kernighan's algorithm */
static int popcount(uint32_t x) {
    int count = 0;
    while (x) { x &= x - 1; count++; }   /* each iteration clears the lowest set bit */
    return count;
}

/* Reverse the bits of a byte */
static uint8_t reverse_bits(uint8_t x) {
    uint8_t r = 0;
    for (int i = 0; i < 8; i++) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

/* Check if x is a power of two */
static int is_power_of_two(uint32_t x) { return x != 0 && (x & (x - 1)) == 0; }

/* Round x up to the next power of two */
static uint32_t next_power_of_two(uint32_t x) {
    if (x == 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

/* ================================================================
   STRUCT BITFIELDS
   Bitfields pack multiple values into one integer.
   The compiler controls the exact layout, so they are not portable
   across compilers/platforms for serialisation — but are useful for
   in-memory flags and hardware register models.
   ================================================================ */
typedef struct {
    uint8_t red   : 5;   /* 5 bits: 0-31 */
    uint8_t green : 6;   /* 6 bits: 0-63 */
    uint8_t blue  : 5;   /* 5 bits: 0-31 */
} RGB565;                /* 16-bit colour, packed into two bytes */

typedef struct {
    unsigned int is_directory : 1;
    unsigned int is_readable  : 1;
    unsigned int is_writable  : 1;
    unsigned int is_hidden    : 1;
    unsigned int reserved     : 4;
} FileAttribs;

/* ================================================================
   PACKING / UNPACKING: manually encoding multiple fields into one
   integer (portable, explicit, useful in protocols and file formats)
   ================================================================ */

/* Pack r(5), g(6), b(5) into a uint16_t */
static uint16_t rgb565_pack(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint16_t)(r & 0x1F) << 11)
         | ((uint16_t)(g & 0x3F) <<  5)
         |  (uint16_t)(b & 0x1F);
}

static uint8_t rgb565_r(uint16_t c) { return (c >> 11) & 0x1F; }
static uint8_t rgb565_g(uint16_t c) { return (c >>  5) & 0x3F; }
static uint8_t rgb565_b(uint16_t c) { return  c        & 0x1F; }

/* ================================================================
   MAIN
   ================================================================ */
int main(void) {

    /* --- Operators --- */
    printf("=== Bitwise operators ===\n");
    uint8_t a = 0xB6;   /* 10110110 = 182 */
    uint8_t b = 0x6D;   /* 01101101 = 109 */

    printf("a   = "); print_bits8(a); printf("  (%3u)\n", a);
    printf("b   = "); print_bits8(b); printf("  (%3u)\n", b);
    printf("a&b = "); print_bits8(a & b); printf("  (%3u)  AND\n",  a & b);
    printf("a|b = "); print_bits8(a | b); printf("  (%3u)  OR\n",   a | b);
    printf("a^b = "); print_bits8(a ^ b); printf("  (%3u)  XOR\n",  a ^ b);
    printf("~a  = "); print_bits8(~a);    printf("  (%3u)  NOT\n",  (uint8_t)~a);

    /* --- Shifts --- */
    printf("\n=== Shifts ===\n");
    uint8_t x = 0x01;   /* 00000001 */
    printf("x      = "); print_bits8(x); printf("  (%u)\n", x);
    printf("x << 3 = "); print_bits8(x << 3); printf("  (%u)  *8\n", x << 3);
    printf("x << 7 = "); print_bits8(x << 7); printf("  (%u)  *128\n", x << 7);
    uint8_t y = 0x80;   /* 10000000 */
    printf("y      = "); print_bits8(y); printf("  (%u)\n", y);
    printf("y >> 1 = "); print_bits8(y >> 1); printf("  (%u)  /2\n", y >> 1);
    printf("y >> 4 = "); print_bits8(y >> 4); printf("  (%u)  /16\n", y >> 4);

    /* --- Set / clear / toggle / test --- */
    printf("\n=== Set, clear, toggle, test ===\n");
    uint8_t flags = 0x00;
    printf("initial:  "); print_bits8(flags); printf("\n");
    flags = bit_set(flags, 0); printf("set   0:  "); print_bits8(flags); printf("\n");
    flags = bit_set(flags, 3); printf("set   3:  "); print_bits8(flags); printf("\n");
    flags = bit_set(flags, 7); printf("set   7:  "); print_bits8(flags); printf("\n");
    flags = bit_clear(flags, 3); printf("clear 3:  "); print_bits8(flags); printf("\n");
    flags = bit_toggle(flags, 0); printf("toggle 0: "); print_bits8(flags); printf("\n");
    printf("test  7:  %d\n", bit_test(flags, 7));
    printf("test  0:  %d\n", bit_test(flags, 0));

    /* --- Useful tricks --- */
    printf("\n=== Useful tricks ===\n");
    uint32_t v = 0xB4;   /* 10110100 */
    printf("v                = "); print_bits32(v); printf("  (%u)\n", v);
    printf("lowest set bit   = "); print_bits32(lowest_set_bit(v)); printf("\n");
    printf("popcount         = %d\n", popcount(v));
    printf("reverse_bits(0xA2) = 0x%02X\n", reverse_bits(0xA2));

    printf("\nis_power_of_two:\n");
    uint32_t tests[] = {0, 1, 2, 3, 4, 15, 16, 255, 256};
    for (int i = 0; i < 9; i++) {
        printf("  %4u -> %s\n", tests[i], is_power_of_two(tests[i]) ? "yes" : "no");
    }

    printf("\nnext_power_of_two:\n");
    uint32_t vals[] = {0, 1, 5, 9, 17, 100, 128, 1000};
    for (int i = 0; i < 8; i++) {
        printf("  %4u -> %u\n", vals[i], next_power_of_two(vals[i]));
    }

    /* --- Struct bitfields --- */
    printf("\n=== Struct bitfields ===\n");
    RGB565 colour = { .red = 31, .green = 63, .blue = 31 };   /* white */
    printf("RGB565 white: r=%u g=%u b=%u  sizeof=%zu\n",
           colour.red, colour.green, colour.blue, sizeof(colour));

    FileAttribs fa = { .is_directory = 0, .is_readable = 1,
                       .is_writable  = 1, .is_hidden   = 0 };
    printf("FileAttribs: dir=%u read=%u write=%u hidden=%u  sizeof=%zu\n",
           fa.is_directory, fa.is_readable, fa.is_writable, fa.is_hidden,
           sizeof(fa));

    /* --- Manual packing / unpacking --- */
    printf("\n=== Manual packing (RGB565) ===\n");
    uint16_t packed = rgb565_pack(31, 20, 10);
    printf("packed   = 0x%04X\n", packed);
    printf("bits     = "); print_bits32(packed); printf("\n");
    printf("r=%u g=%u b=%u\n", rgb565_r(packed), rgb565_g(packed), rgb565_b(packed));

    /* XOR swap — swaps two integers without a temporary variable */
    printf("\n=== XOR swap ===\n");
    int p = 42, q = 99;
    printf("before: p=%d q=%d\n", p, q);
    p ^= q;
    q ^= p;
    p ^= q;
    printf("after:  p=%d q=%d\n", p, q);

    return 0;
}
