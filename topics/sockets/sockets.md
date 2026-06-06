# Sockets

Requires `#define _POSIX_C_SOURCE 200809L` and:
`<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`, `<netdb.h>`, `<sys/un.h>`.

## TCP server skeleton

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int lfd = socket(AF_INET, SOCK_STREAM, 0);

// Allow immediate restart after crash (avoid TIME_WAIT)
int yes = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

struct sockaddr_in addr = {
    .sin_family      = AF_INET,
    .sin_addr.s_addr = htonl(INADDR_ANY),   // all interfaces
    .sin_port        = htons(8080),
};
bind(lfd, (struct sockaddr *)&addr, sizeof addr);
listen(lfd, 128);                            // 128 = backlog queue depth

// Accept loop
for (;;) {
    struct sockaddr_in peer;
    socklen_t plen = sizeof peer;
    int cfd = accept(lfd, (struct sockaddr *)&peer, &plen);
    if (cfd < 0) { perror("accept"); continue; }

    // Handle cfd in a thread or child process
    handle_client(cfd);
    close(cfd);
}
close(lfd);
```

## TCP client skeleton

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in addr = {
    .sin_family      = AF_INET,
    .sin_port        = htons(8080),
};
inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
    perror("connect");
    close(fd);
    return -1;
}

send(fd, "hello", 5, 0);
char buf[64];
ssize_t n = recv(fd, buf, sizeof buf - 1, 0);
buf[n] = '\0';
close(fd);
```

## Sending and receiving reliably

TCP is a **byte stream** — `send`/`recv` may transfer fewer bytes than requested.

```c
// Send all bytes (loop until done)
ssize_t send_all(int fd, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, (char *)buf + sent, len - sent, MSG_NOSIGNAL);
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return (ssize_t)sent;
}

// Receive exactly n bytes
ssize_t recv_all(int fd, void *buf, size_t n) {
    size_t got = 0;
    while (got < n) {
        ssize_t r = recv(fd, (char *)buf + got, n - got, 0);
        if (r <= 0) return (ssize_t)got;
        got += (size_t)r;
    }
    return (ssize_t)got;
}
```

## Message framing

TCP has no message boundaries. Use one of:
- **Length prefix**: send a 4-byte big-endian length before each message
- **Delimiter**: newlines or `\0` (text protocols, HTTP headers)
- **Fixed size**: all messages the same number of bytes

```c
// Length-prefix example
uint32_t len = htonl((uint32_t)strlen(msg));
send_all(fd, &len,  4);
send_all(fd, msg,   strlen(msg));

// Receive
uint32_t nlen;
recv_all(fd, &nlen, 4);
uint32_t msglen = ntohl(nlen);
char *buf = malloc(msglen + 1);
recv_all(fd, buf, msglen);
buf[msglen] = '\0';
```

## UDP

```c
// Server
int fd = socket(AF_INET, SOCK_DGRAM, 0);
bind(fd, (struct sockaddr *)&addr, sizeof addr);

char buf[1024];
struct sockaddr_in from;
socklen_t flen = sizeof from;
ssize_t n = recvfrom(fd, buf, sizeof buf - 1, 0,
                     (struct sockaddr *)&from, &flen);
sendto(fd, "ack", 3, 0, (struct sockaddr *)&from, flen);

// Client — no connect/accept
sendto(fd, msg, len, 0, (struct sockaddr *)&server, sizeof server);
```

## Unix domain sockets (AF_UNIX)

In-process IPC — no network stack, higher throughput, lower latency.

```c
#include <sys/un.h>

int fd = socket(AF_UNIX, SOCK_STREAM, 0);
struct sockaddr_un addr;
memset(&addr, 0, sizeof addr);
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, "/tmp/myapp.sock", sizeof addr.sun_path - 1);

// Server: bind + listen + accept (same as TCP)
unlink("/tmp/myapp.sock");            // remove stale socket
bind(fd, (struct sockaddr *)&addr, sizeof addr);
listen(fd, 8);

// Client: just connect
connect(fd, (struct sockaddr *)&addr, sizeof addr);

// Cleanup
unlink("/tmp/myapp.sock");
```

## getaddrinfo — portable address resolution

```c
#include <netdb.h>

struct addrinfo hints = {
    .ai_family   = AF_UNSPEC,     // IPv4 or IPv6
    .ai_socktype = SOCK_STREAM,
};
struct addrinfo *res;
int rc = getaddrinfo("example.com", "80", &hints, &res);
if (rc) { fprintf(stderr, "%s\n", gai_strerror(rc)); }

// Try each result until one connects
for (struct addrinfo *p = res; p; p = p->ai_next) {
    int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd < 0) continue;
    if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
        // success
        break;
    }
    close(fd);
}
freeaddrinfo(res);
```

## Byte order

Network byte order is big-endian. Convert before sending, after receiving.

```c
htons(x)   // host → network, 16-bit
htonl(x)   // host → network, 32-bit
ntohs(x)   // network → host, 16-bit
ntohl(x)   // network → host, 32-bit

// Text form
inet_pton(AF_INET, "192.168.1.1", &addr.sin_addr);  // string → binary
inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof buf); // binary → string
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Assuming `recv` returns a full message | Loop with `recv_all`; use framing |
| SIGPIPE on write to closed socket | `signal(SIGPIPE, SIG_IGN)` + check `errno == EPIPE`, or use `MSG_NOSIGNAL` |
| Not setting `SO_REUSEADDR` | Port stuck in TIME_WAIT for 2 min after restart |
| Forgetting `htons`/`htonl` | Silent corruption on little-endian hosts |
| Leaking socket FDs | Always `close(fd)` in every code path |
| Blocking `accept` preventing other work | Use `select`/`poll`/`epoll` for multiplexing |
