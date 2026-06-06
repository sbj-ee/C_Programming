/*
 * plugin.c — compiled as a shared library: plugin.so
 * Implements the Plugin interface declared in plugin.h.
 */
#include "plugin.h"
#include <stdlib.h>
#include <math.h>

static double transform_fn(double x) { return sqrt(x * x + 1.0); }
static const char *describe_fn(void) { return "f(x) = sqrt(x^2 + 1)"; }

Plugin *plugin_create(void) {
    Plugin *p    = malloc(sizeof *p);
    p->name      = "math_plugin v1.0";
    p->transform = transform_fn;
    p->describe  = describe_fn;
    return p;
}

void plugin_destroy(Plugin *p) { free(p); }
