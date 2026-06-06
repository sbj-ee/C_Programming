# C Programming

A structured C programming learning project covering fundamentals through data structures.

## Layout

```
C_Programming/
├── exercises/      # Progressive programs, each building on the last
│   ├── 01_hello_world/
│   ├── 02_variables_and_types/
│   ├── 03_control_flow/
│   ├── 04_functions/
│   ├── 05_arrays_and_strings/
│   ├── 06_pointers/
│   ├── 07_structs/
│   ├── 08_memory_management/
│   ├── 09_file_io/
│   └── 10_linked_lists/
└── topics/         # Reference snippets by concept
    ├── pointers/
    ├── memory/
    ├── strings/
    ├── structs/
    ├── file_io/
    └── preprocessor/
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
