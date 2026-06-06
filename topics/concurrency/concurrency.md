# Concurrency (pthreads)

Compile with `-pthread` (sets `-lpthread` and `-D_REENTRANT`).

## Thread lifecycle

```c
#include <pthread.h>

// Thread function signature — must match exactly
void *thread_fn(void *arg);

pthread_t tid;

// Create — arg is passed to thread_fn as its argument
pthread_create(&tid, NULL, thread_fn, arg);

// Join — blocks until thread exits; retrieves return value
void *retval;
pthread_join(tid, &retval);

// Self-identification
pthread_t self = pthread_self();
```

### Passing arguments

```c
// Always heap-allocate args — never pass a stack address
typedef struct { int id; int n; } Args;

void *worker(void *raw) {
    Args *a = raw;
    int result = a->id * a->n;
    free(a);
    // Return heap-allocated result; caller must free
    int *r = malloc(sizeof *r);
    *r = result;
    return r;
}

Args *a = malloc(sizeof *a);
*a = (Args){ .id = 1, .n = 42 };
pthread_create(&tid, NULL, worker, a);

int *res;
pthread_join(tid, (void **)&res);
printf("%d\n", *res);
free(res);
```

## Mutex — mutual exclusion

```c
// Static initialisation
pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

// Dynamic initialisation (for heap-allocated mutex)
pthread_mutex_t mu;
pthread_mutex_init(&mu, NULL);
pthread_mutex_destroy(&mu);   // call when done

// Lock / unlock
pthread_mutex_lock(&mu);
// --- critical section ---
pthread_mutex_unlock(&mu);

// Non-blocking attempt
if (pthread_mutex_trylock(&mu) == 0) {
    // got it
    pthread_mutex_unlock(&mu);
} else {
    // EBUSY — someone else holds it
}
```

### Pattern: protect shared state

```c
static long        g_count = 0;
static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;

void increment(void) {
    pthread_mutex_lock(&g_mu);
    g_count++;
    pthread_mutex_unlock(&g_mu);
}
```

## Condition variables

Condition variables let a thread sleep until a condition becomes true,
without busy-waiting.

```c
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mu   = PTHREAD_MUTEX_INITIALIZER;

// Waiter — must hold mutex before calling wait
pthread_mutex_lock(&mu);
while (!condition)                         // while, NOT if (spurious wakeups!)
    pthread_cond_wait(&cond, &mu);         // atomically releases mu and blocks
// condition is now true; mu is re-acquired
pthread_mutex_unlock(&mu);

// Signaller
pthread_mutex_lock(&mu);
condition = 1;
pthread_cond_signal(&cond);                // wake one waiter
// or pthread_cond_broadcast(&cond);       // wake all waiters
pthread_mutex_unlock(&mu);
```

### Bounded queue (producer/consumer)

```c
typedef struct {
    int  data[8];
    int  head, tail, count, closed;
    pthread_mutex_t mu;
    pthread_cond_t  not_full, not_empty;
} Queue;

void push(Queue *q, int v) {
    pthread_mutex_lock(&q->mu);
    while (q->count == 8) pthread_cond_wait(&q->not_full, &q->mu);
    q->data[q->tail] = v;
    q->tail = (q->tail + 1) % 8;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mu);
}

int pop(Queue *q, int *out) {   // returns 0 when closed and empty
    pthread_mutex_lock(&q->mu);
    while (!q->count && !q->closed) pthread_cond_wait(&q->not_empty, &q->mu);
    if (!q->count) { pthread_mutex_unlock(&q->mu); return 0; }
    *out = q->data[q->head];
    q->head = (q->head + 1) % 8;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mu);
    return 1;
}
```

## Thread attributes

```c
pthread_attr_t attr;
pthread_attr_init(&attr);

// Detached: resources freed on exit, cannot be joined
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

// Custom stack size
pthread_attr_setstacksize(&attr, 256 * 1024);   // 256 KB

pthread_create(&tid, &attr, fn, arg);
pthread_attr_destroy(&attr);
```

## sig_atomic_t flag from a signal handler

```c
static volatile sig_atomic_t g_stop = 0;

void sigint_handler(int sig) { (void)sig; g_stop = 1; }

// In a thread:
while (!g_stop) { do_work(); }
```

## Pitfalls

| Pitfall | Consequence | Fix |
|---------|-------------|-----|
| Accessing shared data without a mutex | Data race → UB | Protect every access |
| Using `if` instead of `while` for cond_wait | Spurious wakeup breaks invariant | Always use `while` |
| Passing stack address as thread arg | Dangling pointer | Heap-allocate args |
| Not joining a joinable thread | Resource leak (zombie thread) | `pthread_join` or detach |
| Acquiring locks in different orders | Deadlock | Establish a global lock order |
| Calling `printf` from a signal handler | Async-signal-unsafe | Use `write()` or set a flag |
| Double-locking a non-recursive mutex | Deadlock | Use `PTHREAD_MUTEX_RECURSIVE` or restructure |

## Deadlock prevention

Always acquire locks in the same global order across all threads.

```c
// Thread A and Thread B both acquire mu_a before mu_b — never deadlock
pthread_mutex_lock(&mu_a);
pthread_mutex_lock(&mu_b);
// ...
pthread_mutex_unlock(&mu_b);
pthread_mutex_unlock(&mu_a);
```
