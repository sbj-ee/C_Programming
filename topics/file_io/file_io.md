# File I/O

C file I/O uses buffered `FILE *` streams (stdio) or raw file descriptors
(POSIX `int fd`). Prefer `FILE *` for portability; use `fd` when you need
`mmap`, `ioctl`, or non-blocking I/O.

## Opening and closing

```c
#include <stdio.h>

FILE *f = fopen(path, mode);
if (!f) { perror(path); return -1; }   // always check

fclose(f);   // flushes buffer and closes; returns EOF on error
```

### Mode strings

| Mode | Meaning |
|------|---------|
| `"r"` | Read. File must exist. |
| `"w"` | Write. Truncates or creates. |
| `"a"` | Append. Creates if needed. Always writes at end. |
| `"r+"` | Read + write. File must exist. |
| `"w+"` | Read + write. Truncates or creates. |
| `"b"` suffix | Binary mode: `"rb"`, `"wb"`, `"ab"` — disables newline translation |

## Text I/O

```c
// Formatted write / read
fprintf(f, "%s = %d\n", key, value);
int n = fscanf(f, "%63s = %d", key, &value);  // returns fields matched

// Line-at-a-time
char line[256];
while (fgets(line, sizeof line, f)) {
    line[strcspn(line, "\n")] = '\0';   // strip trailing newline
    // process line...
}
fputs("hello\n", f);

// Character-at-a-time
int c;
while ((c = fgetc(f)) != EOF)
    fputc(c, stdout);
```

## Binary I/O

```c
// Write n objects of size bytes each
size_t fwrite(const void *ptr, size_t size, size_t n, FILE *f);

// Read — returns number of items read (may be < n on EOF/error)
size_t fread(void *ptr, size_t size, size_t n, FILE *f);

// Example: persist a struct
typedef struct { uint32_t id; float value; } Record;
Record r = {1, 3.14f};
fwrite(&r, sizeof r, 1, f);

Record r2;
size_t got = fread(&r2, sizeof r2, 1, f);
if (got != 1) { /* EOF or error */ }
```

## Seeking

```c
// Move the file position
fseek(f, offset, whence);
//   whence: SEEK_SET (from start), SEEK_CUR (from current), SEEK_END (from end)

long pos = ftell(f);   // current position in bytes
rewind(f);             // seek to start, clear error flag
fseek(f, 0, SEEK_END);
long size = ftell(f);  // file size
```

## Error handling

```c
if (ferror(f)) {
    perror("read error");
    clearerr(f);   // clear error + EOF flags
}
if (feof(f)) { /* end of file reached */ }

// fclose can fail (flush error) — always check on writes
if (fclose(f) != 0) perror("fclose");
```

## Standard streams

```c
stdin   // FILE * — buffered standard input
stdout  // FILE * — line-buffered when connected to terminal
stderr  // FILE * — unbuffered; use for error messages

// Redirect at the OS level:
// ./prog < input.txt > output.txt 2> errors.txt
```

## Temporary files

```c
// tmpfile() — anonymous, deleted on close or exit
FILE *tmp = tmpfile();

// mkstemp() — named, you delete it (POSIX)
char tmpl[] = "/tmp/myapp-XXXXXX";
int fd = mkstemp(tmpl);
// tmpl now contains the actual path
unlink(tmpl);   // delete immediately so it cleans up even if we crash
FILE *f = fdopen(fd, "w+");
```

## POSIX file descriptors

Use when stdio buffering is unwanted (sockets, pipes, mmap).

```c
#include <fcntl.h>
#include <unistd.h>

int fd = open(path, O_RDONLY);
int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);

ssize_t n = read(fd,  buf, sizeof buf);
ssize_t n = write(fd, buf, len);
off_t   p = lseek(fd, offset, SEEK_SET);
close(fd);

// Convert between FILE* and fd
FILE *f = fdopen(fd, "r");   // wraps fd in a FILE*
int  fd = fileno(f);          // extracts fd from FILE*
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Not checking `fopen` return | Always `if (!f) { perror(...); }` |
| `fscanf` buffer overflow | Use field widths: `%63s` |
| Mixing `fgets` + `fscanf` on same file | `fscanf` leaves `\n` in buffer; `fgets` picks it up — use one or the other |
| Forgetting `fclose` | Resource leak; may lose buffered data |
| `fclose` after `fclose` | Double-free equivalent — set `f = NULL` |
| Binary data on Windows | Use `"rb"`/`"wb"` to disable CR/LF translation |
