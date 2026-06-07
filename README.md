# C Programming

A structured C programming learning project covering fundamentals through systems programming.

## Layout

```
C_Programming/
├── Foreword.md      # Historical context: C, UNIX, and Linux
├── Introduction.md  # Dev environment, build tools, exercise progression
├── exercises/       # Progressive programs, each building on the last
│   ├── 01_hello_world/ … 33_terminal/
└── topics/              # Markdown reference sheets by concept
    ├── types/               integer types, casts, sizeof, float pitfalls
    ├── pointers/            declaration, arithmetic, void*, function pointers
    ├── memory/              malloc/free, ownership, goto cleanup, valgrind
    ├── strings/             null termination, key functions, safe patterns
    ├── structs/             typedef, designated init, nested, bit fields
    ├── file_io/             FILE*, fopen/fread/fwrite, POSIX fd API
    ├── preprocessor/        macros, #/##, variadic, include guards, X-macros
    ├── error_handling/      return codes, errno, goto cleanup, setjmp
    ├── concurrency/         pthreads, mutex, condition variable, pitfalls
    ├── processes/           fork, exec, waitpid, pipes, popen, zombies
    ├── signals/             signal/sigaction, sig_atomic_t, sigprocmask, SIGALRM
    ├── sockets/             TCP/UDP skeleton, framing, AF_UNIX, getaddrinfo
    ├── mmap/                file-backed, anonymous, MAP_SHARED, msync
    ├── io_multiplexing/     select/poll/epoll, O_NONBLOCK, EPOLLET
    ├── atomics/             _Atomic, CAS, memory ordering, lock-free stack
    ├── semaphores/          unnamed/named, counting, timedwait, cross-process
    ├── dynamic_loading/     dlopen/dlsym, plugin pattern, visibility, ABI
    ├── regex/               regcomp/regexec, ERE syntax, scan loop, patterns
    └── terminal/            termios, raw mode, ANSI escapes, TIOCGWINSZ
```

## Build

```bash
# Build everything
make

# Build a single file
gcc -Wall -Wextra -std=c11 -g exercises/01_hello_world/hello.c -o hello

# Remove all compiled binaries
make clean
```

## Requirements

- `gcc` or `clang`
- `make`

## Exercises

| # | Topic | Concepts |
|---|-------|----------|
| 01 | Hello World | `printf`, `main`, `return` |
| 02 | Variables & Types | `int`, `float`, `char`, `sizeof`, type casting |
| 03 | Control Flow | `if/else`, `switch`, `for`, `while`, `do-while` |
| 04 | Functions | declaration, definition, scope, recursion |
| 05 | Arrays & Strings | fixed arrays, `char[]`, `strlen`, `strcpy` |
| 06 | Pointers | address-of, dereference, pointer arithmetic |
| 07 | Structs | `struct`, `typedef`, nested structs |
| 08 | Memory Management | `malloc`, `calloc`, `realloc`, `free`, valgrind |
| 09 | File I/O | `fopen`, `fclose`, `fread`, `fwrite`, `fprintf` |
| 10 | Linked Lists | singly linked list with insert, delete, traverse |
| 11 | Function Pointers | syntax, callbacks, dispatch tables, returning pointers, `qsort` |
| 12 | Enums & Unions | `enum`, bit-flag enums, `union`, tagged unions, variant types |
| 13 | Bit Manipulation | operators, set/clear/toggle/test, popcount, bitfields, packing |
| 14 | Preprocessor | object/function-like macros, `#`, `##`, variadic, `#ifdef`, include guards |
| 15 | Binary Search Tree | insert, search, delete (3 cases), traversals, height, visualisation |
| 16 | Stack & Queue | LIFO/FIFO, linked-list implementation, bracket balancing, BFS |
| 17 | Sorting | bubble, insertion, selection, merge, quicksort, stdlib qsort, benchmarks |
| 18 | Hash Table | djb2 hash, separate chaining, put/get/delete, rehash, word frequency |
| 19 | Multi-file Programs | headers, include guards, `static`, `extern`, compilation pipeline, modules |
| 20 | Error Handling | return codes, `errno`, `perror`/`strerror`, error propagation, `goto` cleanup, `setjmp`/`longjmp`, custom error types |
| 21 | Generics via `void *` | type-erased swap/min, generic comparators, `qsort`, `bsearch`, generic dynamic array, type-tagged variants |
| 22 | Variadic Functions | `va_list`, `va_start`, `va_arg`, `va_end`, `va_copy`, sentinel termination, `vsnprintf` wrappers, `_Generic`, tagged-union type safety |
| 23 | Signal Handling | signal table, `signal()`, `sigaction()`, `sig_atomic_t`, `SA_RESTART`, `SA_SIGINFO`, `sigprocmask`, `SIGALRM`/`alarm()`, async-signal safety |
| 24 | POSIX Threads | `pthread_create`/`join`, race conditions, `pthread_mutex_t`, `pthread_cond_t`, producer/consumer, thread attributes, detached threads, deadlock |
| 25 | Process Control | `fork`, `waitpid`, `W*` macros, `execvp`, pipes, two-way IPC, `popen`, zombies, `FD_CLOEXEC` |
| 26 | Sockets | `socket`/`bind`/`listen`/`accept`/`connect`, TCP loopback, UDP datagrams, AF_UNIX, `getaddrinfo`, `SO_REUSEADDR`, `SIGPIPE` |
| 27 | Memory-Mapped Files | `mmap`/`munmap`, file-backed read/write, `MAP_SHARED`/`MAP_PRIVATE`, anonymous mapping, shared memory across `fork`, `msync`, `ftruncate` |
| 28 | I/O Multiplexing | `select()`, `poll()`, `epoll_create1`/`epoll_ctl`/`epoll_wait`, `O_NONBLOCK`, `EPOLLET`, multi-client echo server |
| 29 | C11 Atomics | `_Atomic`, `atomic_fetch_add`, `atomic_compare_exchange`, memory ordering, spinlock (`atomic_flag`), lock-free stack |
| 30 | Semaphores | `sem_init`/`sem_wait`/`sem_post`/`sem_destroy`, binary and counting semaphores, `sem_timedwait`, named semaphores (`sem_open`/`sem_unlink`) |
| 31 | Dynamic Loading | `dlopen`/`dlsym`/`dlclose`/`dlerror`, `RTLD_NOW`/`RTLD_LAZY`, `RTLD_DEFAULT`, plugin dispatch pattern, `memcpy` function-pointer cast |
| 32 | Regular Expressions | `regcomp`/`regexec`/`regfree`/`regerror`, ERE vs BRE, capture groups, `REG_ICASE`/`REG_NEWLINE`, scan loop, IPv4/date/ident patterns |
| 33 | Terminal I/O | `termios`, canonical vs raw mode, `cfmakeraw`, `tcgetattr`/`tcsetattr`, `VMIN`/`VTIME`, ANSI escape codes, `TIOCGWINSZ`, restore-on-exit |

---

## Appendix A: Makefile

The root `Makefile` uses a two-level build strategy: exercises with special
link flags or multiple source files manage their own local `Makefile`; the
root Makefile delegates to them and compiles the remaining single-file
exercises directly.

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g

VALGRIND = valgrind --leak-check=full --error-exitcode=1

# Directories that manage their own build (have a local Makefile)
_MANAGED     := $(shell find exercises topics -mindepth 2 -maxdepth 2 -name 'Makefile' \
                         -exec dirname {} \; 2>/dev/null)
_EXCL        := $(foreach d,$(_MANAGED),-not -path '$(d)/*.c')

# Single-file exercises: one .c per directory, built by this Makefile
SRCS := $(shell find exercises topics $(_EXCL) -name '*.c')
BINS := $(SRCS:.c=)

.PHONY: all clean valgrind

all: $(BINS)
	@for d in $(_MANAGED); do $(MAKE) -C $$d all; done

%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lm

valgrind: all
	@for bin in $(BINS); do \
		echo "--- $$bin ---"; \
		$(VALGRIND) $$bin 2>&1 | grep -E "ERROR SUMMARY|no leaks"; \
	done
	@for d in $(_MANAGED); do $(MAKE) -C $$d valgrind 2>/dev/null || true; done

clean:
	@find exercises topics -type f ! -name '*.c' ! -name '*.h' ! -name 'Makefile' -delete
	@for d in $(_MANAGED); do $(MAKE) -C $$d clean; done
```

### Key concepts

**Variables**

| Variable | Purpose |
|----------|---------|
| `CC` | Compiler — swap to `clang` to cross-check warnings |
| `CFLAGS` | Flags passed to every compilation |
| `_MANAGED` | Directories with their own local `Makefile` (multi-file or special link flags) |
| `SRCS` | `.c` files in non-managed directories — compiled by the root pattern rule |
| `BINS` | Same list with `.c` stripped — the target binary names |

**Flags explained**

| Flag | Meaning |
|------|---------|
| `-Wall` | Enable common warnings |
| `-Wextra` | Enable extra warnings not covered by `-Wall` |
| `-Wpedantic` | Enforce strict ISO C compliance |
| `-std=c11` | Compile as C11 (allows `//` comments, `_Bool`, etc.) |
| `-g` | Embed debug symbols for use with `gdb` / valgrind |
| `-lm` | Link the math library (`<math.h>` functions like `sqrt`) |

**Pattern rule**

```makefile
%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lm
```

`%` is a wildcard. `$<` expands to the first prerequisite (the `.c` file);
`$@` expands to the target name (the binary). This single rule handles every
non-managed exercise without listing them individually.

**Local Makefiles**

Exercises with multiple source files or special link flags use their own
`Makefile` and are excluded from the root pattern rule:

| Exercise | Reason |
|----------|--------|
| 19 Multi-file | Multiple `.c` + `.h` files |
| 24 Threads | `-pthread` |
| 29 Atomics | `-pthread` |
| 30 Semaphores | `-pthread` |
| 31 Dynamic Loading | `-fPIC -shared` for plugin; `-ldl` for loader |

**Automatic variables**

| Variable | Expands to |
|----------|-----------|
| `$@` | The target (left side of `:`) |
| `$<` | The first prerequisite |
| `$^` | All prerequisites |
| `$*` | The stem matched by `%` |

**Phony targets**

`.PHONY` tells make these names are not real files. Without it, if a file
named `clean` existed in the directory, `make clean` would do nothing.

**Common `make` invocations**

```bash
make                                          # build everything
make -C exercises/03_control_flow             # build one exercise
make valgrind                                 # run valgrind on all exercises
make clean                                    # delete all compiled binaries
make CC=clang                                 # override the compiler
make -j$(nproc)                               # parallel build
make -n                                       # dry run: print without running
```

---

## Appendix B: Valgrind

Valgrind's Memcheck tool intercepts every memory operation at runtime and
reports errors that compilers and sanitisers can miss — particularly
use-after-free bugs that only surface under specific execution paths.

### Installation

```bash
sudo apt install valgrind        # Debian / Ubuntu
sudo dnf install valgrind        # Fedora / RHEL
```

### Basic invocation

```bash
valgrind ./myprog                           # minimal — shows errors only
valgrind --leak-check=full ./myprog         # full leak report
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --error-exitcode=1 \
         ./myprog arg1 arg2                 # recommended for CI
```

### Key flags

| Flag | Effect |
|------|--------|
| `--leak-check=full` | Report every individual leaked block |
| `--show-leak-kinds=all` | Include indirect and possible leaks, not just definite |
| `--track-origins=yes` | Show where uninitialised values were allocated (slower) |
| `--error-exitcode=N` | Exit with code N if any errors found (useful in scripts) |
| `--suppressions=file` | Suppress known false positives (see below) |
| `--gen-suppressions=all` | Print suppression entries for every error found |
| `-q` / `--quiet` | Only print errors, no preamble or summary |
| `--log-file=out.txt` | Write output to a file instead of stderr |
| `--num-callers=20` | Deeper stack traces (default 12) |

### Reading the output

A typical leak report:

```
==12345== LEAK SUMMARY:
==12345==    definitely lost: 40 bytes in 1 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==         suppressed: 0 bytes in 0 blocks
```

| Category | Meaning |
|----------|---------|
| **definitely lost** | Pointer to block is gone — classic leak |
| **indirectly lost** | Reachable only through a definitely-lost pointer |
| **possibly lost** | Pointer exists but points into the middle of a block |
| **still reachable** | Pointer still exists at exit — often intentional globals |
| **suppressed** | Matched a suppression rule — not shown |

A typical invalid-read error:

```
==12345== Invalid read of size 4
==12345==    at 0x10916B: main (heap.c:23)
==12345==  Address 0x4a5b080 is 0 bytes after a block of size 40 alloc'd
==12345==    at 0x4848899: malloc (vg_replace_malloc.c:381)
==12345==    by 0x109148: main (heap.c:18)
```

The process ID (`12345`) prefixes every line. Stack frames are listed
innermost-first. Fix the code at the line shown in your source file.

### Common error patterns

**Invalid read / write**
```c
int *p = malloc(10 * sizeof(int));
p[10] = 99;    /* one past the end — invalid write of size 4 */
free(p);
```

**Use after free**
```c
int *p = malloc(sizeof(int));
free(p);
*p = 1;        /* invalid write — block already freed */
```

**Uninitialised value**
```c
int x;
if (x > 0) {}  /* conditional jump on uninitialised value */
```
Add `--track-origins=yes` to see where `x` was allocated.

**Memory leak**
```c
void f(void) {
    char *buf = malloc(64);
    /* forgot to free(buf) before returning */
}
```

**Double free**
```c
int *p = malloc(sizeof(int));
free(p);
free(p);       /* invalid free — already freed */
```

### Suppression files

Some libraries (glibc, OpenSSL) have intentional one-time allocations that
valgrind flags as still-reachable. Suppress them with a `.supp` file:

```
# myproject.supp
{
   glibc_dl_init
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   ...obj:/lib/x86_64-linux-gnu/ld-*.so
}
```

```bash
valgrind --suppressions=myproject.supp --leak-check=full ./myprog
```

Generate suppressions automatically for each error:

```bash
valgrind --gen-suppressions=all --leak-check=full ./myprog 2>&1 | \
    grep -A 20 "^{" > myproject.supp
```

### Valgrind and threads

Valgrind's **Helgrind** tool detects data races:

```bash
valgrind --tool=helgrind ./threaded_prog
```

And **DRD** is a lighter-weight alternative:

```bash
valgrind --tool=drd ./threaded_prog
```

Note: threaded programs run significantly slower under Helgrind/DRD
because every memory access is instrumented.

### AddressSanitizer — faster alternative

For development builds, AddressSanitizer (ASan) is 2–20× faster than
Valgrind and catches most of the same errors:

```bash
gcc -Wall -Wextra -std=c11 -g \
    -fsanitize=address,undefined \
    prog.c -o prog
./prog
```

| | Valgrind | ASan |
|-|----------|------|
| Speed overhead | ~20–50× | ~2× |
| Requires recompile | No | Yes |
| Leak detection | Yes | Yes (with `LSAN_OPTIONS=detect_leaks=1`) |
| Uninitialised reads | Yes | Partial (use `-fsanitize=memory` / MSan) |
| Use after free | Yes | Yes |
| Works on existing binary | Yes | No |

Use ASan for fast iteration during development; run Valgrind before
committing to catch anything ASan misses.

---

## Appendix C: Creating Libraries

A **library** bundles compiled object files so other programs can link against
them without recompiling the source. C has two kinds.

### Object files

Before making a library, compile source to an object file (`.o`) with `-c`
(compile only, no linking):

```bash
gcc -Wall -std=c11 -c mylib.c -o mylib.o
```

### Static library (`.a`)

A static library is an archive of `.o` files. The linker copies the needed
code directly into the final binary — no runtime dependency.

```bash
# 1. Compile to object file(s)
gcc -Wall -std=c11 -c mylib.c -o mylib.o

# 2. Archive into a static library
ar rcs libmylib.a mylib.o          # 'r' insert, 'c' create, 's' index

# 3. Link a program against it
gcc main.c -L. -lmylib -o myprogram
#           ^^              ^^^^^^
#           look in .       lib prefix and .a suffix are implied
```

The header file `mylib.h` declares the public interface; `mylib.c` provides
the implementation.

```
mylib.h      ← shared with callers (#include "mylib.h")
mylib.c      ← implementation
mylib.o      ← compiled object (intermediate)
libmylib.a   ← static library (archive of .o files)
myprogram    ← final binary with library code baked in
```

### Shared library (`.so`)

A shared library is loaded at runtime and shared between processes — only one
copy lives in memory no matter how many programs use it.

```bash
# 1. Compile with Position-Independent Code (-fPIC)
gcc -Wall -std=c11 -fPIC -c mylib.c -o mylib.o

# 2. Link into a shared library
gcc -shared -o libmylib.so mylib.o

# 3. Link a program against it
gcc main.c -L. -lmylib -o myprogram

# 4. Tell the dynamic linker where to find it at runtime
export LD_LIBRARY_PATH=.
./myprogram
```

| | Static (`.a`) | Shared (`.so`) |
|-|--------------|----------------|
| Linking | At compile time | At runtime |
| Binary size | Larger (code embedded) | Smaller (code referenced) |
| Deployment | Single self-contained binary | Must ship the `.so` too |
| Updates | Recompile to pick up library changes | Replace `.so`, no recompile |
| Typical use | CLI tools, embedded | System libraries, plugins |

### Minimal worked example

**`vec2.h`**
```c
#ifndef VEC2_H
#define VEC2_H

typedef struct { double x, y; } Vec2;

Vec2   vec2_add(Vec2 a, Vec2 b);
double vec2_len(Vec2 v);

#endif
```

**`vec2.c`**
```c
#include "vec2.h"
#include <math.h>

Vec2 vec2_add(Vec2 a, Vec2 b) {
    return (Vec2){ a.x + b.x, a.y + b.y };
}

double vec2_len(Vec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}
```

**`main.c`**
```c
#include <stdio.h>
#include "vec2.h"

int main(void) {
    Vec2 a = {3.0, 0.0};
    Vec2 b = {0.0, 4.0};
    Vec2 c = vec2_add(a, b);
    printf("length = %.1f\n", vec2_len(c));   /* 5.0 */
    return 0;
}
```

**Build and run**
```bash
# Static
gcc -Wall -std=c11 -c vec2.c -o vec2.o
ar rcs libvec2.a vec2.o
gcc main.c -L. -lvec2 -lm -o myprogram
./myprogram

# Shared
gcc -Wall -std=c11 -fPIC -c vec2.c -o vec2.o
gcc -shared -o libvec2.so vec2.o
gcc main.c -L. -lvec2 -lm -o myprogram
LD_LIBRARY_PATH=. ./myprogram
```
