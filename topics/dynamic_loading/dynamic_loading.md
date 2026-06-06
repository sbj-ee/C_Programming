# Dynamic Loading (dlopen)

Load shared libraries at runtime; resolve symbols by name. Enables plugin
architectures where new functionality is added without recompiling.

Requires `#define _GNU_SOURCE` and `<dlfcn.h>`. Link with **`-ldl`**.

## Key functions

```c
#include <dlfcn.h>

void *dlopen (const char *filename, int flags);
void *dlsym  (void *handle, const char *symbol);
int   dlclose(void *handle);
char *dlerror(void);             /* returns NULL if no error */
```

## dlopen flags

| Flag | Meaning |
|------|---------|
| `RTLD_LAZY` | Resolve symbols on first call (faster startup) |
| `RTLD_NOW` | Resolve all symbols at load time (catches missing ones immediately) |
| `RTLD_LOCAL` | Symbols NOT visible to later `dlopen` calls (default) |
| `RTLD_GLOBAL` | Symbols visible to subsequently loaded libraries |
| `RTLD_NODELETE` | Don't unload on `dlclose` (useful for libraries with global state) |

Prefer `RTLD_NOW | RTLD_LOCAL` in production — fail fast, no pollution.

## Basic usage

```c
/* Always clear dlerror before each call */
dlerror();
void *handle = dlopen("./plugin.so", RTLD_NOW | RTLD_LOCAL);
if (!handle) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
}

/* Look up a symbol */
dlerror();
void *sym = dlsym(handle, "plugin_create");
char *err = dlerror();
if (err) { fprintf(stderr, "dlsym: %s\n", err); dlclose(handle); return -1; }

/* Cast to function pointer (ISO C: use memcpy, not a direct cast) */
Plugin *(*create_fn)(void);
memcpy(&create_fn, &sym, sizeof create_fn);

Plugin *p = create_fn();
/* use p ... */

dlclose(handle);   /* reference-counted; library unloads at refcount 0 */
```

## Casting function pointers safely

ISO C forbids casting `void *` to a function pointer directly. Use `memcpy`:

```c
void *sym = dlsym(handle, "my_func");

/* Correct (pedantic-clean) */
int (*fn)(int, int);
memcpy(&fn, &sym, sizeof fn);

/* Also accepted by most compilers but technically UB under strict ISO C */
fn = (int (*)(int, int))sym;
```

## Special handles for dlsym

| Handle | Search scope |
|--------|-------------|
| `RTLD_DEFAULT` | Main executable + all `RTLD_GLOBAL` libraries |
| `RTLD_NEXT` | Next definition after the current library (for interposing) |

```c
/* Find strlen from libc (always globally visible) */
void *sym = dlsym(RTLD_DEFAULT, "strlen");

/* Interpose malloc — call the real one from inside the wrapper */
void *sym = dlsym(RTLD_NEXT, "malloc");
```

## Plugin interface pattern

Define a stable C struct as the plugin contract:

```c
/* plugin.h — shared header */
typedef struct {
    const char *name;
    int  version;
    void (*init)(void);
    int  (*process)(const char *input, char *output, size_t len);
    void (*destroy)(void);
} Plugin;

/* Entry points every plugin must export */
Plugin *plugin_create(void);
void    plugin_destroy(Plugin *);
```

```c
/* loader */
Plugin *(*create)(void);
void    (*destroy)(Plugin *);

void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
void *c = dlsym(h, "plugin_create");
void *d = dlsym(h, "plugin_destroy");
memcpy(&create,  &c, sizeof create);
memcpy(&destroy, &d, sizeof destroy);

Plugin *p = create();
p->init();
p->process(input, output, sizeof output);
destroy(p);
dlclose(h);
```

## Symbol visibility in the plugin

Build plugins with `-fvisibility=hidden` and explicitly export only entry points:

```c
/* plugin.c */
__attribute__((visibility("default")))
Plugin *plugin_create(void) { ... }

__attribute__((visibility("default")))
void plugin_destroy(Plugin *p) { ... }

/* Internal helpers — not exported */
static int helper(int x) { ... }
```

Build: `gcc -shared -fPIC -fvisibility=hidden plugin.c -o plugin.so`

## Library search order for dlopen

1. `DT_RUNPATH` / `DT_RPATH` embedded in the binary (`-Wl,-rpath,/path`)
2. `LD_LIBRARY_PATH` environment variable
3. `/etc/ld.so.cache` (populated by `ldconfig`)
4. `/lib`, `/usr/lib`

Use `./` prefix to load from the current directory: `dlopen("./plugin.so", ...)`.

## Building shared libraries

```bash
# Compile to position-independent object
gcc -Wall -std=c11 -fPIC -c plugin.c -o plugin.o

# Link as shared library
gcc -shared plugin.o -o plugin.so [-lm]

# Embed runtime search path (so dlopen finds it without LD_LIBRARY_PATH)
gcc -Wl,-rpath,'$ORIGIN' main.c -ldl -o main
# $ORIGIN = directory of the executable at runtime
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Not calling `dlerror()` to clear before `dlsym` | Always clear before, check immediately after |
| Calling `dlerror()` twice — second call returns NULL | Save the result in a `char *` immediately |
| Casting `void *` to function pointer directly | Use `memcpy` for pedantic compliance |
| Calling `dlclose` on a plugin that registered global callbacks | Don't `dlclose`; or track and remove callbacks first |
| Missing `-fPIC` when building the `.so` | Shared libraries require position-independent code |
| Using `RTLD_GLOBAL` carelessly | Symbol collisions between plugins; use `RTLD_LOCAL` |
| ABI break between plugin versions | Add fields only at the END of the struct; version the interface |
