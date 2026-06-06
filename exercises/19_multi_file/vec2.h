/* vec2.h — 2D vector library public interface
   Include guard prevents double-processing if this header is
   included from multiple translation units. */
#ifndef VEC2_H
#define VEC2_H

typedef struct {
    double x;
    double y;
} Vec2;

/* All declarations live here; definitions live in vec2.c */
Vec2   vec2(double x, double y);
Vec2   vec2_add(Vec2 a, Vec2 b);
Vec2   vec2_sub(Vec2 a, Vec2 b);
Vec2   vec2_scale(Vec2 v, double s);
double vec2_dot(Vec2 a, Vec2 b);
double vec2_len(Vec2 v);
Vec2   vec2_norm(Vec2 v);           /* unit vector */
void   vec2_print(const char *label, Vec2 v);

#endif /* VEC2_H */
