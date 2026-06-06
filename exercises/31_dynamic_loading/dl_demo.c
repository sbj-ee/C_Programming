/*
 * Exercise 31 — Dynamic Loading
 *
 * dlopen() loads a shared library at runtime; dlsym() resolves symbols
 * by name.  This enables plugin architectures where code is discovered
 * and loaded without recompiling the main program.
 *
 * Build: make   (builds plugin.so then dl_demo)
 * Run:   ./dl_demo
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "plugin.h"

/* ── helper ──────────────────────────────────────────────────────────────── */

/* Die with the current dlerror message */
static void dl_die(const char *ctx) {
    fprintf(stderr, "%s: %s\n", ctx, dlerror());
    exit(1);
}

/* ── 1. Concepts ─────────────────────────────────────────────────────────── */

static void section1_concepts(void) {
    puts("═══ 1. Dynamic Loading Concepts ════\n");

    puts("  Static linking: code baked into the binary at link time.");
    puts("  Dynamic linking: .so resolved at program start (default for libc).");
    puts("  dlopen:  .so loaded explicitly at runtime — the plugin model.\n");

    puts("  Key functions (link with -ldl):");
    puts("    dlopen(path, flags)      — load a shared library");
    puts("    dlsym(handle, \"symbol\") — look up a symbol by name");
    puts("    dlclose(handle)          — unload (reference-counted)");
    puts("    dlerror()                — last error string (clears after read)\n");

    puts("  dlopen flags:");
    puts("    RTLD_LAZY   — resolve symbols when first called (faster startup)");
    puts("    RTLD_NOW    — resolve all symbols immediately (catches missing ones)");
    puts("    RTLD_LOCAL  — symbols NOT visible to later dlopen calls (default)");
    puts("    RTLD_GLOBAL — symbols visible to subsequently loaded libraries\n");

    puts("  Special handles for dlsym:");
    puts("    RTLD_DEFAULT — search global symbol table (main + RTLD_GLOBAL libs)");
    puts("    RTLD_NEXT    — find next definition after current library (interposing)\n");
}

/* ── 2. Basic dlopen / dlsym / dlclose ───────────────────────────────────── */

static void section2_basic(void) {
    puts("═══ 2. Basic dlopen / dlsym / dlclose ════\n");

    /* Always clear dlerror before calling dlsym */
    dlerror();
    void *handle = dlopen("./plugin.so", RTLD_NOW | RTLD_LOCAL);
    if (!handle) dl_die("dlopen");
    printf("  dlopen(\"./plugin.so\"): handle=%p\n", handle);

    /* dlsym returns void*; cast via a union to avoid -pedantic warnings */
    dlerror();
    void *raw = dlsym(handle, "plugin_create");
    const char *err = dlerror();
    if (err) { fprintf(stderr, "dlsym: %s\n", err); dlclose(handle); return; }

    /* Casting function pointers: the POSIX-blessed approach */
    Plugin *(*create_fn)(void);
    memcpy(&create_fn, &raw, sizeof create_fn);

    void *raw2 = dlsym(handle, "plugin_destroy");
    void (*destroy_fn)(Plugin *);
    memcpy(&destroy_fn, &raw2, sizeof destroy_fn);

    Plugin *p = create_fn();
    printf("  plugin name:   %s\n", p->name);
    printf("  describe():    %s\n", p->describe());
    printf("  transform(3):  %.6f\n\n", p->transform(3.0));

    destroy_fn(p);
    dlclose(handle);
    puts("  dlclose: library unloaded (if reference count reached zero)\n");
}

/* ── 3. Plugin dispatch pattern ──────────────────────────────────────────── */

typedef struct {
    void   *handle;
    Plugin *plugin;
    void  (*destroy)(Plugin *);
} LoadedPlugin;

static LoadedPlugin load_plugin(const char *path) {
    LoadedPlugin lp = {0};
    dlerror();
    lp.handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!lp.handle) { fprintf(stderr, "dlopen(%s): %s\n", path, dlerror()); return lp; }

    void *c = dlsym(lp.handle, "plugin_create");
    void *d = dlsym(lp.handle, "plugin_destroy");
    if (!c || !d) { fprintf(stderr, "dlsym: %s\n", dlerror()); dlclose(lp.handle); lp.handle = NULL; return lp; }

    Plugin *(*create_fn)(void); memcpy(&create_fn, &c, sizeof create_fn);
    memcpy(&lp.destroy, &d, sizeof lp.destroy);
    lp.plugin = create_fn();
    return lp;
}

static void section3_dispatch(void) {
    puts("═══ 3. Plugin Dispatch Pattern ════\n");

    LoadedPlugin lp = load_plugin("./plugin.so");
    if (!lp.handle) return;

    /* The caller works through the Plugin interface — no knowledge of internals */
    printf("  Loaded: %s\n", lp.plugin->name);
    double inputs[] = { 0.0, 1.0, 3.0, 5.0 };
    for (int i = 0; i < 4; i++)
        printf("  %s  x=%.1f → %.6f\n",
               lp.plugin->describe(), inputs[i], lp.plugin->transform(inputs[i]));

    lp.destroy(lp.plugin);
    dlclose(lp.handle);
    puts("\n  Adding a new plugin = adding a new .so; no recompile of main needed.\n");
}

/* ── 4. RTLD_NOW: catching missing symbols at load time ──────────────────── */

static void section4_rtld_now(void) {
    puts("═══ 4. RTLD_NOW vs RTLD_LAZY ════\n");

    puts("  RTLD_LAZY: undefined symbols tolerated at dlopen; crash at first call.");
    puts("  RTLD_NOW:  all symbols resolved at dlopen; error returned immediately.\n");

    /* Attempt to load a non-existent library to show the error path */
    dlerror();
    void *h = dlopen("./nonexistent.so", RTLD_NOW);
    if (!h)
        printf("  dlopen(\"./nonexistent.so\"): %s\n\n", dlerror());

    /* Load the real plugin with RTLD_NOW — succeeds */
    h = dlopen("./plugin.so", RTLD_NOW);
    if (h) {
        puts("  dlopen(\"./plugin.so\", RTLD_NOW): OK");
        dlclose(h);
    }

    puts("\n  Rule: prefer RTLD_NOW in production; use RTLD_LAZY only when startup");
    puts("  time matters and you know which code paths will be taken.\n");
}

/* ── 5. RTLD_DEFAULT: finding symbols in loaded libraries ────────────────── */

static void section5_rtld_default(void) {
    puts("═══ 5. RTLD_DEFAULT: Search the Global Symbol Table ════\n");

    /* Find strlen from libc — always available in the global namespace */
    dlerror();
    void *sym = dlsym(RTLD_DEFAULT, "strlen");
    if (dlerror()) { puts("  strlen not found"); return; }

    size_t (*my_strlen)(const char *);
    memcpy(&my_strlen, &sym, sizeof my_strlen);
    printf("  strlen via dlsym: strlen(\"hello\") = %zu\n", my_strlen("hello"));
    printf("  symbol address: %p\n\n", sym);

    /* RTLD_DEFAULT also finds functions in the main executable (if exported) */
    puts("  Use cases for RTLD_DEFAULT:");
    puts("    - Runtime introspection / symbol existence checks");
    puts("    - Plugin calling back into the main program");
    puts("    - Building extensible dispatch tables at startup\n");
}

/* ── 6. Pitfalls ─────────────────────────────────────────────────────────── */

static void section6_pitfalls(void) {
    puts("═══ 6. Pitfalls and Best Practices ════\n");

    puts("  Casting function pointers from void*:");
    puts("    ISO C forbids direct cast of object pointer to function pointer.");
    puts("    Use memcpy(&fn_ptr, &void_ptr, sizeof fn_ptr) to be pedantic-clean.\n");

    puts("  dlerror() semantics:");
    puts("    Always call dlerror() to clear state before dlsym/dlopen.");
    puts("    Check dlerror() IMMEDIATELY after; a second call returns NULL.\n");

    puts("  dlclose and global state:");
    puts("    Unloading a plugin that registered global callbacks → crash.");
    puts("    Many programs never dlclose (leak the .so) to avoid this.\n");

    puts("  Symbol visibility:");
    puts("    Build plugins with -fvisibility=hidden and export only the entry");
    puts("    points (__attribute__((visibility(\"default\")))).");
    puts("    Prevents symbol collisions between plugins.\n");

    puts("  ABI stability:");
    puts("    Plugin interface struct must remain layout-compatible across");
    puts("    plugin versions.  Add new fields only at the END; use a version");
    puts("    field to negotiate capabilities.\n");

    puts("  Library search order for dlopen:");
    puts("    1. DT_RUNPATH / DT_RPATH embedded in the binary");
    puts("    2. LD_LIBRARY_PATH environment variable");
    puts("    3. /etc/ld.so.cache (populated by ldconfig)");
    puts("    4. /lib, /usr/lib");
    puts("    Use \"./\" prefix to load from the current directory.\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    section1_concepts();
    section2_basic();
    section3_dispatch();
    section4_rtld_now();
    section5_rtld_default();
    section6_pitfalls();
    return 0;
}
