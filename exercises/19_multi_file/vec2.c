#include "vec2.h"      /* own header first — catches missing declarations */
#include <stdio.h>
#include <math.h>

/* 'static' limits these helpers to this translation unit.
   They don't appear in vec2.h because callers have no business
   calling them directly. */
/* sq is static — a private helper visible only within this file.
   It is not declared in vec2.h so callers cannot access it. */
static double sq(double x) { return x * x; }

Vec2 vec2(double x, double y)           { return (Vec2){x, y}; }
Vec2 vec2_add(Vec2 a, Vec2 b)           { return (Vec2){a.x+b.x, a.y+b.y}; }
Vec2 vec2_sub(Vec2 a, Vec2 b)           { return (Vec2){a.x-b.x, a.y-b.y}; }
Vec2 vec2_scale(Vec2 v, double s)       { return (Vec2){v.x*s, v.y*s}; }
double vec2_dot(Vec2 a, Vec2 b)         { return a.x*b.x + a.y*b.y; }
double vec2_len(Vec2 v)                 { return sqrt(sq(v.x) + sq(v.y)); }

Vec2 vec2_norm(Vec2 v) {
    double len = vec2_len(v);
    if (len == 0.0) return (Vec2){0, 0};
    return vec2_scale(v, 1.0 / len);
}

void vec2_print(const char *label, Vec2 v) {
    printf("%-10s = (%.3f, %.3f)\n", label, v.x, v.y);
}
