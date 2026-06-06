# POSIX Semaphores

A semaphore is a non-negative integer with two atomic operations:
- **wait (P):** if value > 0 decrement; else block until > 0
- **post (V):** increment; unblock one waiter if any

Unlike a mutex, a semaphore can be posted by a different thread/process than the one that waited.

Requires `#define _POSIX_C_SOURCE 200809L` (or `_GNU_SOURCE`) and `<semaphore.h>`.
Link with `-pthread` (and sometimes `-lrt` on older systems).

## Unnamed semaphores (threads / shared memory)

```c
#include <semaphore.h>

sem_t s;

/* pshared=0: threads in same process
 * pshared=1: processes sharing a mmap/shmem region
 * value:     initial count */
sem_init(&s, /*pshared*/0, /*value*/1);

sem_wait(&s);      /* P: decrement; blocks if 0  */
sem_post(&s);      /* V: increment; wakes waiter */

/* Non-blocking attempt */
if (sem_trywait(&s) == 0) {
    /* acquired */
} else {
    /* EAGAIN: semaphore was 0 */
}

/* Inspect value (approximate — may change before you act on it) */
int v;
sem_getvalue(&s, &v);

sem_destroy(&s);   /* must call when done */
```

## Timed wait

```c
#include <time.h>

struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
ts.tv_sec  += 1;          /* 1 second from now */
/* nanosecond overflow guard */
if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }

int rc = sem_timedwait(&s, &ts);
if (rc < 0 && errno == ETIMEDOUT) {
    /* deadline expired */
}
```

The deadline is **absolute** `CLOCK_REALTIME`, not a duration. Always build the timespec from `clock_gettime` + delta.

## Binary semaphore — producer/consumer handoff

```c
/* Semaphore starts at 0; consumer blocks until producer posts */
sem_t ready;
sem_init(&ready, 0, 0);

/* Producer thread */
prepare_data();
sem_post(&ready);   /* signal: data is ready */

/* Consumer thread */
sem_wait(&ready);   /* blocks until producer posts */
consume_data();
```

Use this instead of busy-polling. Prefer a condition variable when the predicate is more complex.

## Counting semaphore — bounded resource pool

```c
#define POOL_SIZE 3
sem_t pool;
sem_init(&pool, 0, POOL_SIZE);

/* Acquire a resource (blocks if all slots in use) */
sem_wait(&pool);
use_resource();
sem_post(&pool);   /* release it */
```

At most `POOL_SIZE` threads hold a resource simultaneously.

## Named semaphores — cross-process

Named semaphores live in the filesystem namespace (`/name`) and survive `fork()`.

```c
/* Create (first process) */
sem_t *s = sem_open("/myapp_sem", O_CREAT | O_EXCL, 0600, /*value*/0);
if (s == SEM_FAILED) { perror("sem_open"); }

/* Open existing (other process) */
sem_t *s2 = sem_open("/myapp_sem", 0);

sem_wait(s2);   /* or sem_post, sem_trywait, sem_timedwait */

sem_close(s);       /* close this handle (does NOT destroy) */
sem_unlink("/myapp_sem");   /* remove from namespace */
```

**Lifecycle:** the semaphore persists until `sem_unlink` — even after all processes call `sem_close`. Always `sem_unlink` in the creator (or a cleanup path).

## Named semaphore full pattern

```c
/* Always unlink stale semaphore from a previous crash */
sem_unlink("/myapp");

sem_t *sem = sem_open("/myapp", O_CREAT | O_EXCL, 0600, 0);

pid_t pid = fork();
if (pid == 0) {
    sem_t *child_sem = sem_open("/myapp", 0);
    do_work();
    sem_post(child_sem);
    sem_close(child_sem);
    exit(0);
}

sem_wait(sem);    /* parent waits for child's signal */
sem_close(sem);
sem_unlink("/myapp");
waitpid(pid, NULL, 0);
```

## Semaphore vs mutex vs condition variable

| | Semaphore | Mutex | Condition variable |
|-|-----------|-------|--------------------|
| Binary (locked/unlocked) | ✓ (value 0/1) | ✓ | — |
| Counting | ✓ | — | — |
| Cross-process | ✓ (named) | Only with `PTHREAD_PROCESS_SHARED` | Only with shared mutex |
| Owner must unlock | No | Yes | — |
| Signalling | ✓ (primary use) | No | ✓ (with mutex) |
| Spurious wakeups | No | — | Yes (use `while` loop) |

**Rule of thumb:** use a semaphore for signalling between two threads/processes; use a mutex + condition variable when you need to check a complex predicate.

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Forgetting `sem_destroy` (unnamed) | Call in every exit path |
| Forgetting `sem_unlink` (named) | Named semaphore leaks in `/dev/shm` |
| Using `sem_getvalue` result as a condition | Value may change immediately — `trywait` is the atomic check |
| `sem_timedwait` with relative time | Must be absolute `CLOCK_REALTIME` |
| `pshared=1` without putting `sem_t` in shared memory | Works only if the sem_t is in `mmap(MAP_SHARED)` or `shm_open` region |
| Posting more than waiting threads expect | Over-post can cause starvation of other semaphores |
