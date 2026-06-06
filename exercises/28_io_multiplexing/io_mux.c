/*
 * Exercise 28 — I/O Multiplexing: select, poll, epoll
 *
 * A single thread monitors multiple file descriptors simultaneously,
 * blocking until one or more become ready for I/O.  This is the
 * foundation of high-performance network servers.
 *
 * Build: make
 * Run:   ./io_mux
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

/* ── helper ──────────────────────────────────────────────────────────────── */

static void set_nonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

/*
 * Fork two writer children: A writes msg_a after delay_a ms,
 * B writes msg_b after delay_b ms. Returns their read-end fds
 * and PIDs so the caller can read and reap.
 */
static void two_writers(int *ra, int *rb,
                        int delay_a, const char *msg_a,
                        int delay_b, const char *msg_b,
                        pid_t *pida, pid_t *pidb) {
    int pa[2], pb[2];
    if (pipe(pa) || pipe(pb)) { perror("pipe"); exit(1); }
    *ra = pa[0]; *rb = pb[0];

    fflush(stdout);
    *pida = fork();
    if (*pida == 0) {
        close(pa[0]); close(pb[0]); close(pb[1]);
        usleep((useconds_t)delay_a * 1000);
        write(pa[1], msg_a, strlen(msg_a));
        close(pa[1]);
        exit(0);
    }
    close(pa[1]);

    fflush(stdout);
    *pidb = fork();
    if (*pidb == 0) {
        close(pa[0]); close(pb[0]);
        usleep((useconds_t)delay_b * 1000);
        write(pb[1], msg_b, strlen(msg_b));
        close(pb[1]);
        exit(0);
    }
    close(pb[1]);
}

/* ── 1. Concepts ─────────────────────────────────────────────────────────── */

static void section1_concepts(void) {
    puts("═══ 1. I/O Multiplexing Concepts ════\n");
    puts("Problem: a server with N clients cannot issue N blocking read() calls.");
    puts("Solution: monitor many fds at once; sleep until ANY becomes ready.\n");

    printf("  %-10s  %-10s  %-12s  %s\n",
           "API", "Standard", "FD limit", "Cost per wakeup");
    printf("  %-10s  %-10s  %-12s  %s\n",
           "select()", "POSIX",   "FD_SETSIZE", "O(max-fd)  — scans bitmap");
    printf("  %-10s  %-10s  %-12s  %s\n",
           "poll()",   "POSIX",   "unlimited",  "O(nfds)   — scans array");
    printf("  %-10s  %-10s  %-12s  %s\n",
           "epoll()",  "Linux",   "unlimited",  "O(events) — returns ready only");

    puts("\n  Readiness modes:");
    puts("    Level-triggered (LT): notified while data is available  [default for all three]");
    puts("    Edge-triggered  (ET): notified once on new data          [EPOLLET, needs O_NONBLOCK]\n");
}

/* ── 2. select() ─────────────────────────────────────────────────────────── */

static void section2_select(void) {
    puts("═══ 2. select() ═════════════════════\n");

    int ra, rb;
    pid_t pa, pb;
    two_writers(&ra, &rb, 150, "from-A", 50, "from-B", &pa, &pb);

    int nfds = (ra > rb ? ra : rb) + 1;
    int done_a = 0, done_b = 0;

    while (!done_a || !done_b) {
        fd_set rset;
        FD_ZERO(&rset);
        if (!done_a) FD_SET(ra, &rset);
        if (!done_b) FD_SET(rb, &rset);

        struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
        int n = select(nfds, &rset, NULL, NULL, &tv);
        if (n == 0) { puts("  timeout"); break; }
        if (n < 0)  { perror("select"); break; }

        if (!done_a && FD_ISSET(ra, &rset)) {
            char buf[32] = {0};
            read(ra, buf, sizeof buf - 1);
            printf("  select got: \"%s\"\n", buf);
            close(ra); done_a = 1;
        }
        if (!done_b && FD_ISSET(rb, &rset)) {
            char buf[32] = {0};
            read(rb, buf, sizeof buf - 1);
            printf("  select got: \"%s\"\n", buf);
            close(rb); done_b = 1;
        }
    }
    waitpid(pa, NULL, 0);
    waitpid(pb, NULL, 0);

    puts("\n  Notes:");
    printf("  fd_set is a bitmap; maximum monitored fd is FD_SETSIZE-1 (usually %d)\n",
           FD_SETSIZE - 1);
    puts("  Must rebuild fd_set from scratch before every select() call");
    puts("  Kernel scans all fds 0..nfds-1 — O(n) even when only one event fires\n");
}

/* ── 3. poll() ───────────────────────────────────────────────────────────── */

static void section3_poll(void) {
    puts("═══ 3. poll() ═══════════════════════\n");

    int ra, rb;
    pid_t pa, pb;
    two_writers(&ra, &rb, 150, "from-A", 50, "from-B", &pa, &pb);

    struct pollfd fds[2] = {
        { .fd = ra, .events = POLLIN },
        { .fd = rb, .events = POLLIN },
    };
    int remaining = 2;

    while (remaining > 0) {
        int n = poll(fds, 2, 2000);
        if (n == 0) { puts("  timeout"); break; }
        if (n < 0)  { perror("poll"); break; }

        for (int i = 0; i < 2 && remaining > 0; i++) {
            if (fds[i].revents & POLLIN) {
                char buf[32] = {0};
                read(fds[i].fd, buf, sizeof buf - 1);
                printf("  poll got: \"%s\" (fd=%d)\n", buf, fds[i].fd);
                close(fds[i].fd);
                fds[i].fd = -1;    /* negative fd: poll ignores this slot */
                remaining--;
            }
        }
    }
    waitpid(pa, NULL, 0);
    waitpid(pb, NULL, 0);

    puts("\n  Notes:");
    puts("  No FD_SETSIZE limit — nfds is just the array length");
    puts("  Set .fd = -1 to disable a slot without shrinking the array");
    puts("  revents is filled by the kernel; events is set only by the caller\n");
}

/* ── 4. epoll ─────────────────────────────────────────────────────────────── */

static void section4_epoll(void) {
    puts("═══ 4. epoll() ══════════════════════\n");

    int ra, rb;
    pid_t pa, pb;
    two_writers(&ra, &rb, 150, "from-A", 50, "from-B", &pa, &pb);

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = ra; epoll_ctl(epfd, EPOLL_CTL_ADD, ra, &ev);
    ev.data.fd = rb; epoll_ctl(epfd, EPOLL_CTL_ADD, rb, &ev);

    int remaining = 2;
    while (remaining > 0) {
        struct epoll_event events[4];
        int n = epoll_wait(epfd, events, 4, 2000);
        if (n == 0) { puts("  timeout"); break; }
        if (n < 0)  { perror("epoll_wait"); break; }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            char buf[32] = {0};
            read(fd, buf, sizeof buf - 1);
            printf("  epoll got: \"%s\" (fd=%d)\n", buf, fd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            remaining--;
        }
    }
    close(epfd);
    waitpid(pa, NULL, 0);
    waitpid(pb, NULL, 0);

    puts("\n  Notes:");
    puts("  epoll_create1  — create the interest table (one per event loop)");
    puts("  epoll_ctl(ADD) — register an fd; stays registered across calls");
    puts("  epoll_ctl(DEL) — unregister (always do this before close!)");
    puts("  epoll_wait     — returns ONLY ready fds, O(events) not O(watched)\n");
}

/* ── 5. Non-blocking I/O ─────────────────────────────────────────────────── */

static void section5_nonblocking(void) {
    puts("═══ 5. Non-blocking I/O (O_NONBLOCK) ════\n");

    int pfd[2];
    pipe(pfd);
    set_nonblocking(pfd[0]);

    /* Read from an empty pipe: must get EAGAIN, not block */
    char buf[32];
    ssize_t n = read(pfd[0], buf, sizeof buf);
    if (n < 0 && errno == EAGAIN)
        printf("  empty read:  EAGAIN (%d) — would block\n", EAGAIN);

    /* Write data, then read */
    write(pfd[1], "ready", 5);
    n = read(pfd[0], buf, sizeof buf - 1);
    buf[n > 0 ? n : 0] = '\0';
    printf("  data read:   \"%s\" (%zd bytes)\n\n", buf, n);

    close(pfd[0]);
    close(pfd[1]);

    puts("  Rules for edge-triggered epoll (EPOLLET):");
    puts("  1. fd MUST be O_NONBLOCK");
    puts("  2. Loop read()/write() on each event until EAGAIN — drain completely");
    puts("  3. If you don't drain, no new event fires until more data arrives");
    printf("  EAGAIN == EWOULDBLOCK on Linux (both %d)\n\n", EAGAIN);
}

/* ── 6. Multi-client echo server (epoll) ─────────────────────────────────── */

#define NUM_CLIENTS 3

static void run_clients(int port) {
    for (int i = 0; i < NUM_CLIENTS; i++) {
        fflush(stdout);
        if (fork() == 0) {
            usleep((useconds_t)(i + 1) * 30000);   /* stagger connections */
            int fd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in sa = {
                .sin_family      = AF_INET,
                .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
                .sin_port        = htons((uint16_t)port),
            };
            if (connect(fd, (struct sockaddr *)&sa, sizeof sa) < 0) {
                perror("client connect"); exit(1);
            }
            char msg[64];
            snprintf(msg, sizeof msg, "ping from client %d", i);
            write(fd, msg, strlen(msg));

            char reply[64];
            ssize_t nr = read(fd, reply, sizeof reply - 1);
            reply[nr > 0 ? nr : 0] = '\0';
            printf("  client %d: echo \"%s\"\n", i, reply);
            close(fd);
            exit(0);
        }
    }
    for (int i = 0; i < NUM_CLIENTS; i++) waitpid(-1, NULL, 0);
}

static void section6_echo_server(void) {
    puts("═══ 6. Multi-client echo server (epoll) ════\n");

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port        = 0,
    };
    bind(lfd, (struct sockaddr *)&addr, sizeof addr);
    listen(lfd, 16);

    socklen_t alen = sizeof addr;
    getsockname(lfd, (struct sockaddr *)&addr, &alen);
    int port = ntohs(addr.sin_port);
    printf("  server on port %d\n", port);

    set_nonblocking(lfd);

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event ev = { .events = EPOLLIN, .data.fd = lfd };
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    fflush(stdout);
    pid_t cpid = fork();
    if (cpid == 0) {
        close(epfd); close(lfd);
        run_clients(port);
        exit(0);
    }

    int active = 0, total_seen = 0;
    struct epoll_event events[16];

    for (;;) {
        int n = epoll_wait(epfd, events, 16, 3000);
        if (n == 0) { puts("  server: idle timeout"); break; }
        if (n < 0)  { perror("epoll_wait"); break; }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == lfd) {
                /* Drain all pending accepts (non-blocking listening socket) */
                for (;;) {
                    struct sockaddr_in peer;
                    socklen_t plen = sizeof peer;
                    int cfd = accept(lfd, (struct sockaddr *)&peer, &plen);
                    if (cfd < 0) break;   /* EAGAIN: all accepted */
                    set_nonblocking(cfd);
                    struct epoll_event cev = {
                        .events  = EPOLLIN | EPOLLRDHUP,
                        .data.fd = cfd,
                    };
                    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &cev);
                    active++; total_seen++;
                    printf("  server: accepted fd=%d (active=%d)\n", cfd, active);
                }
            } else {
                char buf[256];
                ssize_t nr = read(fd, buf, sizeof buf - 1);
                if (nr > 0) {
                    buf[nr] = '\0';
                    printf("  server: echo \"%s\" → fd=%d\n", buf, fd);
                    write(fd, buf, (size_t)nr);
                }
                /* Disconnect: read EOF, error, or peer half-closed */
                if (nr <= 0 || (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    active--;
                    printf("  server: fd=%d closed (active=%d)\n", fd, active);
                    if (active == 0 && total_seen == NUM_CLIENTS)
                        goto done;
                }
            }
        }
    }
done:
    close(epfd);
    close(lfd);
    waitpid(cpid, NULL, 0);

    puts("\n  Event loop pattern:");
    puts("  1. epoll_create1 — one epoll instance per thread/loop");
    puts("  2. accept + epoll_ctl(ADD) — register each client fd");
    puts("  3. epoll_wait — block; only ready fds come back");
    puts("  4. read/write; epoll_ctl(DEL) + close on disconnect");
    puts("  EPOLLRDHUP — peer half-closed; cleaner than checking read()==0 alone\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    section1_concepts();
    section2_select();
    section3_poll();
    section4_epoll();
    section5_nonblocking();
    section6_echo_server();
    return 0;
}
