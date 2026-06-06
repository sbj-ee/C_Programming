# Process Control

Requires `#define _POSIX_C_SOURCE 200809L` and `<unistd.h>`, `<sys/wait.h>`.

## fork — duplicate a process

```c
#include <unistd.h>
#include <sys/wait.h>

fflush(stdout);          // flush before fork — both copies share the buffer
pid_t pid = fork();

if (pid < 0) {
    perror("fork");      // failed — no child created
} else if (pid == 0) {
    // ── child ──
    printf("child: pid=%d ppid=%d\n", getpid(), getppid());
    exit(0);
} else {
    // ── parent ──
    printf("parent: pid=%d child=%d\n", getpid(), pid);
    waitpid(pid, NULL, 0);
}
```

**After fork:** both processes run from the same instruction. Parent and
child have independent copies of all memory (copy-on-write). Open file
descriptors are shared (same underlying file description).

## waitpid — collect exit status

```c
int status;
pid_t done = waitpid(pid, &status, 0);   // 0 = block until child changes state
// waitpid(-1, ...) waits for any child

// Decode the status word
if (WIFEXITED(status))
    printf("exited: %d\n", WEXITSTATUS(status));  // 0..255
else if (WIFSIGNALED(status))
    printf("killed: signal %d\n", WTERMSIG(status));
else if (WIFSTOPPED(status))
    printf("stopped: signal %d\n", WSTOPSIG(status));

// Non-blocking poll
pid_t r = waitpid(pid, &status, WNOHANG);
if (r == 0)  { /* child still running */ }
if (r == pid){ /* child changed state */ }
```

## exec — replace the process image

exec does not return on success. The child continues as the new program.

```c
#include <unistd.h>

// execvp: search PATH; argv is NULL-terminated
char *argv[] = { "echo", "hello", "world", NULL };
execvp("echo", argv);

// execve: full control over environment
char *envp[] = { "PATH=/bin", NULL };
execve("/bin/echo", argv, envp);

// After fork — the canonical pattern:
pid_t pid = fork();
if (pid == 0) {
    execvp(prog, args);
    perror("exec");     // only reached if exec failed
    exit(127);
}
waitpid(pid, &status, 0);
```

## Pipes — unidirectional byte stream

```c
#include <unistd.h>

int fd[2];           // fd[0]=read, fd[1]=write
pipe(fd);

fflush(stdout);
pid_t pid = fork();

if (pid == 0) {
    // child reads
    close(fd[1]);                    // close unused write end
    char buf[128];
    ssize_t n = read(fd[0], buf, sizeof buf - 1);
    buf[n] = '\0';
    printf("child: %s\n", buf);
    close(fd[0]);
    exit(0);
}

// parent writes
close(fd[0]);                        // close unused read end
write(fd[1], "hello", 5);
close(fd[1]);                        // signals EOF to child
waitpid(pid, NULL, 0);
```

### Two-way pipes

```c
int to_child[2], to_parent[2];
pipe(to_child); pipe(to_parent);
// fork...
// child: close(to_child[1]); close(to_parent[0]); read/write
// parent: close(to_child[0]); close(to_parent[1]); write/read
```

## popen — shell command pipe

```c
#include <stdio.h>

// "r" — read stdout of command
FILE *fp = popen("uname -r", "r");
char buf[64];
fgets(buf, sizeof buf, fp);
int rc = pclose(fp);   // rc encoded like waitpid status

// "w" — write to stdin of command
fp = popen("cat -n", "w");
fprintf(fp, "line one\n");
fprintf(fp, "line two\n");
pclose(fp);
```

## Signals and processes

```c
#include <signal.h>

kill(pid, SIGTERM);    // send SIGTERM to a process
kill(pid, SIGKILL);    // cannot be caught or ignored
kill(0,   SIGINT);     // send to whole process group

raise(SIGUSR1);        // send to self
```

## Zombie and orphan processes

- **Zombie**: child exited, parent never called `waitpid`. Slot stays in
  process table. Cleaned up when parent exits.
  *Fix*: always `waitpid`; or install a `SIGCHLD` handler that calls
  `waitpid(-1, NULL, WNOHANG)` in a loop.

- **Orphan**: parent exits before child. Child is re-parented to PID 1
  (init/systemd), which reaps it.
  *Not a bug* unless you need the parent to collect the exit status.

## File descriptor inheritance

Children inherit all open FDs from the parent (including sockets, pipes).
Close FDs that the child should not have access to, or set `O_CLOEXEC`
so they are closed automatically on exec:

```c
#include <fcntl.h>

// Set at open time
int fd = open(path, O_RDONLY | O_CLOEXEC);

// Set after open
fcntl(fd, F_SETFD, FD_CLOEXEC);
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Not flushing stdout before fork | `fflush(stdout)` before every `fork()` |
| Leaving zombie children | Always `waitpid` or install SIGCHLD handler |
| Unclosed pipe ends | Close every unused FD; child never sees EOF otherwise |
| Returning from child without `exit()` | Child continues running the parent's code |
| Accessing parent's heap pointers after fork | Copy-on-write gives a new copy; changes not shared |
