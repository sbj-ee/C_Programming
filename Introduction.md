# Introduction to C Programming

## Development Environment

These exercises are written for **Linux** with **GCC** and target the **C11 standard**.

### Compiler

```bash
gcc --version       # should be GCC 10+ for full C11 support
```

All code is compiled with:

```
-Wall -Wextra -Wpedantic -std=c11 -g
```

| Flag | Purpose |
|------|---------|
| `-Wall` | Enable the most commonly useful warnings |
| `-Wextra` | Enable additional warnings beyond `-Wall` |
| `-Wpedantic` | Enforce strict ISO C11 — no GCC extensions |
| `-std=c11` | Compile as C11 (2011 ISO standard) |
| `-g` | Include debug symbols for use with GDB and Valgrind |

`-Wpedantic` catches things like binary literals (`0b1010`), which are a GCC
extension not in the standard, and forces habits that work across all compilers.

### Tools

| Tool | Install | Purpose |
|------|---------|---------|
| `gcc` | `apt install gcc` | Compiler |
| `make` | `apt install make` | Build system |
| `gdb` | `apt install gdb` | Debugger |
| `valgrind` | `apt install valgrind` | Memory error and leak detector |

### Building

```bash
# Build everything
make

# Build one exercise
make exercises/03_control_flow/control_flow

# Check for memory leaks across all exercises
make valgrind

# Remove compiled binaries
make clean
```

Multi-file exercises (19, 24) have their own `Makefile` and are built
automatically via sub-make.

### GDB quick reference

```bash
gdb exercises/15_binary_search_tree/bst
(gdb) break main          # set a breakpoint
(gdb) run                 # start the program
(gdb) next                # step over one line
(gdb) step                # step into a function
(gdb) print ptr           # print a variable or expression
(gdb) backtrace           # show the call stack
(gdb) quit
```

### Valgrind quick reference

```bash
valgrind --leak-check=full ./myprog
```

Valgrind wraps every memory operation and reports:
- **Invalid reads/writes** — accessing memory out of bounds or after `free`
- **Uninitialised values** — using a variable before assigning it
- **Memory leaks** — heap blocks not freed before exit
- **Double frees** — calling `free` on the same pointer twice

---

## How the Exercises Are Structured

The 24 exercises form four progressive tiers. Each one introduces new concepts
and builds on the ones before it.

### Tier 1 — Language Fundamentals (01–09)

Learn to read and write correct C. No data structures, no dynamic memory.

| Exercise | Topic | What changes from the previous |
|----------|-------|-------------------------------|
| 01 | Hello World | First program: `main`, `printf`, return |
| 02 | Variables & Types | `int`, `float`, `char`, `sizeof`, casting |
| 03 | Control Flow | `if/else`, `switch`, `for`, `while`, `do-while` |
| 04 | Functions | Declaration vs definition, scope, recursion |
| 05 | Arrays & Strings | Fixed arrays, `char[]`, `strlen`, `strcpy` |
| 06 | Pointers | Address-of (`&`), dereference (`*`), arithmetic |
| 07 | Structs | `struct`, `typedef`, nested structs, dot vs arrow |
| 08 | Memory Management | `malloc`/`calloc`/`realloc`/`free`, heap vs stack |
| 09 | File I/O | `fopen`/`fclose`, `fprintf`/`fscanf`, binary mode |

By the end of Tier 1 you can write any sequential program in C.

### Tier 2 — Data Structures (10–18)

Apply pointers and dynamic memory to build classic data structures from scratch.

| Exercise | Topic | What changes from the previous |
|----------|-------|-------------------------------|
| 10 | Linked Lists | Heap-linked nodes, insert/delete/traverse |
| 11 | Function Pointers | Callbacks, dispatch tables, `qsort`, returning functions |
| 12 | Enums & Unions | Bit-flag enums, tagged unions, variant types |
| 13 | Bit Manipulation | Set/clear/toggle/test bits, popcount, bitfields |
| 14 | Preprocessor | Macros, `#`, `##`, variadic macros, include guards |
| 15 | Binary Search Tree | Recursive insert/search/delete, traversals, height |
| 16 | Stack & Queue | LIFO/FIFO on linked lists, bracket balance, BFS |
| 17 | Sorting | Bubble → merge → quicksort, stdlib `qsort`, benchmarks |
| 18 | Hash Table | djb2, separate chaining, put/get/delete, rehashing |

Each data structure exercise builds the full implementation — no standard library
shortcuts — so you understand what is happening at every pointer dereference.

### Tier 3 — Program Structure (19–22)

Move from single-file programs to real-world C organisation patterns.

| Exercise | Topic | What changes from the previous |
|----------|-------|-------------------------------|
| 19 | Multi-file Programs | Headers, include guards, `static`/`extern`, separate compilation |
| 20 | Error Handling | Return codes, `errno`/`strerror`, `goto` cleanup, `setjmp`/`longjmp` |
| 21 | Generics via `void *` | Type erasure, `size_t` element size, comparator function pointers, `qsort`/`bsearch` |
| 22 | Variadic Functions | `va_list`/`va_start`/`va_arg`/`va_end`, `va_copy`, `vsnprintf`, `_Generic` |

These topics explain *how* the C standard library itself is written.
After this tier you can read and understand most real C library source code.

### Tier 4 — Systems Programming (23–24)

Interact with the operating system: signals, processes, and threads.

| Exercise | Topic | What changes from the previous |
|----------|-------|-------------------------------|
| 23 | Signal Handling | `signal`/`sigaction`, `sig_atomic_t`, `sigprocmask`, `SIGALRM`, async-signal safety |
| 24 | POSIX Threads | `pthread_create`/`join`, race conditions, mutex, condition variables, thread attributes |
| 25 | Process Control | `fork`, `waitpid`, `W*` macros, `execvp`, pipes, two-way IPC, `popen`, zombies, `FD_CLOEXEC` |
| 26 | Sockets | `socket`/`bind`/`listen`/`accept`/`connect`, TCP loopback, UDP datagrams, AF_UNIX, `getaddrinfo`, `SO_REUSEADDR`, `SIGPIPE` |

These exercises require POSIX (`_POSIX_C_SOURCE 200809L`). Exercise 24 links with
`-pthread` and uses its own `Makefile`.

---

## What Makes C Different

Understanding these three things separates C from higher-level languages.

### Manual memory management

There is no garbage collector. Every `malloc` must be paired with a `free`.
Valgrind tells you when you get this wrong.

```
heap-allocated memory:   you call malloc → you call free
stack memory:            automatic — gone when the function returns
static memory:           lives for the whole program (global/static variables)
```

Exercise 08 covers this in depth. Exercises 10–18 apply it.

### Pointers

A pointer is just a memory address stored in a variable. Dereferencing reads the
value at that address. Getting this wrong gives you a segfault or silent
corruption — C does not bounds-check for you.

```c
int  x  = 42;
int *p  = &x;    // p holds the address of x
int  y  = *p;    // dereference: read the value at that address
*p = 99;         // write through the pointer — changes x
```

Exercise 06 covers pointers. Exercise 08 covers heap pointers. Exercise 21
covers type-erased (`void *`) pointers.

### Undefined behaviour

C allows the compiler to assume you never trigger undefined behaviour (UB).
If you do, the compiler may generate any code it likes — the program may crash,
produce wrong answers, or appear to work. Common sources:

- Accessing memory out of bounds
- Reading an uninitialised variable
- Signed integer overflow
- Data races (two threads, one write, no synchronisation)
- Dereferencing a NULL or freed pointer

`-Wall -Wextra -Wpedantic` and Valgrind catch many of these. `-fsanitize=address,undefined`
(AddressSanitizer + UBSan) catches almost all of them at runtime:

```bash
gcc -Wall -Wextra -std=c11 -g -fsanitize=address,undefined prog.c -o prog
./prog
```

---

## C11 Portability Notes

Some things that look standard are actually extensions or POSIX:

| Feature | Status | Exercise workaround |
|---------|--------|---------------------|
| `M_PI` | Not in C11; POSIX only | `#define M_PI 3.14159265358979323846` |
| `strdup` | POSIX, not C11 | Manual: `malloc(strlen(s)+1)` + `memcpy` |
| `0b1010` binary literals | GCC extension | Use hex: `0x0A` |
| `##__VA_ARGS__` | GCC extension | Ensure at least one fixed arg |
| `sigaction`, `alarm`, `SIGUSR1` | POSIX | `#define _POSIX_C_SOURCE 200809L` |
| `pthread_*` | POSIX | `-pthread` compiler flag |
