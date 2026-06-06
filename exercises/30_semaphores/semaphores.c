/*
 * Exercise 30 — POSIX Semaphores
 *
 * A semaphore is a non-negative integer counter with two atomic operations:
 *   wait (P): decrement; block if already zero
 *   post (V): increment; wake a waiter if one exists
 *
 * Unlike a mutex, a semaphore can be posted by a different thread than
 * the one that waited — making it ideal for signalling and resource counting.
 *
 * Build: make   (links -pthread -lrt on some systems)
 * Run:   ./semaphores
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

/* ── 1. Concepts ─────────────────────────────────────────────────────────── */

static void section1_concepts(void) {
    puts("═══ 1. Semaphore Concepts ════\n");

    puts("  Two kinds:");
    puts("    Unnamed  (sem_init)   — lives in memory; shared between threads");
    puts("               (or between processes if in shared memory)");
    puts("    Named    (sem_open)   — lives in the filesystem namespace;");
    puts("               survives fork() and can be opened by name\n");

    puts("  Two operations:");
    puts("    sem_wait (P/down): if value>0 decrement; else block until > 0");
    puts("    sem_post (V/up):   increment; unblock one waiter if any\n");

    puts("  Semaphore vs Mutex:");
    puts("    Mutex: binary (locked/unlocked); owner must unlock it");
    puts("    Semaphore: counting; any thread/process can post it");
    puts("    Use semaphores for signalling; use mutexes for mutual exclusion\n");
}

/* ── 2. Unnamed semaphore basics ─────────────────────────────────────────── */

static void section2_basics(void) {
    puts("═══ 2. Unnamed Semaphore Basics ════\n");

    sem_t s;
    sem_init(&s, 0, 3);   /* pshared=0: threads only; initial value=3 */

    int v;
    sem_getvalue(&s, &v);
    printf("  initial value: %d\n", v);

    sem_wait(&s);          /* 3 → 2 */
    sem_getvalue(&s, &v);
    printf("  after wait:    %d\n", v);

    sem_wait(&s);          /* 2 → 1 */
    sem_post(&s);          /* 1 → 2 */
    sem_post(&s);          /* 2 → 3 */
    sem_post(&s);          /* 3 → 4 */
    sem_getvalue(&s, &v);
    printf("  after wait/post sequence: %d\n\n", v);

    /* sem_trywait: non-blocking attempt */
    int rc = sem_trywait(&s);
    printf("  sem_trywait when value=%d: %s\n", v, rc == 0 ? "acquired" : "EAGAIN");

    /* Drain all */
    while (sem_trywait(&s) == 0)
        ;
    rc = sem_trywait(&s);
    printf("  sem_trywait on empty:     %s (errno=%s)\n\n",
           rc == 0 ? "acquired" : "failed", strerror(errno));

    sem_destroy(&s);
}

/* ── 3. Binary semaphore as a signal ─────────────────────────────────────── */

static sem_t g_ready;
static int   g_value = 0;

static void *producer_thread(void *arg) {
    (void)arg;
    usleep(50000);          /* simulate work */
    g_value = 99;
    sem_post(&g_ready);     /* signal: data is ready */
    return NULL;
}

static void section3_signal(void) {
    puts("═══ 3. Binary Semaphore as a Signal ════\n");

    sem_init(&g_ready, 0, 0);   /* initial value 0 — consumer will block */

    pthread_t tid;
    pthread_create(&tid, NULL, producer_thread, NULL);

    puts("  consumer: waiting for producer...");
    sem_wait(&g_ready);         /* blocks until producer posts */
    printf("  consumer: got value %d\n\n", g_value);

    pthread_join(tid, NULL);
    sem_destroy(&g_ready);

    puts("  This is the classic producer→consumer handoff:");
    puts("    semaphore starts at 0; producer posts after preparing data;");
    puts("    consumer wakes and reads — no polling, no busy-wait.\n");
}

/* ── 4. Counting semaphore: bounded resource pool ────────────────────────── */

#define POOL_SIZE    3
#define NUM_WORKERS  6

static sem_t g_pool;    /* counts available resources */

static void *worker(void *arg) {
    int id = *(int *)arg;
    free(arg);

    sem_wait(&g_pool);                      /* acquire a resource slot */
    printf("  worker %d: using resource\n", id);
    usleep((useconds_t)(20000 + id * 5000));
    printf("  worker %d: releasing\n",      id);
    sem_post(&g_pool);                      /* release it */
    return NULL;
}

static void section4_counting(void) {
    puts("═══ 4. Counting Semaphore: Resource Pool ════\n");
    printf("  %d workers compete for %d resource slots\n\n", NUM_WORKERS, POOL_SIZE);

    sem_init(&g_pool, 0, POOL_SIZE);

    pthread_t tids[NUM_WORKERS];
    for (int i = 0; i < NUM_WORKERS; i++) {
        int *id = malloc(sizeof *id);
        *id = i;
        pthread_create(&tids[i], NULL, worker, id);
    }
    for (int i = 0; i < NUM_WORKERS; i++)
        pthread_join(tids[i], NULL);

    sem_destroy(&g_pool);
    puts("\n  At most POOL_SIZE workers run simultaneously.\n");
}

/* ── 5. sem_timedwait: bounded wait ──────────────────────────────────────── */

static void section5_timedwait(void) {
    puts("═══ 5. sem_timedwait (Bounded Wait) ════\n");

    sem_t s;
    sem_init(&s, 0, 0);    /* empty — wait will time out */

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 100000000L;   /* 100 ms deadline */
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }

    int rc = sem_timedwait(&s, &ts);
    if (rc < 0 && errno == ETIMEDOUT)
        puts("  sem_timedwait: timed out after 100 ms  ✓");
    else
        puts("  sem_timedwait: unexpected result");

    /* Now post first, then timedwait — should succeed immediately */
    sem_post(&s);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;
    rc = sem_timedwait(&s, &ts);
    printf("  sem_timedwait when value=1: %s\n\n",
           rc == 0 ? "acquired immediately  ✓" : strerror(errno));

    sem_destroy(&s);

    puts("  Deadline must be absolute CLOCK_REALTIME, not a duration.");
    puts("  Build the timespec with clock_gettime + add the desired interval.\n");
}

/* ── 6. Named semaphores: cross-process synchronisation ──────────────────── */

#define SEM_NAME "/ex30_demo"

static void section6_named(void) {
    puts("═══ 6. Named Semaphores (Cross-process) ════\n");

    /* Clean up from any previous crashed run */
    sem_unlink(SEM_NAME);

    /* Create named semaphore, initial value 0 */
    sem_t *s = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 0);
    if (s == SEM_FAILED) { perror("sem_open"); return; }

    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        /* Child: open the same named semaphore */
        sem_t *cs = sem_open(SEM_NAME, 0);
        usleep(30000);
        puts("  child:  posting named semaphore");
        sem_post(cs);
        sem_close(cs);
        exit(0);
    }

    puts("  parent: waiting on named semaphore...");
    sem_wait(s);
    puts("  parent: acquired  ✓");

    sem_close(s);
    sem_unlink(SEM_NAME);   /* remove from filesystem namespace */
    waitpid(pid, NULL, 0);

    puts("\n  Named semaphore lifecycle:");
    puts("    sem_open(name, O_CREAT, mode, value) — create or open");
    puts("    sem_open(name, 0)                    — open existing");
    puts("    sem_close(s)                          — close this handle");
    puts("    sem_unlink(name)                      — remove from namespace");
    puts("  The semaphore persists until sem_unlink; surviving all close() calls.\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    section1_concepts();
    section2_basics();
    section3_signal();
    section4_counting();
    section5_timedwait();
    section6_named();
    return 0;
}
