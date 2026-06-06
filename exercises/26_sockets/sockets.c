/*
 * Exercise 26: Sockets
 *
 * A socket is a bidirectional I/O endpoint.  Berkeley sockets
 * (POSIX) unify network communication and local IPC under a
 * single read/write-like interface.
 *
 * Topics:
 *   - Socket API: socket/bind/listen/accept/connect/send/recv/close
 *   - TCP (SOCK_STREAM)  — reliable, ordered, connection-oriented
 *   - UDP (SOCK_DGRAM)   — unreliable, connectionless, datagram
 *   - AF_INET loopback TCP client/server (self-contained demo)
 *   - AF_UNIX (Unix domain) — IPC without the network stack
 *   - getaddrinfo — protocol-agnostic address resolution
 *   - Pitfalls — SIGPIPE, partial reads, byte order, SO_REUSEADDR
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>       /* struct sockaddr_un */
#include <netinet/in.h>   /* struct sockaddr_in, INADDR_LOOPBACK */
#include <arpa/inet.h>    /* inet_ntop, ntohs/htons */
#include <netdb.h>        /* getaddrinfo, freeaddrinfo */
#include <errno.h>
#include <signal.h>
#include <stdint.h>

/* ──────────────────────────────────────────────────────────
 * Section 1: Concepts
 *
 * Socket taxonomy:
 *
 *   Domain (address family)
 *     AF_INET   — IPv4 network
 *     AF_INET6  — IPv6 network
 *     AF_UNIX   — local IPC via a filesystem path
 *
 *   Type
 *     SOCK_STREAM  — TCP: reliable, ordered, connection-oriented
 *     SOCK_DGRAM   — UDP: fast, connectionless, may drop/reorder
 *
 * TCP connection lifecycle:
 *
 *   SERVER                            CLIENT
 *   socket()                          socket()
 *   bind()    ← assign address        |
 *   listen()  ← accept queue          |
 *   accept()  ← blocks until ─────── connect()
 *   recv()/send()  ←────────────────► send()/recv()
 *   close()                           close()
 *
 * Byte order:
 *   Network byte order is big-endian.
 *   htons/htonl — host-to-network (for port/address)
 *   ntohs/ntohl — network-to-host (when reading received values)
 * ────────────────────────────────────────────────────────── */

static void section_concepts(void) {
    printf("=== 1. Concepts ===\n");

    /* Show that htons converts on little-endian machines */
    uint16_t port_h = 8080;
    uint16_t port_n = htons(port_h);
    printf("  port 8080 host order:    0x%04x\n", port_h);
    printf("  port 8080 network order: 0x%04x  (%s)\n", port_n,
           port_h == port_n ? "same (big-endian host)" : "swapped (little-endian host)");

    /* Show INADDR_LOOPBACK in dotted-decimal */
    struct in_addr lo = { .s_addr = htonl(INADDR_LOOPBACK) };
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &lo, buf, sizeof buf);
    printf("  INADDR_LOOPBACK = %s\n", buf);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 2: TCP loopback demo
 *
 * The demo is fully self-contained:
 *   - Parent creates a listening socket on port 0 (OS picks)
 *   - Parent forks; child becomes the TCP client
 *   - Parent hands the assigned port to the child via a pipe
 *   - They exchange three messages and the child exits
 *
 * This reuses the pipe + fork pattern from exercise 25.
 * ────────────────────────────────────────────────────────── */

/* Create a listening socket bound to 127.0.0.1:0.
 * Returns the socket fd; writes the assigned port to *port. */
static int make_server(uint16_t *port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); exit(1); }

    /* SO_REUSEADDR lets us re-bind immediately after restart
     * (without it the port stays in TIME_WAIT for ~2 min). */
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port        = htons(0),   /* 0 = OS picks a free port */
    };
    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind"); exit(1);
    }

    /* Discover the port the OS assigned */
    socklen_t alen = sizeof addr;
    getsockname(fd, (struct sockaddr *)&addr, &alen);
    *port = ntohs(addr.sin_port);

    if (listen(fd, 1) < 0) { perror("listen"); exit(1); }
    return fd;
}

/* Connect to 127.0.0.1:port; return the connected socket fd. */
static int make_client(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port        = htons(port),
    };
    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("connect"); exit(1);
    }
    return fd;
}

/* Receive exactly n bytes into buf; handles partial reads. */
static ssize_t recv_all(int fd, char *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t r = recv(fd, buf + total, n - total, 0);
        if (r <= 0) return (ssize_t)total;
        total += (size_t)r;
    }
    return (ssize_t)total;
}

/* Simple length-prefixed framing: 1-byte length then payload */
static void send_msg(int fd, const char *msg) {
    uint8_t len = (uint8_t)strlen(msg);
    send(fd, &len, 1, MSG_NOSIGNAL);
    send(fd, msg,  len, MSG_NOSIGNAL);
}

static int recv_msg(int fd, char *buf, size_t bufsz) {
    uint8_t len;
    if (recv_all(fd, (char *)&len, 1) != 1) return -1;
    if (len >= bufsz) len = (uint8_t)(bufsz - 1);
    ssize_t n = recv_all(fd, buf, len);
    buf[n] = '\0';
    return (int)n;
}

#define NMSGS 3

static const char *client_msgs[NMSGS] = { "hello",  "world",  "bye"   };
static const char *server_echo_fmt    = "[echo] %s";

static void section_tcp(void) {
    printf("=== 2. TCP loopback demo ===\n");

    /* Server setup: listen on loopback:0 */
    uint16_t port;
    int lfd = make_server(&port);
    printf("  server listening on 127.0.0.1:%u\n", port);

    /* Pipe: parent (server) sends the port to the child (client) */
    int pp[2];
    pipe(pp);

    fflush(stdout);
    pid_t pid = fork();

    /* ── child: TCP client ── */
    if (pid == 0) {
        close(lfd);
        close(pp[1]);

        uint16_t p;
        read(pp[0], &p, sizeof p);
        close(pp[0]);

        int cfd = make_client(p);
        printf("  client connected to port %u\n", p);

        char buf[64];
        for (int i = 0; i < NMSGS; i++) {
            send_msg(cfd, client_msgs[i]);
            recv_msg(cfd, buf, sizeof buf);
            printf("  client: sent \"%s\"  got \"%s\"\n", client_msgs[i], buf);
        }
        close(cfd);
        exit(0);
    }

    /* ── parent: TCP server ── */
    close(pp[0]);
    write(pp[1], &port, sizeof port);
    close(pp[1]);

    int cfd = accept(lfd, NULL, NULL);
    printf("  server accepted connection\n");

    char buf[64], echo[80];
    for (int i = 0; i < NMSGS; i++) {
        recv_msg(cfd, buf, sizeof buf);
        snprintf(echo, sizeof echo, server_echo_fmt, buf);
        send_msg(cfd, echo);
        printf("  server: echoed \"%s\"\n", buf);
    }
    close(cfd);
    close(lfd);
    waitpid(pid, NULL, 0);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 3: UDP datagrams
 *
 * UDP is connectionless: no accept/connect handshake.
 * send/recv become sendto/recvfrom — each call carries the
 * peer's address.
 *
 * Use cases: DNS, NTP, game state, video streaming — anywhere
 * low latency matters more than guaranteed delivery.
 * ────────────────────────────────────────────────────────── */

static void section_udp(void) {
    printf("=== 3. UDP datagrams ===\n");

    /* Server socket */
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in saddr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port        = htons(0),
    };
    bind(sfd, (struct sockaddr *)&saddr, sizeof saddr);
    socklen_t alen = sizeof saddr;
    getsockname(sfd, (struct sockaddr *)&saddr, &alen);
    uint16_t port = ntohs(saddr.sin_port);

    /* Pipe: hand port to child */
    int pp[2]; pipe(pp);
    fflush(stdout);
    pid_t pid = fork();

    if (pid == 0) {
        /* ── client ── */
        close(sfd); close(pp[1]);
        uint16_t p; read(pp[0], &p, sizeof p); close(pp[0]);

        int cfd = socket(AF_INET, SOCK_DGRAM, 0);
        struct sockaddr_in dst = {
            .sin_family      = AF_INET,
            .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
            .sin_port        = htons(p),
        };
        const char *msg = "UDP ping";
        sendto(cfd, msg, strlen(msg), 0, (struct sockaddr *)&dst, sizeof dst);
        printf("  client sent: \"%s\"\n", msg);

        char buf[64];
        struct sockaddr_in from;
        socklen_t fl = sizeof from;
        ssize_t n = recvfrom(cfd, buf, sizeof buf - 1, 0,
                             (struct sockaddr *)&from, &fl);
        buf[n] = '\0';
        printf("  client got:  \"%s\"\n", buf);
        close(cfd);
        exit(0);
    }

    /* ── server ── */
    close(pp[0]);
    write(pp[1], &port, sizeof port); close(pp[1]);

    char buf[64];
    struct sockaddr_in from;
    socklen_t fl = sizeof from;
    ssize_t n = recvfrom(sfd, buf, sizeof buf - 1, 0,
                         (struct sockaddr *)&from, &fl);
    buf[n] = '\0';
    printf("  server received: \"%s\"\n", buf);

    /* Echo back to sender */
    char reply[80];
    int rlen = snprintf(reply, sizeof reply, "[udp-echo] %s", buf);
    sendto(sfd, reply, (size_t)rlen, 0, (struct sockaddr *)&from, fl);
    close(sfd);
    waitpid(pid, NULL, 0);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 4: Unix domain sockets (AF_UNIX)
 *
 * AF_UNIX sockets use a filesystem path instead of an IP+port.
 * The kernel transfers data in shared memory — no network stack,
 * so throughput is much higher and latency much lower than loopback.
 *
 * Use for IPC between processes on the same host
 * (databases, display servers, init daemons, etc.).
 *
 * API differences from AF_INET:
 *   - struct sockaddr_un with sun_path (filesystem path)
 *   - No bind/connect to IP or port — just the path
 *   - Remove the socket file with unlink() when done
 * ────────────────────────────────────────────────────────── */

#define SOCK_PATH "/tmp/ex26_demo.sock"

static void section_unix(void) {
    printf("=== 4. Unix domain sockets ===\n");

    unlink(SOCK_PATH);   /* remove stale socket from a previous run */

    /* Server */
    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un saddr;
    memset(&saddr, 0, sizeof saddr);
    saddr.sun_family = AF_UNIX;
    strncpy(saddr.sun_path, SOCK_PATH, sizeof saddr.sun_path - 1);
    bind(lfd, (struct sockaddr *)&saddr, sizeof saddr);
    listen(lfd, 1);
    printf("  server listening on %s\n", SOCK_PATH);

    fflush(stdout);
    pid_t pid = fork();

    if (pid == 0) {
        /* ── client ── */
        close(lfd);
        int cfd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un caddr;
        memset(&caddr, 0, sizeof caddr);
        caddr.sun_family = AF_UNIX;
        strncpy(caddr.sun_path, SOCK_PATH, sizeof caddr.sun_path - 1);
        connect(cfd, (struct sockaddr *)&caddr, sizeof caddr);
        send_msg(cfd, "unix hello");
        char buf[64];
        recv_msg(cfd, buf, sizeof buf);
        printf("  client got: \"%s\"\n", buf);
        close(cfd);
        exit(0);
    }

    /* ── server ── */
    int cfd = accept(lfd, NULL, NULL);
    char buf[64], echo[80];
    recv_msg(cfd, buf, sizeof buf);
    printf("  server got: \"%s\"\n", buf);
    int n = snprintf(echo, sizeof echo, "[unix-echo] %s", buf);
    send_msg(cfd, echo);
    (void)n;
    close(cfd);
    close(lfd);
    unlink(SOCK_PATH);
    waitpid(pid, NULL, 0);

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 5: getaddrinfo — portable address resolution
 *
 * getaddrinfo() resolves a hostname + service into a linked list
 * of struct addrinfo, each describing one viable socket address.
 * It handles IPv4, IPv6, and name resolution transparently.
 *
 * Always prefer getaddrinfo over inet_aton/gethostbyname —
 * those are not thread-safe and IPv4-only.
 *
 * struct addrinfo {
 *     int              ai_flags;
 *     int              ai_family;    // AF_INET or AF_INET6
 *     int              ai_socktype;  // SOCK_STREAM or SOCK_DGRAM
 *     int              ai_protocol;
 *     socklen_t        ai_addrlen;
 *     struct sockaddr *ai_addr;
 *     char            *ai_canonname;
 *     struct addrinfo *ai_next;      // next in list
 * };
 * ────────────────────────────────────────────────────────── */

static void section_getaddrinfo(void) {
    printf("=== 5. getaddrinfo ===\n");

    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;    /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;   /* for bind() — fills in local IP */

    int rc = getaddrinfo("localhost", "80", &hints, &res);
    if (rc != 0) {
        printf("  getaddrinfo: %s\n", gai_strerror(rc));
        return;
    }

    int count = 0;
    for (p = res; p != NULL; p = p->ai_next) {
        char host[128];
        int r = getnameinfo(p->ai_addr, p->ai_addrlen,
                            host, sizeof host, NULL, 0, NI_NUMERICHOST);
        if (r == 0)
            printf("  result %d: family=%-8s addr=%s\n", ++count,
                   p->ai_family == AF_INET  ? "AF_INET" :
                   p->ai_family == AF_INET6 ? "AF_INET6" : "other",
                   host);
    }
    freeaddrinfo(res);

    /* Pattern: iterate the list, try each address until one works */
    printf("  usage: iterate list, socket()+connect() until success\n");

    printf("\n");
}

/* ──────────────────────────────────────────────────────────
 * Section 6: Pitfalls
 *
 * 1. SIGPIPE
 *    Writing to a socket whose remote end closed delivers SIGPIPE
 *    (default action: terminate the process).  Prevent with:
 *      a. signal(SIGPIPE, SIG_IGN) in main
 *      b. MSG_NOSIGNAL flag on each send() call (Linux)
 *      c. SO_NOSIGPIPE socket option (macOS/BSD)
 *    Check send() return value or errno == EPIPE after ignoring.
 *
 * 2. Partial reads / partial writes
 *    send() and recv() may transfer fewer bytes than requested.
 *    TCP is a byte stream — it has no message boundaries.
 *    Always loop until all bytes are sent/received.
 *    Use a framing protocol (length-prefix, newline delimiter,
 *    or fixed-size messages) to delineate application messages.
 *
 * 3. SO_REUSEADDR
 *    After a server crashes, the port may stay in TIME_WAIT for
 *    up to 2 minutes.  SO_REUSEADDR lets you rebind immediately.
 *    Set it before every bind() call.
 *
 * 4. Byte order
 *    Always convert multi-byte values with htons/htonl before
 *    sending, ntohs/ntohl after receiving.  Forgetting produces
 *    silent corruption on little-endian hosts communicating with
 *    big-endian hosts (or with each other when the spec says BE).
 *
 * 5. select() / poll() — handling multiple sockets
 *    A single-threaded server uses select() or poll() to wait on
 *    multiple fds simultaneously instead of blocking on one.
 *    epoll (Linux) is the scalable alternative for many connections.
 * ────────────────────────────────────────────────────────── */

static void section_pitfalls(void) {
    printf("=== 6. Pitfalls ===\n");

    /* Demonstrate SIGPIPE protection via MSG_NOSIGNAL */
    signal(SIGPIPE, SIG_IGN);
    printf("  SIGPIPE suppressed with SIG_IGN (also use MSG_NOSIGNAL)\n");

    /* Show SO_REUSEADDR in practice (already used in make_server) */
    printf("  SO_REUSEADDR: set before bind() to allow immediate restart\n");

    printf("\n  Byte order conversions:\n");
    uint32_t h = 0x12345678;
    uint32_t n = htonl(h);
    printf("    host 0x%08x → network 0x%08x\n", h, n);
    printf("    network 0x%08x → host 0x%08x\n", n, ntohl(n));

    printf("\n  Partial read/write: always loop:\n");
    printf("    while (sent < total) sent += send(fd, buf+sent, total-sent, 0)\n");

    printf("\n  I/O multiplexing:\n");
    printf("    select()  — portable, O(n) scan of fd_set\n");
    printf("    poll()    — no fd_set size limit, still O(n)\n");
    printf("    epoll()   — Linux, O(1) for active fds, scales to millions\n");

    printf("\n");
}

/* ─────────────────────── main ─────────────────────────── */

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    signal(SIGPIPE, SIG_IGN);   /* prevent crash on broken-pipe write */

    section_concepts();
    section_tcp();
    section_udp();
    section_unix();
    section_getaddrinfo();
    section_pitfalls();
    return 0;
}
