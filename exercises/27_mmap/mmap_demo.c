/*
 * Exercise 27: Memory-Mapped Files (mmap)
 *
 * mmap() maps a file or anonymous block of memory directly into the
 * process's virtual address space.  The OS handles paging: data is
 * read from disk on first access (demand paging) and written back
 * automatically for MAP_SHARED mappings.
 *
 * Advantages over read()/write():
 *   - Zero-copy: no user-space buffer; the kernel page cache IS the buffer
 *   - Random access at pointer speed — no lseek needed
 *   - MAP_SHARED lets processes share pages without IPC overhead
 *   - Large files can be mapped without loading the whole file into RAM
 *
 * Topics:
 *   - mmap / munmap basics
 *   - File-backed read-only mapping
 *   - File-backed MAP_SHARED writable mapping (in-place file edit)
 *   - Anonymous private mapping (malloc alternative)
 *   - Anonymous MAP_SHARED: shared memory between parent and child
 *   - msync, page alignment, pitfalls
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdint.h>

#define TMP_RO  "/tmp/ex27_ro.txt"
#define TMP_RW  "/tmp/ex27_rw.bin"

/* ──────────────────────────────────────────────────────────
 * Section 1: Concepts
 *
 * mmap() signature:
 *   void *mmap(void *addr, size_t length, int prot, int flags,
 *              int fd, off_t offset);
 *
 *   addr    — suggested start address (NULL = kernel chooses)
 *   length  — number of bytes to map
 *   prot    — PROT_READ | PROT_WRITE | PROT_EXEC | PROT_NONE
 *   flags   — MAP_SHARED or MAP_PRIVATE, + MAP_ANONYMOUS, etc.
 *   fd      — open file descriptor (-1 for anonymous)
 *   offset  — byte offset in file (must be page-aligned)
 *
 * Returns MAP_FAILED on error (not NULL — always check this way):
 *   if (ptr == MAP_FAILED) { perror("mmap"); ... }
 *
 * Key flag combinations:
 *   MAP_PRIVATE  | file fd  → copy-on-write view of file
 *   MAP_SHARED   | file fd  → writes go back to the file
 *   MAP_PRIVATE  | MAP_ANONYMOUS | fd=-1 → private zero-filled block
 *   MAP_SHARED   | MAP_ANONYMOUS | fd=-1 → shared between forked procs
 * ────────────────────────────────────────────────────────── */

static void section_concepts(void) {
    printf("=== 1. Concepts ===\n");

    long pagesz = sysconf(_SC_PAGESIZE);
    printf("  page size: %ld bytes (%ld KB)\n", pagesz, pagesz / 1024);
    printf("  mmap offset must be a multiple of page size\n");
    printf("  mmap length is rounded up to the next page boundary\n");

    /* Show what MAP_FAILED looks like */
    printf("  MAP_FAILED == (void *)-1 == %p\n", MAP_FAILED);

    /* Intentionally provoke a failure to show the error path */
    void *bad = mmap(NULL, 0, PROT_READ, MAP_PRIVATE, -1, 0);
    if (bad == MAP_FAILED)
        printf("  mmap(length=0): %s\n", strerror(errno));

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: File-backed read-only mapping
 *
 * MAP_PRIVATE gives each process its own copy-on-write view.
 * The mapping is backed by the file — reading from the pointer
 * is equivalent to read(), but without a system call per access.
 *
 * Typical use: parse a large config or data file without
 * loading it all into a malloc'd buffer first.
 * ────────────────────────────────────────────────────────── */

static void section_read_mapping(void) {
    printf("=== 2. File-backed read-only mapping ===\n");

    /* Write a sample file */
    FILE *f = fopen(TMP_RO, "w");
    fprintf(f, "line one\nline two\nline three\n");
    fclose(f);

    /* Open and stat to get exact size */
    int fd = open(TMP_RO, O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    size_t size = (size_t)st.st_size;

    /* Map the whole file read-only */
    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);   /* fd can be closed immediately after mmap */

    if (data == MAP_FAILED) { perror("mmap"); return; }

    /* Access through a pointer — no read() needed */
    printf("  file size: %zu bytes\n", size);
    printf("  mapped at: %p\n", (void *)data);

    /* Count newlines by scanning the pointer */
    int lines = 0;
    for (size_t i = 0; i < size; i++)
        if (data[i] == '\n') lines++;
    printf("  line count: %d\n", lines);

    /* Print first line by finding the newline */
    char *nl = memchr(data, '\n', size);
    if (nl) {
        printf("  first line: \"%.*s\"\n", (int)(nl - data), data);
    }

    munmap(data, size);
    unlink(TMP_RO);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: File-backed MAP_SHARED writable mapping
 *
 * MAP_SHARED makes writes visible in the file immediately (when
 * the OS decides to flush the dirty pages).  Use msync() to
 * force a flush to disk before relying on durability.
 *
 * Typical use: databases, memory-mapped log files, large
 * binary records updated in place.
 * ────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t magic;
    uint32_t count;
    char     label[24];
} Record;

static void section_write_mapping(void) {
    printf("=== 3. File-backed MAP_SHARED write mapping ===\n");

    /* Create the file with initial content */
    int fd = open(TMP_RW, O_RDWR | O_CREAT | O_TRUNC, 0600);
    Record init = { .magic = 0xDEADBEEF, .count = 0, .label = "initial" };
    write(fd, &init, sizeof init);

    /* Map read-write, shared — writes go back to the file */
    Record *rec = mmap(NULL, sizeof(Record),
                       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (rec == MAP_FAILED) { perror("mmap"); return; }

    printf("  before: magic=0x%08X count=%u label=\"%s\"\n",
           rec->magic, rec->count, rec->label);

    /* Modify through the pointer — changes the backing file */
    rec->count = 42;
    strncpy(rec->label, "modified", sizeof rec->label - 1);

    /* msync: ensure dirty pages are written to disk now */
    msync(rec, sizeof(Record), MS_SYNC);

    printf("  after:  magic=0x%08X count=%u label=\"%s\"\n",
           rec->magic, rec->count, rec->label);

    munmap(rec, sizeof(Record));

    /* Re-open the file with ordinary read() to confirm the change */
    fd = open(TMP_RW, O_RDONLY);
    Record verify;
    read(fd, &verify, sizeof verify);
    close(fd);
    unlink(TMP_RW);

    printf("  verified via read(): count=%u label=\"%s\"\n",
           verify.count, verify.label);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: Anonymous private mapping
 *
 * MAP_ANONYMOUS | MAP_PRIVATE gives a zero-filled private block
 * that is not backed by any file.  It is essentially what malloc
 * calls for large allocations (typically > MMAP_THRESHOLD = 128KB).
 *
 * Advantages over malloc for large buffers:
 *   - Returned to the OS immediately on munmap (malloc may cache it)
 *   - Pages that stay zero are never actually allocated (COW with
 *     the zero page) — only touched pages consume RAM
 *   - Can be mremap'd to grow/shrink without copying
 * ────────────────────────────────────────────────────────── */

static void section_anonymous(void) {
    printf("=== 4. Anonymous private mapping ===\n");

    size_t n = 1024 * 1024;   /* 1 MB */
    int *arr = mmap(NULL, n * sizeof(int),
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE,
                    -1, 0);

    if (arr == MAP_FAILED) { perror("mmap"); return; }

    printf("  mapped %zu MB at %p\n", n * sizeof(int) / (1024*1024),
           (void *)arr);

    /* Pages start zeroed — no explicit memset needed */
    printf("  arr[0] before write: %d (guaranteed zero)\n", arr[0]);

    /* Fill and sum */
    long sum = 0;
    for (size_t i = 0; i < n; i++) { arr[i] = (int)(i % 1000); sum += arr[i]; }
    printf("  sum of 1M ints (mod 1000): %ld\n", sum);

    munmap(arr, n * sizeof(int));
    printf("  munmap returned memory to OS immediately\n");

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: Shared memory between processes
 *
 * MAP_ANONYMOUS | MAP_SHARED before fork() creates pages that are
 * physically shared between parent and child — a write in one is
 * immediately visible in the other with no IPC overhead.
 *
 * Synchronization is still required: use a mutex (from pthreads
 * with PTHREAD_PROCESS_SHARED) or a pipe/semaphore for ordering.
 * In this demo the parent waitpid()s for the child, which provides
 * the happens-before guarantee that lets us read safely.
 * ────────────────────────────────────────────────────────── */

typedef struct {
    int  counter;
    char message[64];
} Shared;

static void section_shared(void) {
    printf("=== 5. Shared memory (MAP_SHARED + fork) ===\n");

    Shared *shm = mmap(NULL, sizeof(Shared),
                       PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_SHARED,
                       -1, 0);

    if (shm == MAP_FAILED) { perror("mmap"); return; }

    shm->counter = 0;
    shm->message[0] = '\0';

    fflush(stdout);
    pid_t pid = fork();

    if (pid == 0) {
        /* child writes into the shared pages */
        shm->counter = 99;
        strncpy(shm->message, "written by child", sizeof shm->message - 1);
        printf("  child:  counter=%d  message=\"%s\"\n",
               shm->counter, shm->message);
        munmap(shm, sizeof(Shared));
        exit(0);
    }

    /* parent waits, then reads — waitpid provides the memory barrier */
    waitpid(pid, NULL, 0);
    printf("  parent: counter=%d  message=\"%s\"\n",
           shm->counter, shm->message);

    /* Contrast with MAP_PRIVATE: child's writes are NOT visible */
    Shared *priv = mmap(NULL, sizeof(Shared),
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE,
                        -1, 0);
    priv->counter = 0;

    fflush(stdout);
    pid = fork();
    if (pid == 0) {
        priv->counter = 999;   /* copy-on-write — parent's copy unchanged */
        exit(0);
    }
    waitpid(pid, NULL, 0);
    printf("  MAP_PRIVATE: parent counter=%d (child's write not visible)\n",
           priv->counter);

    munmap(shm,  sizeof(Shared));
    munmap(priv, sizeof(Shared));

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 6: Patterns and pitfalls
 *
 * 1. SIGBUS
 *    A file-backed mapping beyond the end of the file generates
 *    SIGBUS on access (not SIGSEGV).  Always fstat the file and
 *    map exactly st_st_size bytes — or truncate the file first
 *    to make it the right size (ftruncate).
 *
 * 2. msync() for durability
 *    Dirty pages are flushed lazily by the kernel.  Call msync()
 *    before relying on data surviving a crash.
 *    MS_SYNC  — synchronous: wait until pages are on disk
 *    MS_ASYNC — schedule the flush, return immediately
 *
 * 3. Alignment
 *    The offset argument to mmap() must be a multiple of the
 *    page size (sysconf(_SC_PAGESIZE), typically 4096).
 *    To map part of a file, round offset down to a page boundary
 *    and adjust the pointer by the remainder.
 *
 * 4. mmap for large files
 *    Map the whole file at once even if it is larger than RAM.
 *    The OS pages in only what you touch (demand paging).
 *    Hint to the kernel with madvise(MADV_SEQUENTIAL) for
 *    linear scans or madvise(MADV_RANDOM) for random access.
 *
 * 5. Forgetting to munmap
 *    Unlike malloc, valgrind does not report unfree'd mmap regions
 *    as leaks by default — but they still consume virtual address
 *    space and (if dirty) physical RAM until the process exits.
 * ────────────────────────────────────────────────────────── */

static void section_pitfalls(void) {
    printf("=== 6. Patterns and pitfalls ===\n");

    /* Demonstrate ftruncate to extend a file before mapping */
    int fd = open("/tmp/ex27_trunc.bin", O_RDWR | O_CREAT | O_TRUNC, 0600);
    ftruncate(fd, 4096);   /* make it exactly one page */

    char *page = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (page != MAP_FAILED) {
        snprintf(page, 4096, "page-sized file, mapped safely");
        printf("  ftruncate+mmap: \"%s\"\n", page);
        msync(page, 4096, MS_SYNC);
        munmap(page, 4096);
    }
    unlink("/tmp/ex27_trunc.bin");

    /* Show page-aligned offset math */
    long pgsz  = sysconf(_SC_PAGESIZE);
    off_t off  = 5000;   /* unaligned example offset */
    off_t base = (off / pgsz) * pgsz;      /* round down */
    size_t adj = (size_t)(off - base);     /* adjustment into page */
    printf("  offset %lld → page base %lld, ptr adj +%zu bytes\n",
           (long long)off, (long long)base, adj);

    printf("\n  Pitfall summary:\n");
    printf("  SIGBUS:    access past end-of-file on file-backed map\n");
    printf("  msync:     call MS_SYNC before relying on crash durability\n");
    printf("  alignment: offset must be multiple of sysconf(_SC_PAGESIZE)\n");
    printf("  MADV:      madvise(MADV_SEQUENTIAL/RANDOM) for access hints\n");
    printf("  munmap:    always unmap; valgrind won't catch mmap leaks\n");

    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);

    section_concepts();
    section_read_mapping();
    section_write_mapping();
    section_anonymous();
    section_shared();
    section_pitfalls();
    return 0;
}
