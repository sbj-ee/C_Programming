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
    ├── enums/               X-macros, sentinel, dispatch tables, state machines
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
| 34 | Enums in Depth | X-macros, sentinel/COUNT pattern, enum-indexed dispatch tables, state machines with transition tables, `_Static_assert` size checks |

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
	@find exercises topics -type f ! -name '*.c' ! -name '*.h' ! -name '*.md' ! -name 'Makefile' -delete
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

## Appendix C: Debugging with GDB

GDB (GNU Debugger) lets you pause a running program, inspect memory and
variables, step through code line by line, and understand exactly why it
crashed or produced the wrong result. Always compile with `-g` to embed
debug symbols.

### Starting GDB

```bash
gdb ./myprog                      # load program
gdb ./myprog core                 # load program with a core dump
gdb --args ./myprog arg1 arg2     # pass arguments at launch
gdb -q ./myprog                   # quiet mode — suppress banner
```

Inside GDB, pass arguments with `set args` before `run`:

```
(gdb) set args input.txt 42
(gdb) run
```

### Breakpoints

```
(gdb) break main                  # break at function entry
(gdb) break myfile.c:42           # break at file:line
(gdb) break func if x > 10       # conditional breakpoint
(gdb) info breakpoints            # list all breakpoints
(gdb) disable 2                   # disable breakpoint #2
(gdb) enable 2                    # re-enable it
(gdb) delete 2                    # delete breakpoint #2
(gdb) clear func                  # delete all breakpoints at func
```

### Running and stepping

```
(gdb) run                         # start (or restart) the program
(gdb) continue   (c)              # resume until next breakpoint
(gdb) next       (n)              # step over — execute one line
(gdb) step       (s)              # step into — descend into calls
(gdb) finish                      # run until current function returns
(gdb) until 55                    # run until line 55 (skip loops)
(gdb) jump 60                     # jump execution to line 60
```

### Inspecting variables and memory

```
(gdb) print x                     # print value of x
(gdb) print *ptr                  # dereference a pointer
(gdb) print arr[0]@10             # print 10 elements of arr[]
(gdb) print (int *)0x4a5b080      # cast and print a raw address
(gdb) display x                   # print x after every step
(gdb) undisplay 1                 # remove display #1
(gdb) info locals                 # all local variables
(gdb) info args                   # function arguments
(gdb) whatis x                    # show type of x
(gdb) ptype struct Node           # show full struct layout
```

Examine raw memory:

```
(gdb) x/10d ptr                   # 10 decimal words starting at ptr
(gdb) x/10x ptr                   # 10 hex words
(gdb) x/10c ptr                   # 10 chars (useful for strings)
(gdb) x/10i $pc                   # 10 disassembled instructions at PC
```

Format letters for `x`: `d` decimal, `x` hex, `o` octal, `c` char,
`s` string, `f` float, `i` instruction. Size letters: `b` byte,
`h` halfword (2), `w` word (4), `g` giant (8).

### The call stack

```
(gdb) backtrace      (bt)         # show full call stack
(gdb) backtrace 5                 # show top 5 frames only
(gdb) frame 2                     # switch to frame #2
(gdb) info frame                  # details of current frame
(gdb) up                          # move one frame toward caller
(gdb) down                        # move one frame toward callee
```

### Watchpoints — break on data change

```
(gdb) watch x                     # break when x is written
(gdb) rwatch x                    # break when x is read
(gdb) awatch x                    # break on read or write
(gdb) info watchpoints
```

Watchpoints are invaluable for tracking down silent memory corruption —
set one on the variable that ends up with the wrong value.

### Signals

```
(gdb) info signals                # list all signals and GDB's handling
(gdb) handle SIGINT stop          # stop when SIGINT arrives
(gdb) handle SIGPIPE nostop pass  # ignore SIGPIPE, pass to program
```

### Investigating a crash (SIGSEGV)

When a program crashes under GDB it stops at the faulting instruction:

```
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x000000000010916b in main () at heap.c:23

(gdb) backtrace           # see the call stack at crash point
(gdb) info locals         # check local variable values
(gdb) print ptr           # is this NULL or dangling?
(gdb) x/4x ptr            # what is actually at that address?
```

### Core dumps

A core dump is a snapshot of memory at crash time. Enable and use them:

```bash
ulimit -c unlimited               # allow core dumps in this shell
./myprog                          # run until it crashes → writes core
gdb ./myprog core                 # load the core dump
```

Inside GDB, `backtrace` immediately shows the crash location without
needing to reproduce the bug.

### Threads

```
(gdb) info threads                # list all threads
(gdb) thread 2                    # switch to thread #2
(gdb) thread apply all bt         # backtrace every thread at once
(gdb) set scheduler-locking on    # only step the current thread
```

### Pretty-printing structs and pointers

GDB prints structs automatically. For linked data structures:

```
(gdb) print *node                 # one node
(gdb) print *node->next           # follow the link
(gdb) print *(int *)p->data       # cast void* data field
```

### Useful settings

Put these in `~/.gdbinit` for every session:

```
set print pretty on               # indent nested structs
set print array on                # one element per line for arrays
set print array-indexes on        # show array index before each element
set pagination off                # don't pause on long output
set history save on               # persist command history
```

### TUI — text-mode source viewer

```bash
gdb -tui ./myprog                 # start with TUI
```

Inside GDB:

```
(gdb) layout src                  # source + command window
(gdb) layout asm                  # disassembly window
(gdb) layout regs                 # registers + source
(gdb) focus cmd                   # direct keyboard input to command window
Ctrl-X Ctrl-A                     # toggle TUI on/off
```

### Common workflow

```bash
# 1. Build with debug symbols
make

# 2. Run under GDB
gdb ./exercises/08_memory_management/memory_management

# 3. Set a breakpoint and run
(gdb) break main
(gdb) run

# 4. Step through and inspect
(gdb) next
(gdb) print ptr
(gdb) step

# 5. Let it crash, then examine
(gdb) continue
(gdb) backtrace
(gdb) info locals
```

### Quick reference card

| Command | Short | What it does |
|---------|-------|-------------|
| `run` | `r` | Start program |
| `continue` | `c` | Resume to next breakpoint |
| `next` | `n` | Step over one line |
| `step` | `s` | Step into function |
| `finish` | | Run to end of current function |
| `break loc` | `b` | Set breakpoint at location |
| `delete N` | `d N` | Delete breakpoint N |
| `print expr` | `p` | Print expression |
| `display expr` | | Print expr after every step |
| `backtrace` | `bt` | Show call stack |
| `frame N` | `f N` | Switch to stack frame N |
| `info locals` | | Show local variables |
| `watch expr` | | Break on write to expr |
| `quit` | `q` | Exit GDB |

---

## Appendix D: Inspecting Binaries — nm, objdump, readelf

Three complementary tools let you examine what the linker and compiler
produced without running the program.

| Tool | Purpose |
|------|---------|
| `nm` | List symbols (functions, globals) defined or referenced |
| `objdump` | Disassemble, show sections, headers, relocation entries |
| `readelf` | Parse ELF structure: headers, sections, dynamic info |

### nm — symbol table

```bash
nm myprog                         # all symbols
nm -g myprog                      # global (exported) symbols only
nm -u myprog                      # undefined (imported) symbols only
nm -n myprog                      # sort by address
nm -S myprog                      # show symbol size
nm -C myprog                      # demangle C++ names (useful with g++)
nm --defined-only myprog          # skip undefined references
nm libmylib.a                     # list symbols in a static library
```

Each line shows: `address  type  name`

**Symbol type letters** (uppercase = global, lowercase = local/static):

| Letter | Meaning |
|--------|---------|
| `T` / `t` | Code (`.text` section) — a function |
| `D` / `d` | Initialised data (`.data`) — a non-zero global |
| `B` / `b` | Uninitialised data (`.bss`) — a zero-initialised global |
| `R` / `r` | Read-only data (`.rodata`) — a string literal or `const` global |
| `U` | Undefined — referenced but defined elsewhere (in a library) |
| `W` | Weak symbol — overridable by a strong definition |
| `C` | Common — uninitialised global not yet allocated |

**Example — find where a function is defined:**

```bash
nm -g exercises/15_binary_search_tree/bst | grep insert
# 0000000000001234 T bst_insert
```

**Example — check a library exposes the right interface:**

```bash
nm -g --defined-only libvec2.a
# 0000000000000000 T vec2_add
# 0000000000000020 T vec2_len
```

**Example — find undefined symbols (what the linker needs to resolve):**

```bash
nm -u myprog
# U malloc
# U printf
# U pthread_create
```

### objdump — disassembly and section info

```bash
objdump -d myprog                 # disassemble all code sections
objdump -d -M intel myprog        # Intel syntax (vs AT&T default)
objdump -D myprog                 # disassemble all sections (incl. data)
objdump -S myprog                 # interleave source with disassembly (needs -g)
objdump -t myprog                 # symbol table (like nm)
objdump -r myprog.o               # relocation entries
objdump -h myprog                 # section headers
objdump -p myprog                 # program/file headers, dynamic info
objdump -s -j .rodata myprog      # hex dump of .rodata section
```

**Example — find a function and read its assembly:**

```bash
objdump -d myprog | grep -A 30 "<bst_insert>:"
```

**Example — see string literals in the binary:**

```bash
objdump -s -j .rodata myprog
```

**Example — interleaved source + asm (requires `-g` at compile time):**

```bash
objdump -S exercises/06_pointers/pointers | head -60
```

### readelf — ELF file structure

```bash
readelf -h myprog                 # ELF file header (architecture, entry point)
readelf -S myprog                 # section headers
readelf -s myprog                 # symbol table
readelf -d myprog                 # dynamic section (shared lib dependencies)
readelf -l myprog                 # program headers (segments)
readelf -r myprog.o               # relocation entries
readelf -a myprog                 # all of the above
```

**Example — check which shared libraries a binary needs at runtime:**

```bash
readelf -d myprog | grep NEEDED
#  0x0000000000000001 (NEEDED) Shared library: [libm.so.6]
#  0x0000000000000001 (NEEDED) Shared library: [libc.so.6]
```

**Example — verify a binary's architecture and entry point:**

```bash
readelf -h myprog | grep -E "Class|Machine|Entry"
#   Class:                             ELF64
#   Machine:                           Advanced Micro Devices X86-64
#   Entry point address:               0x1060
```

**Example — list all sections and their sizes:**

```bash
readelf -S myprog | grep -E "^\s+\[|Name|Size"
```

### size — section size summary

A quick way to see how much space each section occupies:

```bash
size myprog
#    text    data     bss     dec     hex filename
#    3421     616      48    4085     ff5 myprog
```

| Section | Contains |
|---------|---------|
| `text` | Machine code (compiled functions) |
| `data` | Initialised global and static variables |
| `bss` | Zero-initialised globals (no space in file, allocated at load) |

### strings — find human-readable text

```bash
strings myprog                    # all printable strings (≥4 chars)
strings -n 8 myprog               # only strings ≥8 chars
strings -t x myprog               # show file offset in hex
```

Useful for spotting hard-coded paths, version strings, format strings,
and error messages in a binary you didn't write.

### Practical workflows

**What symbols does my object file export?**
```bash
gcc -Wall -std=c11 -c mylib.c -o mylib.o
nm -g mylib.o
```

**Why is the linker complaining about an undefined reference?**
```bash
nm -u myprog          # shows what's missing
nm -g libfoo.a        # check the library actually defines it
```

**Is a `static` function really hidden from the linker?**
```bash
nm mylib.o | grep myfunc
# 0000000000000000 t myfunc     ← lowercase 't' = local, not exported
```

**What is the binary actually doing at address 0x1234?**
```bash
objdump -d myprog | grep -A 20 "1234:"
```

**Which `.so` files does this program load?**
```bash
readelf -d myprog | grep NEEDED
ldd myprog             # resolves the full path of each dependency
```

---

## Appendix E: Creating Libraries

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
