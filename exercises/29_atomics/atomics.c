/*
 * Exercise 29 — C11 Atomics
 *
 * <stdatomic.h> provides atomic types and operations that are safe
 * to use from multiple threads without a mutex.  The compiler and
 * CPU guarantee that each operation appears instantaneous to all threads.
 *
 * Build: make   (links -pthread)
 * Run:   ./atomics
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

/* ── 1. Atomic types and basic operations ────────────────────────────────── */

static void section1_types(void) {
    puts("═══ 1. Atomic Types and Basic Operations ════\n");

    /* _Atomic qualifier works on any scalar type */
    _Atomic int      ai = 0;
    atomic_long      al = 0;        /* typedef: same as _Atomic long */
    _Atomic _Bool    flag = 0;

    /* Load and store */
    atomic_store(&ai, 42);
    int v = atomic_load(&ai);
    printf("  stored 42, loaded %d\n", v);

    /* Arithmetic: returns the OLD value */
    int old = atomic_fetch_add(&ai, 10);
    printf("  fetch_add(10): old=%d, now=%d\n", old, atomic_load(&ai));

    old = atomic_fetch_sub(&ai, 5);
    printf("  fetch_sub(5):  old=%d, now=%d\n", old, atomic_load(&ai));

    /* Bitwise operations */
    atomic_store(&al, 0x0F);
    atomic_fetch_or(&al, 0xF0);
    printf("  fetch_or(0xF0): result=0x%lx\n", atomic_load(&al));

    atomic_fetch_and(&al, 0x33);
    printf("  fetch_and(0x33): result=0x%lx\n", atomic_load(&al));

    /* Boolean toggle */
    atomic_store(&flag, 1);
    printf("  flag=%d\n\n", (int)atomic_load(&flag));

    puts("  Type aliases (from <stdatomic.h>):");
    puts("    atomic_int, atomic_long, atomic_size_t, atomic_bool ...");
    puts("    All equivalent to _Atomic <type>\n");
}

/* ── 2. Race condition vs atomic counter ─────────────────────────────────── */

#define NTHREADS  4
#define ITERS     500000

static long              g_unsafe  = 0;   /* volatile but NOT atomic */
static atomic_long       g_safe    = 0;

static void *unsafe_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++)
        g_unsafe++;     /* read-modify-write: three instructions, not atomic */
    return NULL;
}

static void *safe_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++)
        atomic_fetch_add_explicit(&g_safe, 1, memory_order_relaxed);
    return NULL;
}

static void section2_counter(void) {
    puts("═══ 2. Race Condition vs Atomic Counter ════\n");

    pthread_t tids[NTHREADS];

    /* Unsafe: data race — result is undefined behaviour */
    g_unsafe = 0;
    for (int i = 0; i < NTHREADS; i++)
        pthread_create(&tids[i], NULL, unsafe_worker, NULL);
    for (int i = 0; i < NTHREADS; i++)
        pthread_join(tids[i], NULL);
    printf("  unsafe  (data race): expected %d, got %ld  %s\n",
           NTHREADS * ITERS, g_unsafe,
           g_unsafe == NTHREADS * ITERS ? "✓" : "(LOST INCREMENTS)");

    /* Atomic: always correct */
    atomic_store(&g_safe, 0);
    for (int i = 0; i < NTHREADS; i++)
        pthread_create(&tids[i], NULL, safe_worker, NULL);
    for (int i = 0; i < NTHREADS; i++)
        pthread_join(tids[i], NULL);
    printf("  atomic_fetch_add:    expected %d, got %ld  ✓\n\n",
           NTHREADS * ITERS, atomic_load(&g_safe));

    puts("  memory_order_relaxed: atomicity guaranteed, but no ordering");
    puts("  guarantees relative to other memory accesses.  Fine for counters\n");
    puts("  where you only care about the final value.\n");
}

/* ── 3. Compare-and-exchange (CAS) ───────────────────────────────────────── */

static void section3_cas(void) {
    puts("═══ 3. Compare-and-Exchange (CAS) ════\n");

    atomic_int val = 100;

    /*
     * atomic_compare_exchange_strong(obj, *expected, desired)
     *   if *obj == *expected  → *obj = desired, return true
     *   else                  → *expected = *obj, return false
     */
    int expected = 100;
    bool ok = atomic_compare_exchange_strong(&val, &expected, 200);
    printf("  CAS(100→200) when val=100: %s, val now %d\n",
           ok ? "success" : "fail", atomic_load(&val));

    expected = 100;   /* stale expectation */
    ok = atomic_compare_exchange_strong(&val, &expected, 300);
    printf("  CAS(100→300) when val=200: %s, expected updated to %d\n\n",
           ok ? "success" : "fail", expected);

    /* CAS loop: atomic maximum update */
    atomic_int maximum = 0;
    int values[] = { 7, 3, 9, 1, 5, 8, 2 };
    for (int i = 0; i < 7; i++) {
        int cur = atomic_load(&maximum);
        while (values[i] > cur) {
            /* If another thread changed maximum, cur is refreshed and we retry */
            if (atomic_compare_exchange_weak(&maximum, &cur, values[i]))
                break;
        }
    }
    printf("  CAS-loop max of {7,3,9,1,5,8,2}: %d\n\n", atomic_load(&maximum));

    puts("  strong vs weak:");
    puts("    strong: guaranteed to succeed if *obj == *expected");
    puts("    weak:   may spuriously fail (use inside a retry loop for performance)\n");
}

/* ── 4. Memory ordering ──────────────────────────────────────────────────── */

static void section4_ordering(void) {
    puts("═══ 4. Memory Ordering ════\n");

    puts("  Every atomic operation accepts an optional memory_order argument.");
    puts("  The default (memory_order_seq_cst) is the strongest and safest.\n");

    puts("  memory_order_relaxed");
    puts("    Only atomicity; no ordering relative to other memory accesses.");
    puts("    Use for: independent counters, statistics, reference counts.\n");

    puts("  memory_order_release  (on a store)");
    puts("  memory_order_acquire  (on a load)");
    puts("    Acquire-release pair: all stores BEFORE the release are visible");
    puts("    to threads that subsequently do an acquire on the same atomic.");
    puts("    Use for: publish-subscribe, passing data between two threads,");
    puts("    implementing mutexes and spinlocks.\n");

    puts("  memory_order_seq_cst  (default)");
    puts("    Total sequential consistency: all threads agree on the order of");
    puts("    ALL seq_cst operations across ALL atomics.");
    puts("    Use for: anything where the relaxed/acq-rel model is unclear.\n");

    /* Demonstrate acquire/release: producer publishes a value */
    atomic_int data    = 0;
    atomic_int ready   = 0;    /* the flag */

    atomic_store_explicit(&data, 42, memory_order_relaxed);
    atomic_store_explicit(&ready, 1,  memory_order_release);  /* publish */

    /* Consumer: acquire ensures data is visible after we see ready==1 */
    int r = atomic_load_explicit(&ready, memory_order_acquire);
    int d = atomic_load_explicit(&data,  memory_order_relaxed);
    printf("  acquire/release: ready=%d, data=%d\n\n", r, d);
}

/* ── 5. Spinlock using atomic_flag ───────────────────────────────────────── */

typedef struct {
    atomic_flag flag;
} Spinlock;

#define SPINLOCK_INIT { ATOMIC_FLAG_INIT }

static void spin_lock(Spinlock *s) {
    /* test_and_set returns the OLD value; loop while it was already set */
    while (atomic_flag_test_and_set_explicit(&s->flag, memory_order_acquire))
        ;   /* spin */
}

static void spin_unlock(Spinlock *s) {
    atomic_flag_clear_explicit(&s->flag, memory_order_release);
}

static Spinlock g_spin = SPINLOCK_INIT;
static long     g_spin_count = 0;

static void *spin_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERS; i++) {
        spin_lock(&g_spin);
        g_spin_count++;
        spin_unlock(&g_spin);
    }
    return NULL;
}

static void section5_spinlock(void) {
    puts("═══ 5. Spinlock (atomic_flag) ════\n");

    puts("  atomic_flag is the only type guaranteed lock-free on every platform.");
    puts("  test_and_set: atomically sets the flag and returns the OLD value.\n");

    pthread_t tids[NTHREADS];
    for (int i = 0; i < NTHREADS; i++)
        pthread_create(&tids[i], NULL, spin_worker, NULL);
    for (int i = 0; i < NTHREADS; i++)
        pthread_join(tids[i], NULL);
    printf("  spinlock counter: expected %d, got %ld  ✓\n\n",
           NTHREADS * ITERS, g_spin_count);

    puts("  Spinlock vs pthread_mutex:");
    puts("    Spinlock: busy-waits (no syscall) — good for <1µs critical sections");
    puts("    Mutex:    sleeps the thread (syscall) — better when lock held >1µs\n");
}

/* ── 6. Lock-free stack (Treiber stack) ──────────────────────────────────── */

typedef struct Node {
    int          value;
    struct Node *next;
} Node;

typedef struct {
    _Atomic(Node *) head;
} LFStack;

static void lf_push(LFStack *s, Node *node) {
    Node *old_head;
    do {
        old_head   = atomic_load(&s->head);
        node->next = old_head;
    } while (!atomic_compare_exchange_weak(&s->head, &old_head, node));
}

static Node *lf_pop(LFStack *s) {
    Node *old_head;
    Node *new_head;
    do {
        old_head = atomic_load(&s->head);
        if (!old_head) return NULL;         /* empty */
        new_head = old_head->next;
    } while (!atomic_compare_exchange_weak(&s->head, &old_head, new_head));
    return old_head;
}

static void section6_lockfree_stack(void) {
    puts("═══ 6. Lock-free Stack (Treiber Stack) ════\n");

    LFStack stack = { .head = NULL };

    /* Push 5 nodes */
    for (int i = 1; i <= 5; i++) {
        Node *n = malloc(sizeof *n);
        n->value = i * 10;
        lf_push(&stack, n);
    }

    /* Pop them (LIFO order) */
    printf("  pop order: ");
    Node *n;
    while ((n = lf_pop(&stack)) != NULL) {
        printf("%d ", n->value);
        free(n);
    }
    puts("");

    puts("\n  How push works:");
    puts("    1. load current head");
    puts("    2. set node->next = head");
    puts("    3. CAS: if head unchanged, swing it to node; else retry from 1");

    puts("\n  ABA problem (not fixed here):");
    puts("    Thread 1 reads head=A. Thread 2 pops A, pushes B, pushes A.");
    puts("    Thread 1 CAS succeeds (head still looks like A) but next is wrong.");
    puts("    Fix: hazard pointers, tagged pointers, or epoch-based reclamation.\n");

    puts("  When to use atomics:");
    puts("    ✓ Simple counters / flags (atomic_fetch_add, atomic_flag)");
    puts("    ✓ Publish a value to other threads (store/release + load/acquire)");
    puts("    ✓ Lock-free data structures (CAS loops) when contention is high");
    puts("    ✗ Invariants spanning multiple variables → use a mutex instead\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    section1_types();
    section2_counter();
    section3_cas();
    section4_ordering();
    section5_spinlock();
    section6_lockfree_stack();
    return 0;
}
