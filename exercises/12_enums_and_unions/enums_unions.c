#include <stdio.h>
#include <string.h>
#include <math.h>

#define M_PI 3.14159265358979323846

/* ================================================================
   ENUMS
   ================================================================ */

/* enum assigns sequential integer constants starting at 0 */
typedef enum {
    MON = 0, TUE, WED, THU, FRI, SAT, SUN
} Weekday;

/* Values can be set explicitly; subsequent values continue from there */
typedef enum {
    HTTP_OK       = 200,
    HTTP_CREATED  = 201,
    HTTP_NOT_FOUND = 404,
    HTTP_SERVER_ERROR = 500
} HttpStatus;

/* Bit-flag enum: each value is a distinct power of two so they can be OR'd */
typedef enum {
    PERM_NONE    = 0,
    PERM_READ    = 1 << 0,   /* 1 */
    PERM_WRITE   = 1 << 1,   /* 2 */
    PERM_EXECUTE = 1 << 2    /* 4 */
} Permission;

const char *weekday_name(Weekday d) {
    const char *names[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    return (d >= MON && d <= SUN) ? names[d] : "?";
}

const char *http_label(HttpStatus s) {
    switch (s) {
        case HTTP_OK:           return "OK";
        case HTTP_CREATED:      return "Created";
        case HTTP_NOT_FOUND:    return "Not Found";
        case HTTP_SERVER_ERROR: return "Internal Server Error";
        default:                return "Unknown";
    }
}

void print_permissions(Permission perms) {
    printf("r%c w%c x%c\n",
        (perms & PERM_READ)    ? '+' : '-',
        (perms & PERM_WRITE)   ? '+' : '-',
        (perms & PERM_EXECUTE) ? '+' : '-');
}

/* ================================================================
   UNIONS
   ================================================================ */

/* A union allocates one shared block of memory large enough for its
   largest member. Only one member holds a valid value at any time. */
typedef union {
    int    i;
    float  f;
    char   bytes[4];
} MultiView;

/* ================================================================
   TAGGED UNION — the canonical C pattern for variant types.
   A struct pairs a union with a tag (enum) that records which
   member is currently active. */
typedef enum { NUM, STR, BOOL } ValueKind;

typedef struct {
    ValueKind kind;
    union {
        double      number;
        char        string[32];
        int         boolean;
    } as;
} Value;

Value make_num (double d)      { return (Value){ .kind = NUM,  .as.number  = d }; }
Value make_str (const char *s) {
    Value v = { .kind = STR };
    strncpy(v.as.string, s, sizeof(v.as.string) - 1);
    return v;
}
Value make_bool(int b)         { return (Value){ .kind = BOOL, .as.boolean = b }; }

void print_value(Value v) {
    switch (v.kind) {
        case NUM:  printf("number: %g\n",        v.as.number);  break;
        case STR:  printf("string: \"%s\"\n",    v.as.string);  break;
        case BOOL: printf("bool:   %s\n",   v.as.boolean ? "true" : "false"); break;
    }
}

/* ================================================================
   GEOMETRIC SHAPE — a realistic tagged union example
   ================================================================ */
typedef enum { CIRCLE, RECT, TRIANGLE } ShapeKind;

typedef struct {
    ShapeKind kind;
    union {
        struct { double radius; }                         circle;
        struct { double width, height; }                  rect;
        struct { double a, b, c; }                        triangle; /* side lengths */
    };
} Shape;

double shape_area(Shape s) {
    switch (s.kind) {
        case CIRCLE:
            return M_PI * s.circle.radius * s.circle.radius;
        case RECT:
            return s.rect.width * s.rect.height;
        case TRIANGLE: {
            /* Heron's formula */
            double p = (s.triangle.a + s.triangle.b + s.triangle.c) / 2.0;
            return sqrt(p * (p - s.triangle.a)
                          * (p - s.triangle.b)
                          * (p - s.triangle.c));
        }
    }
    return 0;
}

const char *shape_name(ShapeKind k) {
    switch (k) {
        case CIRCLE:   return "circle";
        case RECT:     return "rect";
        case TRIANGLE: return "triangle";
    }
    return "?";
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void) {

    /* --- Basic enum --- */
    printf("=== enum: Weekday ===\n");
    Weekday today = WED;
    printf("today = %s (%d)\n", weekday_name(today), today);

    for (Weekday d = MON; d <= SUN; d++) {
        printf("  %d = %s%s\n", d, weekday_name(d),
               (d >= SAT) ? " (weekend)" : "");
    }

    /* --- Explicit values --- */
    printf("\n=== enum: HttpStatus ===\n");
    HttpStatus codes[] = {HTTP_OK, HTTP_CREATED, HTTP_NOT_FOUND, HTTP_SERVER_ERROR};
    for (int i = 0; i < 4; i++) {
        printf("  %d %s\n", codes[i], http_label(codes[i]));
    }

    /* --- Bit-flag enum --- */
    printf("\n=== enum: bit flags ===\n");
    Permission owner = PERM_READ | PERM_WRITE | PERM_EXECUTE;
    Permission group = PERM_READ | PERM_EXECUTE;
    Permission other = PERM_READ;

    printf("owner: "); print_permissions(owner);
    printf("group: "); print_permissions(group);
    printf("other: "); print_permissions(other);

    /* Test and modify individual bits */
    group |=  PERM_WRITE;    /* grant write */
    other &= ~PERM_READ;     /* revoke read */
    printf("group after grant write: "); print_permissions(group);
    printf("other after revoke read: "); print_permissions(other);

    /* --- Union: shared memory --- */
    printf("\n=== union: shared memory ===\n");
    MultiView u;
    u.i = 0x41424344;   /* store as int */
    printf("as int:   0x%08X\n", u.i);
    printf("as bytes: %c %c %c %c\n",
           u.bytes[3], u.bytes[2], u.bytes[1], u.bytes[0]);
    printf("sizeof(MultiView) = %zu  (size of largest member)\n", sizeof(u));

    u.f = 1.0f;         /* now float is valid; int reading is undefined */
    printf("as float: %f  (bits: 0x%08X)\n", u.f, u.i);

    /* --- Tagged union: Value --- */
    printf("\n=== tagged union: Value ===\n");
    Value vals[] = {
        make_num(3.14),
        make_str("hello"),
        make_bool(1),
        make_num(-0.5),
        make_bool(0),
    };
    int n = sizeof(vals) / sizeof(vals[0]);
    for (int i = 0; i < n; i++) print_value(vals[i]);

    /* --- Tagged union: Shape --- */
    printf("\n=== tagged union: Shape ===\n");
    Shape shapes[] = {
        { .kind = CIRCLE,   .circle   = { .radius = 5.0 }         },
        { .kind = RECT,     .rect     = { .width  = 4.0, .height = 6.0 } },
        { .kind = TRIANGLE, .triangle = { .a = 3.0, .b = 4.0, .c = 5.0 } },
    };
    int ns = sizeof(shapes) / sizeof(shapes[0]);
    for (int i = 0; i < ns; i++) {
        printf("%-10s area = %.4f\n", shape_name(shapes[i].kind),
               shape_area(shapes[i]));
    }

    return 0;
}
