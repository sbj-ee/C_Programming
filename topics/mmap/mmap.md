# Memory-Mapped Files (mmap)

Requires `#define _POSIX_C_SOURCE 200809L` and `<sys/mman.h>`, `<fcntl.h>`.

## Signature

```c
#include <sys/mman.h>

void *mmap(void   *addr,    // suggested start (NULL = kernel chooses)
           size_t  length,  // bytes to map
           int     prot,    // PROT_READ | PROT_WRITE | PROT_EXEC | PROT_NONE
           int     flags,   // MAP_SHARED or MAP_PRIVATE, + modifiers
           int     fd,      // open file descriptor (-1 for anonymous)
           off_t   offset); // byte offset in file (must be page-aligned)

// Returns MAP_FAILED ((void *)-1) on error — never NULL
if (ptr == MAP_FAILED) { perror("mmap"); }

int munmap(void *addr, size_t length);  // release the mapping
```

## Key flag combinations

| Flags | fd | Description |
|-------|----|-------------|
| `MAP_PRIVATE \| PROT_READ` | file | Read-only copy-on-write view of file |
| `MAP_SHARED \| PROT_READ\|WRITE` | file | Writes go back to the file |
| `MAP_PRIVATE \| MAP_ANONYMOUS` | -1 | Private zero-filled block (like malloc) |
| `MAP_SHARED \| MAP_ANONYMOUS` | -1 | Shared between parent and forked children |

## File-backed read-only mapping

```c
int fd = open(path, O_RDONLY);
struct stat st;
fstat(fd, &st);
size_t size = (size_t)st.st_size;

char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
close(fd);   // fd can be closed immediately after mmap

if (data == MAP_FAILED) { perror("mmap"); return -1; }

// Access file contents through a pointer — no read() needed
char *nl = memchr(data, '\n', size);
if (nl) printf("first line length: %td\n", nl - data);

// Count occurrences of a byte
size_t count = 0;
for (size_t i = 0; i < size; i++)
    if (data[i] == ',') count++;

munmap(data, size);
```

## File-backed writable mapping (MAP_SHARED)

Writes are reflected in the file when the OS flushes dirty pages.
`msync` forces an immediate flush.

```c
// The file must be large enough before mapping — ftruncate if needed
int fd = open(path, O_RDWR | O_CREAT, 0600);
ftruncate(fd, sizeof(MyStruct));    // ensure the file is exactly this size

MyStruct *s = mmap(NULL, sizeof(MyStruct),
                   PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
close(fd);

s->count++;                         // modifies the backing file
msync(s, sizeof(MyStruct), MS_SYNC); // flush to disk synchronously

munmap(s, sizeof(MyStruct));

// Verify with read():
fd = open(path, O_RDONLY);
MyStruct v; read(fd, &v, sizeof v); close(fd);
```

## Anonymous private mapping

Zero-filled private block — not backed by a file. Returned to the OS
immediately on `munmap` (unlike `free`, which may cache it).

```c
size_t n = 1024 * 1024;   // 1M elements
int *arr = mmap(NULL, n * sizeof(int),
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE,
                -1, 0);
// arr[i] == 0 guaranteed — no memset needed
arr[0] = 42;
munmap(arr, n * sizeof(int));
```

Pages that remain zero are never physically allocated (they share the
zero page via copy-on-write), so you can map large ranges cheaply.

## Shared memory between processes

`MAP_ANONYMOUS | MAP_SHARED` before `fork()` — both processes see
each other's writes. `waitpid` provides the necessary happens-before.

```c
typedef struct { int counter; char msg[64]; } Shared;

Shared *shm = mmap(NULL, sizeof(Shared),
                   PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_SHARED, -1, 0);
shm->counter = 0;

fflush(stdout);
pid_t pid = fork();
if (pid == 0) {
    shm->counter = 42;
    strncpy(shm->msg, "from child", sizeof shm->msg - 1);
    munmap(shm, sizeof(Shared));
    exit(0);
}
waitpid(pid, NULL, 0);
printf("counter=%d msg=%s\n", shm->counter, shm->msg);
munmap(shm, sizeof(Shared));
```

**For concurrent access** (no `waitpid` ordering): use a mutex with
`PTHREAD_PROCESS_SHARED` attribute, or a POSIX semaphore.

## msync

```c
// MS_SYNC  — block until pages are on disk
// MS_ASYNC — schedule the write, return immediately
// MS_INVALIDATE — invalidate other mappings of the same file

msync(addr, length, MS_SYNC);
```

## Page alignment

The `offset` argument must be a multiple of the page size.
To map starting at an unaligned byte offset, round down and adjust:

```c
long   pgsz = sysconf(_SC_PAGESIZE);   // typically 4096
off_t  off  = desired_offset;
off_t  base = (off / pgsz) * pgsz;    // round down to page boundary
size_t adj  = (size_t)(off - base);   // bytes into the first page

char *map = mmap(NULL, length + adj, prot, flags, fd, base);
char *ptr = map + adj;                 // actual start of desired data
// ...
munmap(map, length + adj);
```

## madvise — access hints

```c
#include <sys/mman.h>

madvise(ptr, length, MADV_SEQUENTIAL);  // prefetch ahead (linear scan)
madvise(ptr, length, MADV_RANDOM);      // no prefetch (random access)
madvise(ptr, length, MADV_WILLNEED);    // start loading pages now
madvise(ptr, length, MADV_DONTNEED);    // release pages (keep mapping)
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Checking `== NULL` instead of `== MAP_FAILED` | Always `if (ptr == MAP_FAILED)` |
| File-backed map larger than the file | SIGBUS on access past EOF; `ftruncate` first |
| Unaligned `offset` | Round down to `sysconf(_SC_PAGESIZE)` multiple |
| Forgetting `munmap` | No valgrind warning — virtual address space leak |
| Not calling `msync` before crash-recovery check | Dirty pages may not be on disk yet |
| Sharing `MAP_PRIVATE` across fork expecting writes to be visible | Use `MAP_SHARED` instead |
