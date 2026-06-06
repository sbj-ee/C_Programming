/*
 * Exercise 25: Process Control
 *
 * A process is an independent program instance with its own address
 * space, file descriptors, and signal mask.  fork() creates a copy;
 * exec() replaces the copy with a new program; waitpid() collects
 * the exit status and prevents zombie processes.
 *
 * Topics:
 *   - fork()       — duplicate the calling process
 *   - getpid / getppid — process identity
 *   - waitpid()    — wait for a child, decode its exit status
 *   - exec() family — replace the process image with a new program
 *   - pipe()       — unidirectional kernel buffer between processes
 *   - popen()      — convenience wrapper: pipe to/from a shell command
 *   - Pitfalls     — zombies, orphans, buffering, fd inheritance
 *
 * POSIX features are enabled via _POSIX_C_SOURCE.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* fork, pipe, read, write, getpid, getppid, sleep */
#include <sys/wait.h>  /* waitpid, WIFEXITED, WEXITSTATUS, WIFSIGNALED */
#include <errno.h>
#include <fcntl.h>     /* fcntl, FD_CLOEXEC */
#include <signal.h>    /* kill, SIGTERM */

/* ──────────────────────────────────────────────────────────
 * Section 1: fork() and process identity
 *
 * fork() duplicates the calling process.  It returns twice:
 *   - In the parent: the child's PID (> 0)
 *   - In the child:  0
 *   - On failure:   -1 (no child created)
 *
 * After fork, parent and child run the same code from the same
 * instruction pointer but in separate address spaces — writes in
 * one do not affect the other (copy-on-write).
 *
 * Always fflush(stdout) before fork.  If buffered output is not
 * flushed, both parent and child inherit the unflushed buffer
 * and will both flush it — printing every line twice.
 * ────────────────────────────────────────────────────────── */

static void section_fork(void) {
    printf("=== 1. fork() and process identity ===\n");

    fflush(stdout);           /* flush before fork — see Pitfalls */
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* ── child ── */
        printf("  child:  pid=%-6d ppid=%d\n", getpid(), getppid());
        exit(0);
    }

    /* ── parent ── */
    printf("  parent: pid=%-6d child_pid=%d\n", getpid(), pid);
    waitpid(pid, NULL, 0);   /* reap child to avoid zombie */

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: Exit status
 *
 * waitpid(pid, &status, 0) blocks until the child with that
 * PID changes state.  The status word is decoded with macros:
 *
 *   WIFEXITED(status)   — true if child called exit() / returned from main
 *   WEXITSTATUS(status) — the value passed to exit() (low 8 bits)
 *   WIFSIGNALED(status) — true if child was killed by a signal
 *   WTERMSIG(status)    — which signal killed it
 *   WIFSTOPPED(status)  — true if child was stopped (SIGSTOP/SIGTSTP)
 *
 * Use -1 as pid to wait for any child; use WNOHANG to poll
 * without blocking.
 * ────────────────────────────────────────────────────────── */

static void check_status(int status) {
    if (WIFEXITED(status))
        printf("  exited normally, status=%d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("  killed by signal %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));
    else
        printf("  stopped or other\n");
}

static void section_status(void) {
    printf("=== 2. Exit status ===\n");

    int status;

    /* Child exits normally with code 42 */
    fflush(stdout);
    if (fork() == 0) exit(42);
    waitpid(-1, &status, 0);
    printf("  normal exit:  ");
    check_status(status);

    /* Child exits with code 0 (success) */
    fflush(stdout);
    if (fork() == 0) exit(0);
    waitpid(-1, &status, 0);
    printf("  exit(0):      ");
    check_status(status);

    /* Child killed by SIGTERM */
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) { pause(); exit(0); }   /* child waits for a signal */
    kill(pid, SIGTERM);
    waitpid(pid, &status, 0);
    printf("  killed:       ");
    check_status(status);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: exec() family
 *
 * exec() replaces the calling process's image with a new program.
 * If exec() succeeds it does not return — the process continues
 * as the new program from its own main().
 * If exec() fails it returns -1 and sets errno.
 *
 * Key variants:
 *   execvp(file, argv)   — search PATH; argv is NULL-terminated array
 *   execv(path, argv)    — absolute path; no PATH search
 *   execve(path, argv, envp) — also replace the environment
 *
 * The fork/exec pattern:
 *   pid = fork();
 *   if (pid == 0) { execvp(...); exit(127); }   // child
 *   waitpid(pid, ...);                           // parent
 * ────────────────────────────────────────────────────────── */

static void section_exec(void) {
    printf("=== 3. exec() ===\n");

    /* Run /bin/echo in a child */
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = { "echo", "  hello from execvp", NULL };
        execvp("echo", argv);
        /* Only reached on failure */
        perror("execvp");
        exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    printf("  echo exited with status %d\n", WEXITSTATUS(status));

    /* Exec a non-existent program — shows error handling */
    fflush(stdout);
    pid = fork();
    if (pid == 0) {
        char *argv[] = { "no_such_program_xyzzy", NULL };
        execvp(argv[0], argv);
        fprintf(stderr, "  execvp failed: %s\n", strerror(errno));
        exit(127);
    }
    waitpid(pid, &status, 0);
    printf("  bad exec: child exited with status %d\n", WEXITSTATUS(status));

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: Pipes
 *
 * pipe(fd) creates a kernel buffer with two file descriptors:
 *   fd[0] — read end
 *   fd[1] — write end
 *
 * After fork, both parent and child inherit both ends.
 * Always close the end you don't use:
 *   - If parent writes: parent closes fd[0]; child closes fd[1]
 *   - Unclosed write ends prevent EOF on the read end
 *
 * Pipes are unidirectional.  For two-way communication, create
 * two pipes before fork.
 *
 * Pipe capacity is typically 64 KB on Linux.  Writes larger
 * than PIPE_BUF (4096 bytes) may be split non-atomically.
 * ────────────────────────────────────────────────────────── */

static void section_pipe(void) {
    printf("=== 4. Pipes ===\n");

    /* ── one-way: parent writes, child reads ── */
    int p[2];
    pipe(p);
    fflush(stdout);
    pid_t pid = fork();

    if (pid == 0) {
        close(p[1]);                   /* child does not write */
        char buf[128];
        ssize_t n = read(p[0], buf, sizeof buf - 1);
        buf[n] = '\0';
        printf("  child received: \"%s\"\n", buf);
        close(p[0]);
        exit(0);
    }

    close(p[0]);                       /* parent does not read */
    const char *msg = "hello through the pipe";
    write(p[1], msg, strlen(msg));
    close(p[1]);                       /* EOF signals child that writing is done */
    waitpid(pid, NULL, 0);

    /* ── two-way: parent↔child via two pipes ── */
    int to_child[2], to_parent[2];
    pipe(to_child);
    pipe(to_parent);
    fflush(stdout);
    pid = fork();

    if (pid == 0) {
        /* child: read from to_child, write response to to_parent */
        close(to_child[1]);
        close(to_parent[0]);

        char buf[128];
        ssize_t n = read(to_child[0], buf, sizeof buf - 1);
        buf[n] = '\0';

        char resp[160];
        int rlen = snprintf(resp, sizeof resp, "echo: %s", buf);
        write(to_parent[1], resp, (size_t)rlen);

        close(to_child[0]);
        close(to_parent[1]);
        exit(0);
    }

    /* parent: write, then read response */
    close(to_child[0]);
    close(to_parent[1]);
    write(to_child[1], "ping", 4);
    close(to_child[1]);

    char resp[160];
    ssize_t n = read(to_parent[0], resp, sizeof resp - 1);
    resp[n] = '\0';
    printf("  parent received: \"%s\"\n", resp);
    close(to_parent[0]);
    waitpid(pid, NULL, 0);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: popen() — pipe to/from a shell command
 *
 * popen(command, mode) forks a shell, runs the command, and
 * returns a FILE* connected to its stdin ("w") or stdout ("r").
 * pclose() waits for the shell to finish and returns its exit
 * status (same encoding as waitpid).
 *
 * Equivalent to:  pipe → fork → exec("sh -c command")
 * but in one call.  Convenient for quick sub-process output.
 *
 * Limitation: only one direction per popen call.  For two-way
 * communication use two pipes as in Section 4.
 * ────────────────────────────────────────────────────────── */

static void section_popen(void) {
    printf("=== 5. popen() ===\n");

    /* Read the output of a command */
    FILE *fp = popen("uname -sr", "r");
    if (!fp) { perror("popen"); return; }
    char buf[256];
    while (fgets(buf, sizeof buf, fp)) {
        /* strip newline */
        buf[strcspn(buf, "\n")] = '\0';
        printf("  uname: %s\n", buf);
    }
    int rc = pclose(fp);
    printf("  pclose status: %d\n", WEXITSTATUS(rc));

    /* Write to a command's stdin */
    fp = popen("cat -n", "w");
    if (!fp) { perror("popen"); return; }
    fprintf(fp, "first line\n");
    fprintf(fp, "second line\n");
    fprintf(fp, "third line\n");
    pclose(fp);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 6: Pitfalls
 *
 * 1. Zombie processes
 *    A process that has exited but whose status has not been
 *    collected by waitpid is a zombie.  It keeps a slot in the
 *    process table until the parent waits.  A parent that never
 *    waits leaves zombies until the parent itself exits, at which
 *    point init (PID 1) adopts them and reaps them.
 *    Fix: always waitpid (or install SIGCHLD handler, or double-fork).
 *
 * 2. Orphan processes
 *    If the parent exits before the child, the child is re-parented
 *    to PID 1 (init/systemd).  It continues to run — not a bug per
 *    se, but often unintended.
 *
 * 3. Buffered I/O before fork
 *    stdio buffers are copied into both parent and child.
 *    Unflushed content is written twice.  Always fflush before fork.
 *
 * 4. File descriptor inheritance
 *    The child inherits all open file descriptors.  Unclosed FDs:
 *    - Keep the write end of a pipe open → child never sees EOF
 *    - Leak resources (open files, sockets)
 *    Fix: close unused ends; set O_CLOEXEC on FDs that should not
 *    survive exec (pipe2(fd, O_CLOEXEC) sets this atomically).
 *
 * 5. exec and open files
 *    exec does not close FDs unless O_CLOEXEC is set.  Inherited
 *    FDs in the exec'd program can be a security risk.
 * ────────────────────────────────────────────────────────── */

static void section_pitfalls(void) {
    printf("=== 6. Pitfalls ===\n");

    /* Demonstrate zombie briefly: fork, child exits, parent sleeps */
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) exit(0);   /* child exits immediately */

    /* Without waitpid here, pid is a zombie right now.
     * We collect it immediately to keep the demo clean. */
    int status;
    waitpid(pid, &status, 0);
    printf("  reaped zombie (status %d) — without waitpid this\n", WEXITSTATUS(status));
    printf("  entry would stay in the process table until we exit\n");

    /* Demonstrate FD_CLOEXEC — close-on-exec flag */
    {
        int fd[2];
        pipe(fd);
        /* Set close-on-exec so the FDs do not leak into exec'd children */
        fcntl(fd[0], F_SETFD, FD_CLOEXEC);
        fcntl(fd[1], F_SETFD, FD_CLOEXEC);
        printf("  FD_CLOEXEC set on pipe fds %d,%d — will close on exec\n",
               fd[0], fd[1]);
        close(fd[0]);
        close(fd[1]);
    }

    printf("\n  Pitfall summary:\n");
    printf("  ZOMBIE:    child exits, parent never calls waitpid\n");
    printf("  ORPHAN:    parent exits before child — child reparented to init\n");
    printf("  STDIO:     fflush(stdout) before every fork()\n");
    printf("  PIPE EOF:  close unused pipe ends or child never sees EOF\n");
    printf("  FD LEAK:   use O_CLOEXEC / pipe2 for FDs that should not exec\n");
    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);

    section_fork();
    section_status();
    section_exec();
    section_pipe();
    section_popen();
    section_pitfalls();
    return 0;
}
