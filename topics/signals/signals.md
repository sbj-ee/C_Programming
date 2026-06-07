# Signal Handling

Requires `#define _POSIX_C_SOURCE 200809L` and `<signal.h>`.
`_POSIX_C_SOURCE 200809L` enables POSIX (Portable Operating System Interface)
extensions to C11 — the standard set of OS-level APIs available on Linux,
macOS, and other Unix-like systems. The value `200809L` pins the 2008 edition.

## What is a signal

A signal is an asynchronous notification delivered to a process by the
kernel or another process. The process can **catch** (handle), **ignore**,
or **block** most signals; a few (`SIGKILL`, `SIGSTOP`) cannot be caught or
ignored.

Common signals:

| Signal | Default action | Typical cause |
|--------|---------------|---------------|
| `SIGINT` | Terminate | Ctrl-C |
| `SIGTERM` | Terminate | `kill pid` |
| `SIGKILL` | Terminate (uncatchable) | `kill -9 pid` |
| `SIGSEGV` | Core dump | Null / invalid pointer dereference |
| `SIGFPE` | Core dump | Division by zero, integer overflow |
| `SIGALRM` | Terminate | `alarm()` timer expired |
| `SIGCHLD` | Ignore | Child process stopped or exited |
| `SIGUSR1` | Terminate | Application-defined |
| `SIGUSR2` | Terminate | Application-defined |
| `SIGPIPE` | Terminate | Write to a closed pipe or socket |

## signal() — basic handler registration

```c
#include <signal.h>

static void handler(int sig) {
    /* keep it short — see async-signal safety below */
    write(STDOUT_FILENO, "caught!\n", 8);
}

signal(SIGINT, handler);   /* register handler */
signal(SIGINT, SIG_DFL);   /* restore default action */
signal(SIGINT, SIG_IGN);   /* ignore the signal */
```

`signal()` is simple but has unspecified behaviour on some platforms.
Prefer `sigaction()` for production code.

## sigaction() — reliable handler registration

```c
#include <signal.h>

static void handler(int sig, siginfo_t *info, void *ctx) {
    (void)ctx;
    /* info->si_pid: sender PID (when available) */
}

struct sigaction sa = {0};
sa.sa_sigaction = handler;
sa.sa_flags     = SA_SIGINFO   /* pass siginfo_t to handler  */
                | SA_RESTART;  /* restart syscalls on return */
sigemptyset(&sa.sa_mask);
sigaddset(&sa.sa_mask, SIGUSR2);  /* block SIGUSR2 during handler */

if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("sigaction");
}
```

Key `sa_flags`:

| Flag | Effect |
|------|--------|
| `SA_RESTART` | Automatically restart interrupted system calls |
| `SA_SIGINFO` | Handler receives `siginfo_t` with sender info |
| `SA_NODEFER` | Don't block the signal while the handler runs |
| `SA_RESETHAND` | Restore default action after first delivery |

## sig_atomic_t — safe flag from a handler

The only type guaranteed safe to read/write from a signal handler is
`volatile sig_atomic_t`:

```c
#include <signal.h>

static volatile sig_atomic_t g_stop = 0;

static void handle_sigint(int sig) {
    (void)sig;
    g_stop = 1;
}

/* main loop */
while (!g_stop) {
    /* do work */
}
```

## Blocking signals — sigprocmask()

Block a set of signals so they are held pending until unblocked:

```c
#include <signal.h>

sigset_t mask, old_mask;
sigemptyset(&mask);
sigaddset(&mask, SIGINT);
sigaddset(&mask, SIGTERM);

/* block — signals accumulate as pending */
sigprocmask(SIG_BLOCK, &mask, &old_mask);

/* critical section: signals won't interrupt here */

/* unblock — pending signals are delivered now */
sigprocmask(SIG_SETMASK, &old_mask, NULL);
```

`SIG_BLOCK` adds to current mask; `SIG_UNBLOCK` removes; `SIG_SETMASK`
replaces entirely.

## SIGALRM — timer signals

```c
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t g_alarm = 0;

static void on_alarm(int sig) { (void)sig; g_alarm = 1; }

signal(SIGALRM, on_alarm);
alarm(5);           /* deliver SIGALRM in 5 seconds */

pause();            /* sleep until any signal arrives */
/* or use sigsuspend() to atomically unblock + sleep */

if (g_alarm) puts("timed out");

alarm(0);           /* cancel a pending alarm */
```

## Sending signals

```c
#include <signal.h>
#include <unistd.h>

kill(pid,  SIGTERM);   /* send to specific process */
kill(0,    SIGINT);    /* send to own process group */
kill(-pgid, SIGTERM);  /* send to process group pgid */
raise(SIGUSR1);        /* send to self */
```

## Async-signal safety

A signal handler can interrupt the program at any point — including inside
`malloc` or `printf`. Calling non-async-signal-safe functions from a
handler causes undefined behaviour.

**Safe in handlers:** `write`, `read`, `_exit`, `kill`, `signal`,
`sigaction`, `alarm`, arithmetic on `sig_atomic_t`.

**Not safe:** `printf`, `malloc`, `free`, `fflush`, `exit`, `strerror`,
`errno` (safe to read, but many library functions modify it — save and
restore it):

```c
static void handler(int sig) {
    int saved_errno = errno;
    write(STDOUT_FILENO, ".\n", 2);   /* safe */
    errno = saved_errno;
}
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Calling `printf` / `malloc` from a handler | Use only async-signal-safe functions |
| Using `int` instead of `sig_atomic_t` for flags | Declare flag `volatile sig_atomic_t` |
| Forgetting `SA_RESTART` | Syscalls return `EINTR`; wrap with `EINTR` retry loop or use `SA_RESTART` |
| Not masking the signal during its own handler | Set `sa.sa_mask` or use `SA_NODEFER` intentionally |
| Relying on `signal()` portability | Use `sigaction()` instead |
| Signal lost between test and `pause()` | Use `sigsuspend()` to atomically unblock + wait |
