# C11 Atomics

Compile with `-pthread` (or at minimum `-lpthread`). Include `<stdatomic.h>`.
No `_GNU_SOURCE` needed — C11 standard.

## Atomic types

```c
#include <stdatomic.h>

/* _Atomic qualifier */
_Atomic int      ai;
_Atomic long     al;
_Atomic _Bool    ab;
_Atomic(int *)   ap;   /* atomic pointer */

/* Predefined typedef aliases */
atomic_int        /* _Atomic int        */
atomic_long       /* _Atomic long       */
atomic_size_t     /* _Atomic size_t     */
atomic_bool       /* _Atomic _Bool      */
atomic_uintptr_t  /* _Atomic uintptr_t  */
```

## Basic operations

```c
atomic_int v = 0;   /* zero-initialise */

/* Store / load */
atomic_store(&v, 42);
int x = atomic_load(&v);

/* Arithmetic — returns the OLD value */
int old = atomic_fetch_add(&v, 5);   /* v += 5 */
old      = atomic_fetch_sub(&v, 2);  /* v -= 2 */

/* Bitwise — returns OLD value */
atomic_fetch_or (&v, 0xF0);
atomic_fetch_and(&v, 0x33);
atomic_fetch_xor(&v, 0xFF);
```

## Compare-and-exchange (CAS)

```c
/* atomic_compare_exchange_strong(obj, *expected, desired)
 *   if *obj == *expected  → *obj = desired, return true
 *   else                  → *expected = *obj, return false
 */
int expected = 0;
if (atomic_compare_exchange_strong(&v, &expected, 1)) {
    /* success: v was 0, now 1 */
} else {
    /* failure: expected updated to the actual value of v */
}

/* Weak variant: may spuriously fail — use inside a retry loop */
int cur;
do {
    cur = atomic_load(&v);
} while (!atomic_compare_exchange_weak(&v, &cur, cur + 1));
```

## CAS loop pattern

```c
/* Atomic max update (example of a CAS loop) */
void atomic_max(atomic_int *m, int val) {
    int cur = atomic_load(m);
    while (val > cur)
        if (atomic_compare_exchange_weak(m, &cur, val))
            break;   /* success */
    /* if CAS fails, cur is refreshed with the new value; retry */
}
```

## Memory ordering

Every atomic operation accepts an optional `memory_order` argument.
The default is `memory_order_seq_cst` (strongest, safest).

| Order | Guarantees |
|-------|-----------|
| `memory_order_relaxed` | Only atomicity; no ordering relative to other accesses |
| `memory_order_consume` | Depends on the value loaded (complex; use acquire instead) |
| `memory_order_acquire` | All stores before the paired release are visible after this load |
| `memory_order_release` | All stores before this store are visible to a paired acquire |
| `memory_order_acq_rel` | Both acquire and release (for read-modify-write ops) |
| `memory_order_seq_cst` | Total sequential consistency across all threads |

```c
/* Publish-subscribe with acquire/release */
_Atomic int ready = 0;
int data = 0;

/* Producer */
data = 42;                                                /* plain store */
atomic_store_explicit(&ready, 1, memory_order_release);  /* publish */

/* Consumer — sees data==42 once it observes ready==1 */
while (!atomic_load_explicit(&ready, memory_order_acquire))
    ;    /* spin */
use(data);   /* safe to read */

/* Counter (ordering irrelevant — use relaxed) */
atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed);
```

## atomic_flag — guaranteed lock-free

`atomic_flag` is the only type the standard guarantees is always lock-free.

```c
atomic_flag lock = ATOMIC_FLAG_INIT;   /* must use this initialiser */

/* test_and_set: atomically sets flag, returns OLD value */
while (atomic_flag_test_and_set_explicit(&lock, memory_order_acquire))
    ;   /* spin until we get it */

/* critical section */

atomic_flag_clear_explicit(&lock, memory_order_release);
```

## Spinlock pattern

```c
typedef struct { atomic_flag flag; } Spinlock;
#define SPINLOCK_INIT { ATOMIC_FLAG_INIT }

static inline void spin_lock(Spinlock *s) {
    while (atomic_flag_test_and_set_explicit(&s->flag, memory_order_acquire))
        ;
}
static inline void spin_unlock(Spinlock *s) {
    atomic_flag_clear_explicit(&s->flag, memory_order_release);
}
```

Use spinlocks only for critical sections shorter than ~1 µs. For longer holds, use `pthread_mutex_t`.

## Lock-free stack (Treiber)

```c
typedef struct Node { int val; struct Node *next; } Node;
typedef struct { _Atomic(Node *) head; } LFStack;

void push(LFStack *s, Node *n) {
    Node *old;
    do {
        old    = atomic_load(&s->head);
        n->next = old;
    } while (!atomic_compare_exchange_weak(&s->head, &old, n));
}

Node *pop(LFStack *s) {
    Node *old, *next;
    do {
        old = atomic_load(&s->head);
        if (!old) return NULL;
        next = old->next;
    } while (!atomic_compare_exchange_weak(&s->head, &old, next));
    return old;
}
```

**ABA problem:** thread reads head=A; another thread pops A, pushes B, pushes A; first thread CAS succeeds but `next` is stale. Mitigations: hazard pointers, tagged pointers, epoch-based reclamation.

## is_lock_free check

```c
atomic_int v;
if (atomic_is_lock_free(&v))
    puts("hardware atomic");
else
    puts("uses internal lock");

/* Compile-time constants */
ATOMIC_INT_LOCK_FREE    /* 0=never, 1=sometimes, 2=always */
ATOMIC_LONG_LOCK_FREE
ATOMIC_POINTER_LOCK_FREE
```

## When to use atomics vs mutex

| Use atomics for | Use a mutex for |
|-----------------|-----------------|
| Simple counters (`fetch_add`) | Invariants spanning multiple variables |
| Boolean flags (`atomic_bool`) | Complex data structure updates |
| Reference counting | Any critical section > ~1 µs |
| Publish one value to other threads | When you need condition variable |
| Lock-free structures (CAS loop) | When correctness is more important than throughput |

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Using `if` instead of `while` for CAS loop | `while (!CAS(...))` — spurious failures are normal for `weak` |
| Relaxed ordering for publish/subscribe | Use release on store, acquire on load |
| Assuming `volatile` is sufficient for multi-threading | Use `_Atomic` — volatile provides no ordering |
| Double-checking a non-atomic condition outside a lock | Re-check inside the lock or use atomics consistently |
| ABA in lock-free structures | Tag pointers or use hazard pointers |
