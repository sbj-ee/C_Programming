# Enums

An `enum` defines a named integer type whose values are symbolic constants.
The compiler assigns sequential integers starting at 0 unless you say otherwise.

## Basic syntax

```c
typedef enum {
    MON = 0, TUE, WED, THU, FRI, SAT, SUN
} Weekday;

Weekday today = WED;    /* WED == 2 */
```

Without `typedef`, every use requires the `enum` keyword:
```c
enum Weekday today = WED;   /* without typedef */
```

## Explicit values

Values can be set manually; subsequent values continue from there:

```c
typedef enum {
    HTTP_OK           = 200,
    HTTP_CREATED      = 201,   /* 202 next if not specified */
    HTTP_NOT_FOUND    = 404,
    HTTP_SERVER_ERROR = 500
} HttpStatus;
```

Values do not need to be unique or contiguous.

## Bit-flag enums

Assign each constant a distinct power of two so they can be combined with `|`:

```c
typedef enum {
    PERM_NONE    = 0,
    PERM_READ    = 1 << 0,   /* 1 */
    PERM_WRITE   = 1 << 1,   /* 2 */
    PERM_EXECUTE = 1 << 2    /* 4 */
} Permission;

Permission p = PERM_READ | PERM_WRITE;

/* Test a bit */
if (p & PERM_WRITE) { /* ... */ }

/* Set a bit */
p |= PERM_EXECUTE;

/* Clear a bit */
p &= ~PERM_WRITE;
```

## Underlying type

An `enum` constant has type `int`. The enum type itself has an
implementation-defined integer type (usually `int`). You cannot portably
control which integer type backs an enum in C11 (unlike C++). If you need
a specific width, use a `typedef` alongside:

```c
typedef enum { A, B, C } MyEnum;
typedef uint8_t MyEnum8;   /* store in a byte if you need to */
```

## Sentinel / COUNT pattern

Place a sentinel at the end to record the number of values:

```c
typedef enum {
    NORTH, SOUTH, EAST, WEST,
    DIRECTION_COUNT   /* always last */
} Direction;

const char *names[DIRECTION_COUNT] = { "NORTH", "SOUTH", "EAST", "WEST" };

/* Bounds check */
int direction_valid(int d) { return (unsigned)d < DIRECTION_COUNT; }
```

Adding a new direction automatically updates `DIRECTION_COUNT` and the
bounds check. The array size is a compile-time constant.

## X-macro pattern

Define every enum member once in a list macro; derive parallel tables
from it automatically:

```c
#define COLOUR_LIST \
    X(RED,   "red",   0xFF0000) \
    X(GREEN, "green", 0x00FF00) \
    X(BLUE,  "blue",  0x0000FF)

/* Generate enum */
typedef enum {
#define X(id, name, hex) id,
    COLOUR_LIST
#undef X
    COLOUR_COUNT
} Colour;

/* Generate name table */
static const char *colour_name[COLOUR_COUNT] = {
#define X(id, name, hex) [id] = name,
    COLOUR_LIST
#undef X
};

/* Generate hex table */
static const unsigned colour_hex[COLOUR_COUNT] = {
#define X(id, name, hex) [id] = hex,
    COLOUR_LIST
#undef X
};
```

Adding a new colour requires changing one line in `COLOUR_LIST` — all
tables update automatically.

## Enum as array index (dispatch table)

Use the enum value as an index into a function-pointer array for O(1)
dispatch without a `switch`:

```c
typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_COUNT } Op;
typedef double (*BinFn)(double, double);

static double do_add(double a, double b) { return a + b; }
static double do_sub(double a, double b) { return a - b; }
static double do_mul(double a, double b) { return a * b; }

static const BinFn op_fn[OP_COUNT] = {
    [OP_ADD] = do_add,
    [OP_SUB] = do_sub,
    [OP_MUL] = do_mul,
};

/* Compile-time check: table must be fully populated */
_Static_assert(sizeof(op_fn) / sizeof(op_fn[0]) == OP_COUNT,
               "op_fn table out of sync with Op enum");

double apply(Op op, double a, double b) {
    assert((unsigned)op < OP_COUNT);
    return op_fn[op](a, b);
}
```

## State machine

Model a protocol or workflow as an enum (state) + transition table:

```c
typedef enum { S_IDLE, S_OPEN, S_CLOSED, S_COUNT } State;
typedef enum { E_OPEN, E_CLOSE, E_COUNT } Event;

#define NO_TRANS S_COUNT   /* sentinel: invalid transition */

static const State transitions[S_COUNT][E_COUNT] = {
    [S_IDLE] = { [E_OPEN]  = S_OPEN,   [E_CLOSE] = NO_TRANS },
    [S_OPEN] = { [E_OPEN]  = NO_TRANS, [E_CLOSE] = S_CLOSED },
    [S_CLOSED] = { [E_OPEN] = NO_TRANS, [E_CLOSE] = NO_TRANS },
};

int step(State *s, Event e) {
    State next = transitions[*s][e];
    if (next == NO_TRANS) return 0;   /* invalid */
    *s = next;
    return 1;
}
```

## _Static_assert for compile-time checks

Catch mismatches between enum size and table size at compile time:

```c
_Static_assert(DIRECTION_COUNT == 4,
               "Update direction tables when adding a Direction");
```

## enum vs #define

| | `enum` | `#define` |
|-|--------|-----------|
| Type safety | Yes (compiler warns on wrong type) | No |
| Debugger visibility | Yes (name shown in GDB) | No |
| Scope | Respects block scope | Global after definition |
| Can take address | No | No |
| Can be used in `switch` | Yes | Yes |

Prefer `enum` for named integer constants — the debugger will show
`NORTH` instead of `0`.

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Comparing signed enum to unsigned without cast | Use `(unsigned)val < COUNT` for bounds checks |
| Using `0` as "invalid" when `0` is a valid state | Use the COUNT sentinel as the invalid marker |
| Adding an enum member without updating `switch` | Enable `-Wswitch` (included in `-Wall`) — GCC warns on unhandled cases |
| Forgetting `#undef X` after an X-macro expansion | Always pair `#define X` with `#undef X` immediately after the use |
| Relying on enum being a specific integer width | Use `uint8_t` / `uint32_t` if size matters; cast explicitly |
