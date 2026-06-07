/*
 * Exercise 24: POSIX Threads (pthreads)
 *
 * A thread is an independent execution context within a process.
 * All threads share the same address space, file descriptors, and
 * signal dispositions — which makes coordination essential.
 *
 * Topics:
 *   - pthread_create / pthread_join / pthread_t
 *   - Passing arguments and returning values through void *
 *   - Race conditions and why they happen
 *   - Mutex (pthread_mutex_t) — exclusive access
 *   - Condition variables (pthread_cond_t) — event notification
 *   - Thread attributes — detached threads, stack size
 *   - Pitfalls: deadlock, spurious wakeups, lock ordering
 *
 * Compile: gcc -Wall -Wextra -std=c11 -g -pthread threads.c -o threads
 * The -pthread flag adds -lpthread AND -D_REENTRANT for thread-safe libc.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>    /* sysconf */
#include <stdint.h>
#include <stdatomic.h>

/* ──────────────────────────────────────────────────────────
 * Section 1: Thread basics
 *
 * pthread_create(tid, attrs, start_fn, arg)
 *   - start_fn has signature: void *fn(void *arg)
 *   - arg is passed as a void* — use a heap struct for
 *     multiple values (never pass a stack address that may
 *     go away before the thread reads it)
 *
 * pthread_join(tid, &retval)
 *   - blocks until the thread exits
 *   - retrieves the value returned by start_fn
 *   - each joinable thread must be joined exactly once
 * ────────────────────────────────────────────────────────── */

typedef struct { int id; int n; } WorkArgs;
typedef struct { int id; long sum; } WorkResult;

static void *worker(void *arg) {
    WorkArgs *a = arg;
    long sum = 0;
    for (int i = 1; i <= a->n; i++) sum += i;

    WorkResult *res = malloc(sizeof *res);
    res->id  = a->id;
    res->sum = sum;
    free(a);
    return res;   /* caller must free */
}

static void section_basics(void) {
    printf("=== 1. Thread basics ===\n");

    const int NTHREADS = 4;
    pthread_t tids[4];

    /* Spawn threads with heap-allocated args */
    for (int i = 0; i < NTHREADS; i++) {
        WorkArgs *a = malloc(sizeof *a);
        a->id = i;
        a->n  = (i + 1) * 10;   /* each thread sums 1..n */
        pthread_create(&tids[i], NULL, worker, a);
    }

    /* Join each thread and collect results */
    for (int i = 0; i < NTHREADS; i++) {
        WorkResult *res;
        pthread_join(tids[i], (void **)&res);
        printf("  thread %d: sum(1..%d) = %ld\n", res->id, (res->id+1)*10, res->sum);
        free(res);
    }

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: Race condition
 *
 * A race condition occurs when two threads access shared state
 * concurrently and at least one write is involved.
 *
 *   g_count++ compiles to three instructions:
 *     LOAD  reg, g_count
 *     ADD   reg, 1
 *     STORE g_count, reg
 *
 * If thread A loads, then thread B loads (same value), then
 * both store, one increment is lost.  With N threads and
 * ITERS increments each, the result is usually less than N*ITERS.
 *
 * volatile prevents the compiler from caching g_count in a
 * register, making the individual load/store visible — but it
 * does NOT make the read-modify-write atomic.
 * ────────────────────────────────────────────────────────── */

#define NTHREADS 4
#define ITERS    500000L

static volatile long g_unsafe = 0;

static void *unsafe_inc(void *arg) {
    (void)arg;
    for (long i = 0; i < ITERS; i++) g_unsafe++;
    return NULL;
}

static void section_race(void) {
    printf("=== 2. Race condition ===\n");

    pthread_t t[NTHREADS];
    for (int i = 0; i < NTHREADS; i++) pthread_create(&t[i], NULL, unsafe_inc, NULL);
    for (int i = 0; i < NTHREADS; i++) pthread_join(t[i], NULL);

    long expected = NTHREADS * ITERS;
    printf("  expected: %ld\n", expected);
    printf("  actual:   %ld\n", g_unsafe);
    if (g_unsafe == expected)
        printf("  (got lucky — no race observed; try again or add more threads)\n");
    else
        printf("  lost %ld increments to the race\n", expected - g_unsafe);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: Mutex (mutual exclusion)
 *
 * A mutex allows only one thread to hold it at a time.
 * Every access to shared state is guarded by lock/unlock.
 *
 * PTHREAD_MUTEX_INITIALIZER — static initialisation (no destroy needed)
 * pthread_mutex_init / pthread_mutex_destroy — dynamic initialisation
 *
 * Invariant: always unlock on every exit path (including errors).
 * A thread that holds a mutex and exits without unlocking
 * leaves it permanently locked — deadlock for all other threads.
 * ────────────────────────────────────────────────────────── */

static long g_safe = 0;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *safe_inc(void *arg) {
    (void)arg;
    for (long i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&g_mutex);
        g_safe++;
        pthread_mutex_unlock(&g_mutex);
    }
    return NULL;
}

static void section_mutex(void) {
    printf("=== 3. Mutex ===\n");

    pthread_t t[NTHREADS];
    for (int i = 0; i < NTHREADS; i++) pthread_create(&t[i], NULL, safe_inc, NULL);
    for (int i = 0; i < NTHREADS; i++) pthread_join(t[i], NULL);

    long expected = NTHREADS * ITERS;
    printf("  expected: %ld\n", expected);
    printf("  actual:   %ld\n", g_safe);
    printf("  correct:  %s\n", g_safe == expected ? "yes" : "NO — bug!");

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: Condition variable — bounded producer/consumer
 *
 * A condition variable lets a thread sleep until some condition
 * becomes true, without busy-waiting.
 *
 * pthread_cond_wait(cond, mutex)
 *   1. Atomically releases mutex and blocks
 *   2. When signalled, reacquires mutex before returning
 *
 * CRITICAL: always recheck the condition in a while loop, not
 * an if.  Spurious wakeups are allowed by POSIX — a thread can
 * wake up without being signalled.
 *
 *   while (condition_not_met)       // ← while, not if
 *       pthread_cond_wait(&cond, &mu);
 *
 * pthread_cond_signal   — wake one waiter
 * pthread_cond_broadcast — wake all waiters (use when state
 *                          change could unblock multiple threads)
 * ────────────────────────────────────────────────────────── */

#define QCAP   8
#define NITEMS 20

typedef struct {
    int             data[QCAP];
    int             head, tail, count;
    int             closed;            /* producer signals no more items */
    pthread_mutex_t mu;
    pthread_cond_t  not_full;
    pthread_cond_t  not_empty;
} BQueue;

static BQueue bq = {
    .mu       = PTHREAD_MUTEX_INITIALIZER,
    .not_full  = PTHREAD_COND_INITIALIZER,
    .not_empty = PTHREAD_COND_INITIALIZER,
};

static void bq_push(BQueue *q, int val) {
    pthread_mutex_lock(&q->mu);
    while (q->count == QCAP)
        pthread_cond_wait(&q->not_full, &q->mu);
    q->data[q->tail] = val;
    q->tail = (q->tail + 1) % QCAP;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mu);
}

/* Returns 1 with *out set, or 0 if queue is closed and empty */
static int bq_pop(BQueue *q, int *out) {
    pthread_mutex_lock(&q->mu);
    while (q->count == 0 && !q->closed)
        pthread_cond_wait(&q->not_empty, &q->mu);
    if (q->count == 0) { pthread_mutex_unlock(&q->mu); return 0; }
    *out = q->data[q->head];
    q->head = (q->head + 1) % QCAP;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mu);
    return 1;
}

static void bq_close(BQueue *q) {
    pthread_mutex_lock(&q->mu);
    q->closed = 1;
    pthread_cond_broadcast(&q->not_empty);  /* wake all blocked consumers */
    pthread_mutex_unlock(&q->mu);
}

static void *producer(void *arg) {
    BQueue *q = arg;
    for (int i = 0; i < NITEMS; i++) {
        bq_push(q, i);
        printf("  P: pushed %2d  (queue size %d)\n", i, q->count);  /* display-only race; count may be stale */
    }
    bq_close(q);
    return NULL;
}

static void *consumer(void *arg) {
    BQueue *q = arg;
    int sum = 0, n = 0, val;
    while (bq_pop(q, &val)) { sum += val; n++; }
    printf("  C: consumed %d items, sum=%d\n", n, sum);
    return NULL;
}

static void section_condvar(void) {
    printf("=== 4. Condition variable: producer/consumer ===\n");

    /* Reset queue state */
    bq.head = bq.tail = bq.count = bq.closed = 0;

    pthread_t prod, cons1, cons2;
    pthread_create(&cons1, NULL, consumer, &bq);
    pthread_create(&cons2, NULL, consumer, &bq);
    pthread_create(&prod,  NULL, producer, &bq);

    pthread_join(prod,  NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    /* Expected: two consumers split NITEMS items, sum = NITEMS*(NITEMS-1)/2 */
    printf("  expected sum across consumers: %d\n", NITEMS * (NITEMS - 1) / 2);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: Thread attributes
 *
 * pthread_attr_t controls how a thread is created:
 *   - Detach state: joinable (default) or detached
 *   - Stack size and guard size
 *   - Scheduling policy and priority (requires privilege)
 *
 * Detached thread:
 *   - Resources are released automatically when it exits
 *   - Cannot be joined — calling pthread_join on it is UB
 *   - Use when you never need the return value and fire-and-forget
 * ────────────────────────────────────────────────────────── */

static atomic_int g_detached_done = 0;   /* volatile alone is not a sync mechanism in C11 */

static void *detached_fn(void *arg) {
    int n = *(int *)arg;
    free(arg);
    printf("  fire-and-forget thread: n=%d, done\n", n);
    g_detached_done = 1;
    return NULL;
}

static void *stacksz_fn(void *arg) {
    int n = *(int *)arg;
    free(arg);
    printf("  custom-stack thread: n=%d, done\n", n);
    return NULL;
}

static void section_attrs(void) {
    printf("=== 5. Thread attributes ===\n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    /* --- Default stack size --- */
    size_t stacksz;
    pthread_attr_getstacksize(&attr, &stacksz);
    printf("  default stack size: %zu KB\n", stacksz / 1024);

    /* --- Detached thread ---
     * Resources freed automatically on exit; cannot be joined. */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int *n = malloc(sizeof *n);
    *n = 99;
    pthread_t tid;
    pthread_create(&tid, &attr, detached_fn, n);
    while (!g_detached_done) { /* spin — only safe because it's volatile */ }
    printf("  detached thread finished\n");

    pthread_attr_destroy(&attr);

    /* --- Custom stack size (joinable) --- */
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);   /* 64 KB */
    pthread_attr_getstacksize(&attr, &stacksz);
    printf("  custom stack size:  %zu KB\n", stacksz / 1024);

    pthread_t t2;
    int *m = malloc(sizeof *m);
    *m = 7;
    /* attr is already joinable by default; no need to set explicitly */
    pthread_create(&t2, &attr, stacksz_fn, m);
    pthread_join(t2, NULL);   /* joinable — use join, not a spin flag */

    pthread_attr_destroy(&attr);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 6: Pitfalls
 *
 * 1. Deadlock — two threads each hold a lock the other needs.
 *    Solution: always acquire locks in the same global order.
 *
 * 2. Spurious wakeups — pthread_cond_wait can return without
 *    a signal; always recheck the condition in a while loop.
 *
 * 3. Forgetting to unlock on error — use goto cleanup or a
 *    scoped lock wrapper; every lock() must have an unlock().
 *
 * 4. Using a destroyed mutex — pthread_mutex_destroy while
 *    another thread holds it is UB.
 *
 * 5. Returning a stack address from a thread function — the
 *    stack is gone when the thread exits.
 * ────────────────────────────────────────────────────────── */

static pthread_mutex_t lock_a = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lock_b = PTHREAD_MUTEX_INITIALIZER;

/* Safe: both threads acquire in the same order (A then B) */
static void *safe_order(void *arg) {
    int id = *(int *)arg;
    pthread_mutex_lock(&lock_a);
    pthread_mutex_lock(&lock_b);
    printf("  thread %d: holds A and B\n", id);
    pthread_mutex_unlock(&lock_b);
    pthread_mutex_unlock(&lock_a);
    return NULL;
}

static void section_pitfalls(void) {
    printf("=== 6. Pitfalls ===\n");

    /* Demonstrate safe lock ordering */
    pthread_t t[2];
    int ids[2] = {0, 1};
    pthread_create(&t[0], NULL, safe_order, &ids[0]);
    pthread_create(&t[1], NULL, safe_order, &ids[1]);
    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);
    printf("  lock ordering OK — no deadlock\n");

    printf("\n  Pitfall summary:\n");
    printf("  DEADLOCK:  thread A holds lock1, waits for lock2;\n");
    printf("             thread B holds lock2, waits for lock1.\n");
    printf("  FIX:       always acquire locks in the same global order.\n");
    printf("\n");
    printf("  SPURIOUS WAKEUP: cond_wait returns without a signal.\n");
    printf("  FIX:       use while(!condition) cond_wait, never if.\n");
    printf("\n");
    printf("  STACK RETURN: returning &local_var from thread fn.\n");
    printf("  FIX:       heap-allocate the result; caller frees it.\n");
    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);

    section_basics();
    section_race();
    section_mutex();
    section_condvar();
    section_attrs();
    section_pitfalls();
    return 0;
}
