#ifndef PLUGIN_H
#define PLUGIN_H

/* Interface that every plugin must implement */
typedef struct {
    const char *name;                      /* human-readable plugin name    */
    double (*transform)(double x);         /* the plugin's primary function */
    const char *(*describe)(void);         /* one-line description          */
} Plugin;

/* Entry points dlsym'd by the loader */
Plugin *plugin_create(void);
void    plugin_destroy(Plugin *p);

#endif
