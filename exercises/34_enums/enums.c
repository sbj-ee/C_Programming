/*
 * Exercise 34 — Enums in Depth
 *
 * Exercise 12 introduced basic enums, explicit values, and bit-flag enums
 * alongside unions. This exercise focuses on four advanced enum patterns
 * that appear constantly in real C codebases:
 *
 *   1. X-macros  — generate parallel tables (names, labels, handlers)
 *                  from a single source-of-truth list.
 *   2. Sentinel   — a final COUNT/MAX value that gives the enum a size,
 *                   enables array sizing, and makes bounds-checking trivial.
 *   3. Dispatch   — use the enum as an array index into a function-pointer
 *                   table for O(1) dispatch without switch statements.
 *   4. State machine — model a protocol or workflow as an enum + transition
 *                      table; the enum is the current state.
 *
 * Compile: gcc -Wall -Wextra -Wpedantic -std=c11 -g enums.c -o enums
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ================================================================
   PATTERN 1: X-MACROS
   ================================================================
   An X-macro list defines every member of an enum once.  A macro X()
   is invoked for each entry, and callers redefine X to extract what
   they need — names, labels, handler table, etc.
   Adding a new colour requires changing exactly one line.           */

#define COLOUR_LIST \
    X(RED,   "red",   0xFF0000) \
    X(GREEN, "green", 0x00FF00) \
    X(BLUE,  "blue",  0x0000FF) \
    X(WHITE, "white", 0xFFFFFF) \
    X(BLACK, "black", 0x000000)

/* Generate the enum from the list */
typedef enum {
#define X(id, name, hex) id,
    COLOUR_LIST
#undef X
    COLOUR_COUNT   /* sentinel: number of colours */
} Colour;

/* Generate a name table */
static const char *colour_name[COLOUR_COUNT] = {
#define X(id, name, hex) [id] = name,
    COLOUR_LIST
#undef X
};

/* Generate a hex-value table */
static const unsigned int colour_hex[COLOUR_COUNT] = {
#define X(id, name, hex) [id] = hex,
    COLOUR_LIST
#undef X
};

/* Lookup by name — O(n) linear scan, but the table is always in sync */
Colour colour_from_name(const char *s) {
    for (Colour c = 0; c < COLOUR_COUNT; c++) {
        if (strcmp(colour_name[c], s) == 0) return c;
    }
    return COLOUR_COUNT; /* sentinel = not found */
}

/* ================================================================
   PATTERN 2: SENTINEL VALUE
   ================================================================
   Placing COUNT/MAX at the end of an enum provides:
     - A compile-time array size that updates automatically.
     - An out-of-range sentinel return value.
     - A bounds check: (unsigned)value < COUNT.                    */

#define DIRECTION_LIST \
    X(NORTH) X(SOUTH) X(EAST) X(WEST)

typedef enum {
#define X(id) id,
    DIRECTION_LIST
#undef X
    DIRECTION_COUNT
} Direction;

static const char *dir_name[DIRECTION_COUNT] = {
#define X(id) [id] = #id,   /* # stringifies the token */
    DIRECTION_LIST
#undef X
};

/* Reverse direction lookup */
static const Direction dir_opposite[DIRECTION_COUNT] = {
    [NORTH] = SOUTH,
    [SOUTH] = NORTH,
    [EAST]  = WEST,
    [WEST]  = EAST,
};

/* Bounds check using the sentinel */
int direction_valid(int d) {
    return (unsigned)d < DIRECTION_COUNT;
}

/* ================================================================
   PATTERN 3: ENUM-INDEXED DISPATCH TABLE
   ================================================================
   Instead of a switch, store one function pointer per enum value in
   an array indexed by the enum.  Dispatch is O(1) and the table is
   checked at compile time against the enum size via _Static_assert. */

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_COUNT
} Op;

static const char *op_symbol[OP_COUNT] = {
    [OP_ADD] = "+",
    [OP_SUB] = "-",
    [OP_MUL] = "*",
    [OP_DIV] = "/",
};

typedef double (*BinaryFn)(double, double);

static double do_add(double a, double b) { return a + b; }
static double do_sub(double a, double b) { return a - b; }
static double do_mul(double a, double b) { return a * b; }
static double do_div(double a, double b) { return b != 0.0 ? a / b : 0.0; }

static const BinaryFn op_fn[OP_COUNT] = {
    [OP_ADD] = do_add,
    [OP_SUB] = do_sub,
    [OP_MUL] = do_mul,
    [OP_DIV] = do_div,
};

/* Verifies array SIZE matches OP_COUNT, but does NOT catch missing entries:
 * a new Op value with no designated initialiser gets a NULL slot and will
 * crash at runtime in apply(). The assert() in apply() provides the runtime guard. */
_Static_assert(sizeof(op_fn) / sizeof(op_fn[0]) == OP_COUNT,
               "op_fn table size must match OP_COUNT");

double apply(Op op, double a, double b) {
    assert((unsigned)op < OP_COUNT);
    return op_fn[op](a, b);
}

/* ================================================================
   PATTERN 4: STATE MACHINE
   ================================================================
   A state machine models a system that moves through a finite set of
   named states in response to events.  The enum is the current state;
   a transition table maps (state, event) → next state.             */

typedef enum {
    STATE_IDLE,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_CLOSING,
    STATE_CLOSED,
    STATE_COUNT
} ConnState;

typedef enum {
    EVT_CONNECT,
    EVT_OPEN,
    EVT_SEND,
    EVT_CLOSE,
    EVT_ERROR,
    EVT_COUNT
} ConnEvent;

static const char *state_name[STATE_COUNT] = {
    [STATE_IDLE]       = "IDLE",
    [STATE_CONNECTING] = "CONNECTING",
    [STATE_CONNECTED]  = "CONNECTED",
    [STATE_CLOSING]    = "CLOSING",
    [STATE_CLOSED]     = "CLOSED",
};

static const char *event_name[EVT_COUNT] = {
    [EVT_CONNECT] = "CONNECT",
    [EVT_OPEN]    = "OPEN",
    [EVT_SEND]    = "SEND",
    [EVT_CLOSE]   = "CLOSE",
    [EVT_ERROR]   = "ERROR",
};

/*
 * Transition table: transitions[state][event] = next state.
 * STATE_COUNT is used as the "no valid transition" sentinel so that
 * zero-initialised (undefined) slots don't accidentally map to a
 * real state.  Every valid transition is listed explicitly.
 */
#define NO_TRANS STATE_COUNT

static const ConnState transitions[STATE_COUNT][EVT_COUNT] = {
    [STATE_IDLE] = {
        [EVT_CONNECT] = STATE_CONNECTING,
        [EVT_ERROR]   = STATE_CLOSED,
        [EVT_OPEN]    = NO_TRANS,
        [EVT_SEND]    = NO_TRANS,
        [EVT_CLOSE]   = NO_TRANS,
    },
    [STATE_CONNECTING] = {
        [EVT_OPEN]    = STATE_CONNECTED,
        [EVT_ERROR]   = STATE_CLOSED,
        [EVT_CONNECT] = NO_TRANS,
        [EVT_SEND]    = NO_TRANS,
        [EVT_CLOSE]   = NO_TRANS,
    },
    [STATE_CONNECTED] = {
        [EVT_SEND]    = STATE_CONNECTED,
        [EVT_CLOSE]   = STATE_CLOSING,
        [EVT_ERROR]   = STATE_CLOSED,
        [EVT_CONNECT] = NO_TRANS,
        [EVT_OPEN]    = NO_TRANS,
    },
    [STATE_CLOSING] = {
        [EVT_CLOSE]   = STATE_CLOSED,
        [EVT_ERROR]   = STATE_CLOSED,
        [EVT_CONNECT] = NO_TRANS,
        [EVT_OPEN]    = NO_TRANS,
        [EVT_SEND]    = NO_TRANS,
    },
    [STATE_CLOSED] = {
        [EVT_CONNECT] = NO_TRANS,
        [EVT_OPEN]    = NO_TRANS,
        [EVT_SEND]    = NO_TRANS,
        [EVT_CLOSE]   = NO_TRANS,
        [EVT_ERROR]   = NO_TRANS,
    },
};

/* Feed an event to the state machine; returns 0 on invalid transition */
int sm_step(ConnState *state, ConnEvent evt) {
    ConnState next = transitions[*state][evt];
    if (next == NO_TRANS) {
        printf("  invalid transition: %s + %s\n",
               state_name[*state], event_name[evt]);
        return 0;
    }
    printf("  %s + %s -> %s\n",
           state_name[*state], event_name[evt], state_name[next]);
    *state = next;
    return 1;
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void) {

    /* --- Pattern 1: X-macros --- */
    printf("=== X-macro: Colour tables ===\n");
    printf("COLOUR_COUNT = %d\n", COLOUR_COUNT);
    for (Colour c = 0; c < COLOUR_COUNT; c++) {
        printf("  [%d] %-6s  #%06X\n", c, colour_name[c], colour_hex[c]);
    }

    Colour found = colour_from_name("blue");
    printf("colour_from_name(\"blue\") = %d (%s)\n",
           found, found < COLOUR_COUNT ? colour_name[found] : "not found");

    Colour missing = colour_from_name("purple");
    printf("colour_from_name(\"purple\") = %s\n",
           missing == COLOUR_COUNT ? "not found" : colour_name[missing]);

    /* --- Pattern 2: Sentinel --- */
    printf("\n=== Sentinel: Direction ===\n");
    printf("DIRECTION_COUNT = %d\n", DIRECTION_COUNT);
    for (Direction d = 0; d < DIRECTION_COUNT; d++) {
        printf("  opposite(%s) = %s\n",
               dir_name[d], dir_name[dir_opposite[d]]);
    }
    printf("direction_valid(1) = %d\n", direction_valid(1));
    printf("direction_valid(9) = %d\n", direction_valid(9));

    /* --- Pattern 3: Dispatch table --- */
    printf("\n=== Dispatch table: Op ===\n");
    double a = 10.0, b = 4.0;
    for (Op op = 0; op < OP_COUNT; op++) {
        printf("  %.0f %s %.0f = %g\n",
               a, op_symbol[op], b, apply(op, a, b));
    }

    /* --- Pattern 4: State machine --- */
    printf("\n=== State machine: connection lifecycle ===\n");
    ConnState state = STATE_IDLE;
    printf("initial state: %s\n", state_name[state]);

    ConnEvent sequence[] = {
        EVT_CONNECT, EVT_OPEN, EVT_SEND, EVT_SEND, EVT_CLOSE, EVT_CLOSE
    };
    int nev = (int)(sizeof(sequence) / sizeof(sequence[0]));
    for (int i = 0; i < nev; i++) {
        sm_step(&state, sequence[i]);
    }
    printf("final state: %s\n", state_name[state]);

    /* Invalid transition demo */
    printf("\nInvalid transition demo:\n");
    state = STATE_IDLE;
    sm_step(&state, EVT_SEND);   /* can't send from IDLE */

    return 0;
}
