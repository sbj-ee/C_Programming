# Terminal I/O (termios)

The terminal line discipline (tty) buffers input, echoes characters, and handles
special keys. `<termios.h>` lets you read and modify these settings.

Requires `#define _GNU_SOURCE` (for `cfmakeraw`) and `<termios.h>`, `<sys/ioctl.h>`, `<fcntl.h>`.

## struct termios fields

```c
struct termios {
    tcflag_t c_iflag;   /* input flags  */
    tcflag_t c_oflag;   /* output flags */
    tcflag_t c_cflag;   /* control flags (baud, data bits) */
    tcflag_t c_lflag;   /* local flags  */
    cc_t     c_cc[NCCS];/* special characters */
};
```

### Key c_lflag bits

| Flag | Default | Meaning |
|------|---------|---------|
| `ICANON` | on | Line-buffered input; deliver after Enter |
| `ECHO` | on | Echo typed characters |
| `ECHOE` | on | Echo backspace as BS-SP-BS |
| `ISIG` | on | Generate signals for Ctrl-C, Ctrl-Z |
| `IEXTEN` | on | Enable extended processing (Ctrl-V, etc.) |

### Key c_iflag bits

| Flag | Default | Meaning |
|------|---------|---------|
| `ICRNL` | on | Translate CR→LF on input |
| `IXON` | on | Enable XON/XOFF flow control (Ctrl-S/Q) |
| `BRKINT` | on | SIGINT on break |

### c_cc[] entries

| Index | Default char | Meaning |
|-------|-------------|---------|
| `VMIN` | 1 | Minimum bytes for a non-canonical read |
| `VTIME` | 0 | Timeout in tenths of a second (0 = no timeout) |
| `VINTR` | Ctrl-C | Send SIGINT |
| `VEOF` | Ctrl-D | EOF in canonical mode |
| `VSUSP` | Ctrl-Z | Send SIGTSTP |

## Get / set terminal attributes

```c
#include <termios.h>

struct termios t;
tcgetattr(STDIN_FILENO, &t);   /* read current settings */

/* Modify ... */
t.c_lflag &= ~ECHO;            /* disable echo */

/* Apply */
/* TCSANOW:   apply immediately
 * TCSADRAIN: apply after all output is written
 * TCSAFLUSH: apply after output written; discard unread input */
tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);
```

## Raw mode

Raw mode gives byte-at-a-time input with no echo or signal processing.

```c
void enter_raw(int fd, struct termios *saved) {
    tcgetattr(fd, saved);

    struct termios raw = *saved;
    cfmakeraw(&raw);        /* clears ECHO, ICANON, ISIG, IEXTEN (c_lflag)
                             * BRKINT, ICRNL, INPCK, ISTRIP, IXON (c_iflag)
                             * OPOST (c_oflag); sets CS8 (c_cflag) */
    raw.c_cc[VMIN]  = 1;   /* return after 1 byte */
    raw.c_cc[VTIME] = 0;   /* no timeout */
    tcsetattr(fd, TCSAFLUSH, &raw);
}

void leave_raw(int fd, const struct termios *saved) {
    tcsetattr(fd, TCSAFLUSH, saved);
}
```

### Manual raw mode (without cfmakeraw)

```c
t.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
t.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
t.c_oflag &= ~OPOST;
t.c_cflag |=  CS8;
t.c_cc[VMIN]  = 1;
t.c_cc[VTIME] = 0;
```

### VMIN / VTIME combinations

| VMIN | VTIME | Behaviour |
|------|-------|-----------|
| > 0 | 0 | Block until VMIN bytes arrive |
| > 0 | > 0 | Block until VMIN bytes arrive, timer starts after first byte |
| 0 | > 0 | Return after VTIME tenths-sec (or earlier if bytes arrive) |
| 0 | 0 | Return immediately with whatever is available (non-blocking) |

## Reading single keystrokes

```c
unsigned char c;
ssize_t n = read(STDIN_FILENO, &c, 1);

/* Arrow keys send escape sequences: ESC [ A/B/C/D */
if (c == 27) {
    unsigned char seq[2];
    read(STDIN_FILENO, seq, 2);
    if (seq[0] == '[') {
        switch (seq[1]) {
            case 'A': /* Up    */ break;
            case 'B': /* Down  */ break;
            case 'C': /* Right */ break;
            case 'D': /* Left  */ break;
        }
    }
}
```

In raw mode use `write()` not `printf()` and end lines with `\r\n` (OPOST is off).

## Restore-on-exit pattern

```c
static struct termios g_saved;

static void restore(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved);
}

static void sig_handler(int sig) {
    restore();
    signal(sig, SIG_DFL);
    raise(sig);   /* re-deliver for correct exit status */
}

/* In main: */
tcgetattr(STDIN_FILENO, &g_saved);
atexit(restore);
signal(SIGINT,  sig_handler);
signal(SIGTERM, sig_handler);
signal(SIGHUP,  sig_handler);
/* enter raw mode... */
```

## ANSI escape codes

All sequences begin with `ESC` (`\x1b`) followed by `[`:

```c
#define ESC "\x1b["

/* Text attributes (SGR — Select Graphic Rendition) */
printf(ESC "0m");     /* reset all       */
printf(ESC "1m");     /* bold            */
printf(ESC "4m");     /* underline       */
printf(ESC "2m");     /* dim             */
printf(ESC "7m");     /* reverse video   */

/* 8 foreground colours (30-37) */
printf(ESC "31m");    /* red             */
printf(ESC "32m");    /* green           */
printf(ESC "33m");    /* yellow          */
printf(ESC "34m");    /* blue            */

/* 8 background colours (40-47) */
printf(ESC "41m");    /* red bg          */

/* 256-colour mode */
printf(ESC "38;5;%dm", n);    /* fg colour n (0-255) */
printf(ESC "48;5;%dm", n);    /* bg colour n (0-255) */

/* True colour (24-bit) */
printf(ESC "38;2;%d;%d;%dm", r, g, b);   /* fg */
printf(ESC "48;2;%d;%d;%dm", r, g, b);   /* bg */
```

### Cursor control

```c
printf(ESC "H");           /* move cursor home (1,1) */
printf(ESC "%d;%dH", r, c);/* move to row r, col c (1-indexed) */
printf(ESC "2J");          /* clear screen */
printf(ESC "2K\r");        /* erase current line */
printf(ESC "%dA", n);      /* move up n rows    */
printf(ESC "%dB", n);      /* move down n rows  */
printf(ESC "%dC", n);      /* move right n cols */
printf(ESC "%dD", n);      /* move left n cols  */
printf(ESC "s");           /* save cursor position   */
printf(ESC "u");           /* restore cursor position */
printf(ESC "?25l");        /* hide cursor */
printf(ESC "?25h");        /* show cursor */
```

## Terminal size

```c
#include <sys/ioctl.h>

struct winsize ws;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
printf("rows=%u cols=%u\n", ws.ws_row, ws.ws_col);
```

Handle `SIGWINCH` to redraw when the window is resized:

```c
static volatile sig_atomic_t g_resized = 0;
signal(SIGWINCH, sigwinch_handler);   /* handler sets g_resized = 1 */

/* In event loop: */
if (g_resized) { g_resized = 0; ioctl(fd, TIOCGWINSZ, &ws); redraw(); }
```

## isatty check

Always check before entering raw mode — stdin may be a pipe:

```c
if (!isatty(STDIN_FILENO)) {
    /* stdin is redirected; open the controlling terminal directly */
    int ttyfd = open("/dev/tty", O_RDWR);
}
```

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Crashing in raw mode leaves terminal broken | `atexit` + signal handlers to restore |
| Using `printf` / `\n` in raw mode | Use `write()` and `\r\n` (OPOST is off) |
| Forgetting to check `isatty` | Program hangs when input is piped |
| Not handling arrow keys (multi-byte escape sequences) | Read and parse the 3-byte sequence |
| Hard-coding terminal dimensions | Query `TIOCGWINSZ` and handle `SIGWINCH` |
| `VTIME` timeout in tenths, not milliseconds | `VTIME=1` = 100 ms, not 1 ms |
