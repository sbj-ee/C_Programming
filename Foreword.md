# Foreword

## Standing on the Shoulders of Giants

C did not arrive fully formed. It grew — pragmatically, organically — out of the
needs of a small team trying to write a portable operating system in the early 1970s.
That operating system was UNIX. The language and the system shaped each other so
thoroughly that understanding one illuminates the other.

---

## The Language and the Book

In 1978, Brian W. Kernighan and Dennis M. Ritchie published
*The C Programming Language*. It is one of the most influential technical books
ever written: concise where most textbooks are verbose, precise where most are
vague, and honest about what C is — a small language that trusts the programmer
completely and punishes carelessness without mercy.

The second edition (1988) updated the text for ANSI C and remains the reference
against which every C programmer measures themselves. The exercises here owe their
spirit to K&R: learn by doing, understand what the machine is actually doing, and
treat simplicity as a virtue rather than a limitation.

> *"C is not a big language, and it is not well served by a big book."*
> — Kernighan & Ritchie, Preface to the First Edition

---

## UNIX: The Soil C Grew In

Dennis Ritchie developed C specifically to rewrite the UNIX kernel — first on
a PDP-11, later on hardware far beyond anything Bell Labs imagined in 1969.
The design decision to express the operating system itself in a high-level
language was radical. It gave UNIX portability that assembly-language operating
systems could never have, and it gave C a proving ground where every abstraction
had to justify its cost.

The UNIX philosophy — small tools that do one thing well, connected by pipes,
built on a uniform file abstraction — is baked into C's standard library.
`fopen`, `read`, `write`, `fork`, `exec`: these are not arbitrary API choices.
They are the distillation of decades of thinking about how programs should
communicate and compose.

The exercises in Tier 4 (signals, threads, processes, sockets, mmap) are not
advanced C topics in an abstract sense. They are the vocabulary of UNIX
programming — the interface between a C program and the kernel it lives inside.

---

## Linux: The Torch Carried Forward

In 1991, Linus Torvalds announced a "hobby" project: a free UNIX-like kernel
for the 386. Linux is written in C. Its coding style, its data structures, its
approach to concurrency and memory — all of it is C, shaped by the same pressures
that shaped UNIX three decades earlier.

Linux turned C from the language of a proprietary research operating system into
the lingua franca of systems programming worldwide. Today it runs on phones,
servers, routers, satellites, and supercomputers. Every exercise in this project
compiles and runs on Linux, using a toolchain — GCC, GNU Make, glibc — built by
thousands of contributors over forty years, all writing C.

The POSIX standard that governs exercises 23 through 27 (`_POSIX_C_SOURCE 200809L`)
exists because the UNIX tradition needed a portable specification. Linux implements
it. The calls you write — `fork`, `mmap`, `pthread_create`, `socket` — are the
same ones Linus reads in the kernel that services them.

---

## What This Project Is

These 27 exercises are a path from `printf("hello\n")` to shared memory across
processes — from the first thing Kernighan and Ritchie taught to the mechanisms
that make modern software possible. They are written in C11, the most recent
stable standard, compiled with warnings that K&R's compilers could not have
imagined.

The topic reference sheets alongside each exercise exist because C rewards readers
who pause to understand, not just coders who copy and paste. The pitfall tables
document real mistakes — the kind that took real programs down in production
before anyone thought to write them down.

C is fifty years old. It is not going anywhere. Learn it well.

---

*"UNIX is simple. It just takes a genius to understand its simplicity."*
— Dennis M. Ritchie
