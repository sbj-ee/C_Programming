/*
 * Exercise 23: Signal Handling
 *
 * A signal is an asynchronous notification delivered to a process.
 * The OS can deliver one at any point — even in the middle of a
 * library call — so signal handlers run in a restricted context.
 *
 * Topics:
 *   - Common signals and their default dispositions
 *   - signal()    — C standard, simple but limited
 *   - sigaction() — POSIX, precise and portable
 *   - sig_atomic_t — the only type safe to touch from a handler
 *   - SIGALRM / alarm() — implementing timeouts
 *   - Async-signal safety — what you can and cannot do in a handler
 *
 * _POSIX_C_SOURCE enables sigaction, SIGUSR1, SIGALRM, alarm(), etc.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>   /* alarm, write, getpid */
#include <string.h>
#include <errno.h>

/* ──────────────────────────────────────────────────────────
 * Section 1: Signal table
 *
 * Every process starts with a default disposition for each
 * signal: Term (terminate), Core (core dump), Ign (ignore),
 * Stop (pause execution), Cont (resume if stopped).
 * ────────────────────────────────────────────────────────── */

static void section_table(void) {
    printf("=== 1. Common signals ===\n");

    struct { int num; const char *name; const char *action; const char *meaning; } sigs[] = {
        { SIGINT,  "SIGINT",  "Term", "Interrupt (Ctrl-C)"           },
        { SIGTERM, "SIGTERM", "Term", "Graceful termination request"  },
        { SIGQUIT, "SIGQUIT", "Core", "Quit (Ctrl-\\) — core dump"   },
        { SIGKILL, "SIGKILL", "Term", "Unconditional kill (no catch)" },
        { SIGSTOP, "SIGSTOP", "Stop", "Pause process (no catch)"     },
        { SIGCONT, "SIGCONT", "Cont", "Resume a stopped process"     },
        { SIGALRM, "SIGALRM", "Term", "Timer expired (alarm())"      },
        { SIGCHLD, "SIGCHLD", "Ign",  "Child state changed"          },
        { SIGUSR1, "SIGUSR1", "Term", "User-defined signal 1"        },
        { SIGUSR2, "SIGUSR2", "Term", "User-defined signal 2"        },
        { SIGSEGV, "SIGSEGV", "Core", "Invalid memory access"        },
        { SIGFPE,  "SIGFPE",  "Core", "Arithmetic exception"         },
        { SIGPIPE, "SIGPIPE", "Term", "Write to closed pipe"         },
    };

    printf("  %-8s %-10s %-6s %s\n", "Num", "Name", "Dflt", "Meaning");
    printf("  %-8s %-10s %-6s %s\n", "---", "----", "----", "-------");
    for (size_t i = 0; i < sizeof sigs / sizeof *sigs; i++)
        printf("  %-8d %-10s %-6s %s\n",
               sigs[i].num, sigs[i].name, sigs[i].action, sigs[i].meaning);
    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: signal() — C standard handler registration
 *
 * signal(signum, handler) installs handler for signum.
 * Returns the previous handler (or SIG_ERR on failure).
 *
 * Special values:
 *   SIG_DFL — restore default disposition
 *   SIG_IGN — ignore the signal
 *
 * Limitation: the standard does not specify whether the
 * handler is reset to SIG_DFL after firing.  Use sigaction()
 * for reliable behaviour.
 * ────────────────────────────────────────────────────────── */

static void simple_handler(int sig) {
    /* write() is async-signal-safe; printf() is not.
     * For clarity in this demo we use write() in the handler. */
    const char *msg = "  [handler] caught SIGINT\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    (void)sig;
}

static void section_signal(void) {
    printf("=== 2. signal() — C standard ===\n");

    /* Save and replace the handler */
    void (*prev)(int) = signal(SIGINT, simple_handler);

    /* Raise the signal ourselves — no keyboard needed */
    printf("  Raising SIGINT...\n");
    raise(SIGINT);
    printf("  Returned from raise()\n");

    /* Restore the previous handler */
    signal(SIGINT, prev);

    /* Demonstrate SIG_IGN */
    signal(SIGUSR1, SIG_IGN);
    printf("  Raising SIGUSR1 (ignored)...\n");
    raise(SIGUSR1);
    printf("  SIGUSR1 had no effect\n");
    signal(SIGUSR1, SIG_DFL);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: sig_atomic_t — the safe flag idiom
 *
 * A signal can interrupt any instruction.  Reading or writing
 * a multi-byte variable from both a handler and main is a data
 * race — even int is not guaranteed atomic on all platforms.
 *
 * sig_atomic_t is an integer type that can be read and written
 * atomically with respect to signal delivery.
 *
 * volatile tells the compiler not to cache the variable in a
 * register — the handler may change it "from outside" the
 * normal control flow.
 *
 * Pattern:
 *   volatile sig_atomic_t g_flag = 0;
 *   handler: g_flag = 1;
 *   main loop: while (!g_flag) { do_work(); }
 * ────────────────────────────────────────────────────────── */

static volatile sig_atomic_t g_shutdown = 0;
static volatile sig_atomic_t g_usr1     = 0;

static void shutdown_handler(int sig) {
    (void)sig;
    g_shutdown = 1;
}

static void usr1_handler(int sig) {
    (void)sig;
    g_usr1 = 1;
}

static void section_atomic_flag(void) {
    printf("=== 3. sig_atomic_t flag pattern ===\n");

    signal(SIGTERM, shutdown_handler);
    signal(SIGUSR1, usr1_handler);

    /* Simulate work loop responding to signals */
    for (int tick = 0; tick < 5; tick++) {
        if (g_shutdown) { printf("  tick %d: shutdown requested\n", tick); break; }
        if (g_usr1)     { printf("  tick %d: SIGUSR1 received\n",   tick); g_usr1 = 0; }

        printf("  tick %d: working...\n", tick);

        /* Fire signals mid-loop to show the flag is checked */
        if (tick == 2) raise(SIGUSR1);
        if (tick == 3) raise(SIGTERM);
    }

    /* Reset */
    g_shutdown = 0;
    signal(SIGTERM, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: sigaction() — POSIX, precise control
 *
 * sigaction() offers guarantees signal() does not:
 *   - Handler stays installed after firing (no reset to SIG_DFL)
 *   - SA_RESTART: automatically restart syscalls interrupted by
 *     the signal (avoids EINTR on read/write/accept)
 *   - SA_SIGINFO: handler receives extended signal info (sender
 *     PID, signal value, fault address, etc.)
 *
 * struct sigaction {
 *     void     (*sa_handler)(int);           // simple handler
 *     void     (*sa_sigaction)(int,siginfo_t*,void*); // SA_SIGINFO
 *     sigset_t sa_mask;     // signals blocked during handler
 *     int      sa_flags;    // SA_RESTART | SA_SIGINFO | ...
 * };
 * ────────────────────────────────────────────────────────── */

static void sa_simple_handler(int sig) {
    const char *msg = "  [sigaction handler] SIGUSR1 received\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    (void)sig;
}

static void sa_info_handler(int sig, siginfo_t *info, void *ctx) {
    (void)ctx;
    /* async-signal-safe: use write(), not printf() */
    char buf[64];
    int n = snprintf(buf, sizeof buf,
                     "  [SA_SIGINFO] sig=%d from pid=%d\n",
                     sig, (int)info->si_pid);
    write(STDOUT_FILENO, buf, (size_t)n);
}

static void section_sigaction(void) {
    printf("=== 4. sigaction() ===\n");

    struct sigaction sa, old;

    /* --- 4a: simple handler with SA_RESTART --- */
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = sa_simple_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;           /* restart interrupted syscalls */
    sigaction(SIGUSR1, &sa, &old);

    printf("  Raising SIGUSR1 via sigaction handler...\n");
    raise(SIGUSR1);

    /* Restore previous handler */
    sigaction(SIGUSR1, &old, NULL);

    /* --- 4b: SA_SIGINFO — handler receives signal metadata --- */
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = sa_info_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGUSR2, &sa, &old);

    printf("  Raising SIGUSR2 (SA_SIGINFO)...\n");
    raise(SIGUSR2);

    sigaction(SIGUSR2, &old, NULL);

    /* --- 4c: blocking signals during a critical section --- */
    printf("  Blocking SIGUSR1 during critical section...\n");

    /* Install a handler — unblocking a signal with SIG_DFL kills the process */
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = sa_simple_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, &old);

    sigset_t block, prev_mask;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    sigprocmask(SIG_BLOCK, &block, &prev_mask);

    raise(SIGUSR1);   /* delivered but pending, not handled yet */
    printf("  Still in critical section (signal pending)\n");

    /* Restore mask — pending signal is delivered now */
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    printf("  Mask restored\n");

    sigaction(SIGUSR1, &old, NULL);   /* restore previous disposition */

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: SIGALRM / alarm() — timeouts
 *
 * alarm(seconds) schedules SIGALRM to be delivered after n
 * seconds.  Only one alarm is active at a time; calling
 * alarm(0) cancels the pending alarm.
 *
 * Pattern: set a flag in the SIGALRM handler, check it in
 * the work loop to detect and handle the timeout.
 * ────────────────────────────────────────────────────────── */

static volatile sig_atomic_t g_timed_out = 0;

static void alarm_handler(int sig) {
    (void)sig;
    g_timed_out = 1;
}

/* Simulate slow work that may time out */
static int slow_operation(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (g_timed_out) return -1;
        /* simulate one unit of work */
        volatile long dummy = 0;
        for (long j = 0; j < 10000000L; j++) dummy += j;
        (void)dummy;
        printf("  slow_op: iteration %d done\n", i);
    }
    return 0;
}

static void section_alarm(void) {
    printf("=== 5. SIGALRM / alarm() ===\n");

    signal(SIGALRM, alarm_handler);

    /* Run a 5-iteration job with a 2-second timeout */
    g_timed_out = 0;
    alarm(2);
    int rc = slow_operation(5);
    alarm(0);   /* cancel any remaining alarm */

    if (rc == 0)
        printf("  Operation completed within timeout\n");
    else
        printf("  Operation timed out!\n");

    /* Demonstrate cancelling an alarm */
    printf("  Setting alarm for 10s then cancelling it...\n");
    alarm(10);
    unsigned remaining = alarm(0);   /* cancel; returns seconds remaining */
    printf("  Cancelled alarm with ~%u second(s) remaining\n", remaining);

    signal(SIGALRM, SIG_DFL);
    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 6: Async-signal safety
 *
 * A signal can arrive between any two machine instructions,
 * including inside malloc, printf, or any library function
 * that uses a global lock or buffer.
 *
 * If a signal handler calls a non-reentrant function while
 * that same function is executing in main, the result is UB.
 *
 * POSIX defines a list of async-signal-safe functions (man 7
 * signal-safety on Linux).  Key safe ones:
 *   write()  _exit()  getpid()  kill()  signal()  sigaction()
 *   memcpy() strlen() memmove() — technically not on the list
 *   but safe in practice since they don't use global state
 *
 * NOT safe (use global state / locks):
 *   printf() / fprintf()  malloc() / free()  strtok()
 *   fopen() / fclose()    exit()             errno-modifying calls
 * ────────────────────────────────────────────────────────── */

/* async-signal-safe integer-to-string (no malloc, no printf) */
static void write_int(int fd, int n) {
    char buf[32];
    int neg = (n < 0);
    if (neg) n = -n;
    int i = (int)(sizeof buf) - 1;
    buf[i] = '\0';
    do { buf[--i] = '0' + (n % 10); n /= 10; } while (n > 0);
    if (neg) buf[--i] = '-';
    write(fd, buf + i, (size_t)((int)(sizeof buf) - 1 - i)); /* avoid strlen: not on POSIX async-signal-safe list */
}

static volatile sig_atomic_t g_sigusr1_count = 0;

static void safe_handler(int sig) {
    (void)sig;
    g_sigusr1_count++;

    /* Safe: write() to stdout */
    const char *prefix = "  [safe_handler] count=";
    write(STDOUT_FILENO, prefix, strlen(prefix));
    write_int(STDOUT_FILENO, (int)g_sigusr1_count);
    write(STDOUT_FILENO, "\n", 1);
}

static void section_safety(void) {
    printf("=== 6. Async-signal safety ===\n");

    /* Use sigaction() — signal() may reset the handler to SIG_DFL after
     * the first delivery (SYSV one-shot semantics), which would kill the
     * process on the second raise.  sigaction() always persists. */
    struct sigaction sa, old;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = safe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, &old);

    printf("  Firing SIGUSR1 three times:\n");
    raise(SIGUSR1);
    raise(SIGUSR1);
    raise(SIGUSR1);

    printf("  Final count (read from main): %d\n", (int)g_sigusr1_count);

    printf("\n  Rules for signal handlers:\n");
    printf("  DO:   set volatile sig_atomic_t flags\n");
    printf("  DO:   call write(), _exit(), kill(), getpid()\n");
    printf("  DON'T: call printf/malloc/free (not async-signal-safe)\n");
    printf("  DON'T: call exit() — use _exit() (skips atexit/fflush)\n");
    printf("  DON'T: modify errno without saving/restoring it first\n");
    printf("  WHY:  signals interrupt arbitrarily — even inside malloc's\n");
    printf("        internal lock — calling malloc again deadlocks\n");

    sigaction(SIGUSR1, &old, NULL);
    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    /* Make stdout line-buffered so printf output isn't lost if a
     * signal kills the process before the buffer is flushed. */
    setvbuf(stdout, NULL, _IOLBF, 0);

    section_table();
    section_signal();
    section_atomic_flag();
    section_sigaction();
    section_alarm();
    section_safety();
    return 0;
}
