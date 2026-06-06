/* main.c — uses two separate modules: vec2 and stats.
   #include brings in declarations; the linker resolves the actual
   function bodies from vec2.o and stats.o at link time. */
#include <stdio.h>
#include "vec2.h"
#include "stats.h"

/* ================================================================
   The compilation pipeline for this exercise:

   vec2.c  ──┐
              ├─ compiler ─► vec2.o  ──┐
   vec2.h  ──┘                         │
                                        ├─ linker ─► multi_file
   stats.c ──┐                         │
              ├─ compiler ─► stats.o ──┘
   stats.h ──┘                         │
                                        │
   main.c  ──┐                         │
              ├─ compiler ─► main.o  ──┘
   vec2.h  ──┤
   stats.h ──┘

   Each .c file is compiled independently (it only sees its own
   headers). The linker combines the .o files into one binary.
   ================================================================ */

int main(void) {

    /* --- vec2 module --- */
    printf("=== vec2 module ===\n");

    Vec2 a = vec2(3.0, 4.0);
    Vec2 b = vec2(1.0, 2.0);

    vec2_print("a",        a);
    vec2_print("b",        b);
    vec2_print("a+b",      vec2_add(a, b));
    vec2_print("a-b",      vec2_sub(a, b));
    vec2_print("a*2",      vec2_scale(a, 2.0));
    vec2_print("norm(a)",  vec2_norm(a));
    printf("%-10s = %.3f\n", "dot(a,b)", vec2_dot(a, b));
    printf("%-10s = %.3f\n", "len(a)",   vec2_len(a));

    /* --- stats module --- */
    printf("\n=== stats module ===\n");

    double data[] = {4.0, 7.0, 13.0, 2.0, 1.0, 9.0, 6.0, 3.0};
    int    n      = sizeof(data) / sizeof(data[0]);

    printf("data:     ");
    for (int i = 0; i < n; i++) printf("%.0f ", data[i]);
    printf("\n");
    printf("mean:     %.3f\n", stats_mean(data, n));
    printf("variance: %.3f\n", stats_variance(data, n));
    printf("stddev:   %.3f\n", stats_stddev(data, n));
    printf("min:      %.3f\n", stats_min(data, n));
    printf("max:      %.3f\n", stats_max(data, n));

    /* extern variable defined in stats.c, visible here via the header */
    printf("stats_calls (extern): %d\n", stats_calls);

    /* --- Demonstrate that each module is independent ---
       main.c knows nothing about vec2.c's internals (e.g. the static
       clamp() helper) — it only sees what vec2.h declares.
       This is information hiding / encapsulation in C. */
    printf("\n=== Module independence ===\n");
    printf("vec2.h exports %d functions\n", 8);
    printf("stats.h exports %d functions + 1 extern variable\n", 5);
    printf("static helpers in vec2.c are invisible to main.c\n");

    return 0;
}
