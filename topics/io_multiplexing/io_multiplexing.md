# I/O Multiplexing

Monitor multiple file descriptors simultaneously; block until any is ready.
Requires `#define _GNU_SOURCE` (for epoll) and `<sys/select.h>`, `<poll.h>`, `<sys/epoll.h>`.

## When to use each API

| API | Standard | FD limit | Cost per wakeup | Use when |
|-----|----------|----------|-----------------|----------|
| `select()` | POSIX | `FD_SETSIZE` (1024) | O(max-fd) | Portability, handful of fds |
| `poll()` | POSIX | unlimited | O(nfds) | POSIX, moderate fd count |
| `epoll()` | Linux | unlimited | O(ready events) | High fd count, production servers |

## select()

```c
#include <sys/select.h>

fd_set rset, wset;
FD_ZERO(&rset);
FD_SET(fd_a, &rset);
FD_SET(fd_b, &rset);
int nfds = MAX(fd_a, fd_b) + 1;

struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };   /* 2 s timeout; NULL = block forever */
int n = select(nfds, &rset, /*wset*/NULL, /*eset*/NULL, &tv);
if (n < 0)  { perror("select"); }
if (n == 0) { /* timeout */ }

if (FD_ISSET(fd_a, &rset)) { /* fd_a is readable */ }
if (FD_ISSET(fd_b, &rset)) { /* fd_b is readable */ }
```

**Gotchas:**
- Rebuild `fd_set` from scratch before every call — select() modifies it.
- `nfds` is the highest fd + 1, not the count.
- FDs ≥ `FD_SETSIZE` are silently ignored (usually 1024).

## poll()

```c
#include <poll.h>

struct pollfd fds[] = {
    { .fd = fd_a, .events = POLLIN  },
    { .fd = fd_b, .events = POLLIN | POLLOUT },
};
int n = poll(fds, 2, 2000);   /* timeout in ms; -1 = block forever */

for (int i = 0; i < 2; i++) {
    if (fds[i].revents & POLLIN)  { /* readable */ }
    if (fds[i].revents & POLLOUT) { /* writable */ }
    if (fds[i].revents & POLLERR) { /* error    */ }
}

/* Disable a slot without shrinking the array */
fds[1].fd = -1;   /* poll ignores negative fds */
```

**events / revents flags:**

| Flag | Meaning |
|------|---------|
| `POLLIN` | Data available to read |
| `POLLOUT` | Write buffer has space |
| `POLLERR` | Error condition (revents only) |
| `POLLHUP` | Hang-up (revents only) |
| `POLLRDHUP` | Peer closed write half (Linux) |
| `POLLNVAL` | fd not open (revents only) |

## epoll

```c
#include <sys/epoll.h>

/* 1. Create the interest table */
int epfd = epoll_create1(EPOLL_CLOEXEC);

/* 2. Register fds */
struct epoll_event ev;
ev.events  = EPOLLIN;      /* or EPOLLIN | EPOLLOUT | EPOLLRDHUP */
ev.data.fd = client_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

/* 3. Wait */
struct epoll_event events[64];
int n = epoll_wait(epfd, events, 64, -1);   /* -1 = block forever */
for (int i = 0; i < n; i++) {
    int fd = events[i].data.fd;
    if (events[i].events & EPOLLIN)   { /* readable */ }
    if (events[i].events & EPOLLRDHUP){ /* peer half-closed */ }
}

/* 4. Remove when done */
epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);   /* always before close! */
close(client_fd);
close(epfd);
```

**epoll_ctl operations:**

| Op | Meaning |
|----|---------|
| `EPOLL_CTL_ADD` | Add fd to interest set |
| `EPOLL_CTL_MOD` | Change events for a registered fd |
| `EPOLL_CTL_DEL` | Remove fd from interest set |

**epoll_event.data union:** use `.fd` (int), `.ptr` (void\*), or `.u64` to identify the event.

## O_NONBLOCK

A non-blocking fd returns `EAGAIN`/`EWOULDBLOCK` instead of blocking:

```c
#include <fcntl.h>

/* Set after open */
int flags = fcntl(fd, F_GETFL);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);

/* Set at open time */
int fd = open(path, O_RDONLY | O_NONBLOCK);

/* Check the result */
ssize_t n = read(fd, buf, len);
if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    /* no data right now — try again later */
}
```

`EAGAIN == EWOULDBLOCK` on Linux (both equal 11).

## Edge-triggered epoll (EPOLLET)

```c
ev.events = EPOLLIN | EPOLLET;    /* edge-triggered */
```

- **Level-triggered (default):** notified while data is available.
- **Edge-triggered:** notified once when new data arrives.
- ET **requires** O_NONBLOCK and a read loop until EAGAIN:

```c
/* ET read loop — drain all available data */
for (;;) {
    ssize_t n = read(fd, buf, sizeof buf);
    if (n < 0) {
        if (errno == EAGAIN) break;   /* all data consumed */
        /* real error */
        break;
    }
    if (n == 0) { /* EOF — peer closed */ break; }
    /* process buf[0..n) */
}
```

## Non-blocking accept drain loop

```c
/* On EPOLLIN for the listening socket, drain all pending connections */
for (;;) {
    struct sockaddr_in peer;
    socklen_t plen = sizeof peer;
    int cfd = accept(lfd, (struct sockaddr *)&peer, &plen);
    if (cfd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;  /* all accepted */
        perror("accept"); break;
    }
    /* register cfd with epoll */
    set_nonblocking(cfd);
    struct epoll_event cev = { .events = EPOLLIN | EPOLLRDHUP, .data.fd = cfd };
    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &cev);
}
```

## Disconnect detection

```c
if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}
/* Also: read() returning 0 = EOF; < 0 (not EAGAIN) = error */
```

`EPOLLRDHUP` (Linux 2.6.17+) fires when the peer closes the write half — cleaner than relying solely on `read() == 0`.

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Forgetting to rebuild `fd_set` before each `select()` | Rebuild in every loop iteration |
| FD ≥ `FD_SETSIZE` with select | Switch to poll or epoll |
| Not draining all data in edge-triggered mode | Loop read() until EAGAIN |
| `epoll_ctl(DEL)` after `close()` | Always DEL before close; closed fd auto-removed but avoids races |
| Storing raw fd in `epoll_event.data` and reusing it | Prefer `.data.ptr` with a context struct |
| Using epoll from multiple threads on same epfd without locking | One thread per epoll loop, or use `EPOLLONESHOT` |
