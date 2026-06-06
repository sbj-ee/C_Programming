/*
 * Exercise 33 — Terminal I/O
 *
 * <termios.h> controls the terminal line discipline: how the kernel
 * processes characters between the hardware and the program.
 * Raw mode lets applications read single keystrokes, implement full-screen
 * UIs, and control every aspect of the display via ANSI escape codes.
 *
 * Build: make
 * Run:   ./terminal
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>

/* ── ANSI escape code helpers ─────────────────────────────────────────────── */

#define ESC "\x1b["

/* Text attributes */
#define ANSI_RESET      ESC "0m"
#define ANSI_BOLD       ESC "1m"
#define ANSI_DIM        ESC "2m"
#define ANSI_UNDERLINE  ESC "4m"

/* Foreground colours (30-37) */
#define ANSI_BLACK      ESC "30m"
#define ANSI_RED        ESC "31m"
#define ANSI_GREEN      ESC "32m"
#define ANSI_YELLOW     ESC "33m"
#define ANSI_BLUE       ESC "34m"
#define ANSI_MAGENTA    ESC "35m"
#define ANSI_CYAN       ESC "36m"
#define ANSI_WHITE      ESC "37m"

/* Background colours (40-47) */
#define ANSI_BG_RED     ESC "41m"
#define ANSI_BG_GREEN   ESC "42m"
#define ANSI_BG_BLUE    ESC "44m"

/* Cursor control */
#define ANSI_CLEAR      ESC "2J" ESC "H"   /* clear screen, home */
#define ANSI_ERASE_LINE ESC "2K" "\r"      /* erase line, carriage return */

/* ── 1. Concepts ─────────────────────────────────────────────────────────── */

static void section1_concepts(void) {
    puts("═══ 1. Terminal I/O Concepts ════\n");

    puts("  The terminal line discipline (tty) sits between hardware and the process.");
    puts("  It buffers input into lines, translates \\n↔\\r\\n, echoes characters, and");
    puts("  handles special keys (Ctrl-C → SIGINT, Ctrl-Z → SIGTSTP, etc.).\n");

    puts("  Two main modes:");
    puts("    Canonical (cooked): input delivered line-at-a-time after Enter");
    puts("    Raw:               input delivered byte-at-a-time; no echo or processing\n");

    puts("  struct termios fields:");
    puts("    c_iflag   input flags  (ICRNL, IXON, BRKINT …)");
    puts("    c_oflag   output flags (OPOST, ONLCR …)");
    puts("    c_cflag   control flags (baud rate, data bits …)");
    puts("    c_lflag   local flags  (ECHO, ICANON, ISIG, IEXTEN …)");
    puts("    c_cc[]    special characters (VMIN, VTIME, VINTR, VEOF …)\n");

    puts("  Key API:");
    puts("    tcgetattr(fd, &saved)        — read current settings");
    puts("    tcsetattr(fd, TCSAFLUSH, &t) — apply new settings");
    puts("    cfmakeraw(&t)                — convenience: set all raw flags\n");
}

/* ── 2. Inspecting current settings ──────────────────────────────────────── */

static void section2_inspect(void) {
    puts("═══ 2. Inspecting Current Terminal Settings ════\n");

    int fd = STDIN_FILENO;
    if (!isatty(fd)) {
        puts("  stdin is not a tty (piped/redirected) — showing defaults only\n");
        fd = open("/dev/tty", O_RDONLY);   /* open the controlling terminal directly */
        if (fd < 0) { puts("  /dev/tty unavailable; skipping\n"); return; }
    }

    struct termios t;
    tcgetattr(fd, &t);

    printf("  c_lflag bits set:\n");
    struct { tcflag_t bit; const char *name; } lflags[] = {
        { ECHO,    "ECHO"    }, { ECHOE,   "ECHOE"   },
        { ECHOK,   "ECHOK"   }, { ICANON,  "ICANON"  },
        { ISIG,    "ISIG"    }, { IEXTEN,  "IEXTEN"  },
    };
    for (int i = 0; i < 6; i++)
        printf("    %-10s  %s\n", lflags[i].name,
               (t.c_lflag & lflags[i].bit) ? "on" : "off");

    printf("\n  c_iflag bits set:\n");
    struct { tcflag_t bit; const char *name; } iflags[] = {
        { ICRNL, "ICRNL" }, { IXON, "IXON" }, { BRKINT, "BRKINT" },
    };
    for (int i = 0; i < 3; i++)
        printf("    %-10s  %s\n", iflags[i].name,
               (t.c_iflag & iflags[i].bit) ? "on" : "off");

    printf("\n  VMIN=%d  VTIME=%d  (read blocks for VMIN chars or VTIME tenths-sec)\n\n",
           t.c_cc[VMIN], t.c_cc[VTIME]);

    if (fd != STDIN_FILENO) close(fd);
}

/* ── 3. Raw mode ─────────────────────────────────────────────────────────── */

static struct termios g_saved;
static int            g_raw_fd = -1;

static void restore_terminal(void) {
    if (g_raw_fd >= 0)
        tcsetattr(g_raw_fd, TCSAFLUSH, &g_saved);
}

static void enter_raw(int fd) {
    tcgetattr(fd, &g_saved);
    g_raw_fd = fd;
    atexit(restore_terminal);   /* always restore on exit */

    struct termios raw = g_saved;
    cfmakeraw(&raw);            /* clears ECHO, ICANON, ISIG, IEXTEN, etc. */
    raw.c_cc[VMIN]  = 1;        /* read returns after 1 byte */
    raw.c_cc[VTIME] = 0;        /* no timeout */
    tcsetattr(fd, TCSAFLUSH, &raw);
}

static void section3_raw_mode(void) {
    puts("═══ 3. Raw Mode ════\n");

    int fd = STDIN_FILENO;
    if (!isatty(fd)) {
        fd = open("/dev/tty", O_RDWR);   /* open controlling terminal for raw mode */
        if (fd < 0) {
            puts("  stdin is not a tty — raw mode demo skipped\n");
            puts("  In raw mode, read() returns after each keystroke without");
            puts("  waiting for Enter.  Used by editors, TUIs, games.\n");
            return;
        }
    }

    puts("  Entering raw mode — press any 5 keys (q to quit early):\n");
    fflush(stdout);

    enter_raw(fd);

    int count = 0;
    while (count < 5) {
        unsigned char c;
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) break;

        /* Print char info without waiting for newline (raw mode) */
        if (c < 32 || c == 127) {
            /* Control character */
            const char *name = "";
            switch (c) {
                case  3: name = "Ctrl-C"; break;
                case  4: name = "Ctrl-D"; break;
                case  9: name = "Tab";    break;
                case 13: name = "Enter";  break;
                case 27: name = "Esc";    break;
                case 127: name = "Bksp"; break;
            }
            char msg[64];
            int len = snprintf(msg, sizeof msg,
                               "  key %d: byte=0x%02x (%s)\r\n", count + 1, c, name);
            write(fd, msg, (size_t)len);
        } else {
            char msg[64];
            int len = snprintf(msg, sizeof msg,
                               "  key %d: '%c'  (byte=0x%02x)\r\n", count + 1, c, c);
            write(fd, msg, (size_t)len);
        }
        count++;
        if (c == 'q' || c == 'Q' || c == 3 || c == 4) break;
    }

    restore_terminal();
    g_raw_fd = -1;   /* prevent double-restore from atexit */

    puts("\n  Raw mode disabled.\n");
    puts("  cfmakeraw() clears these flags:");
    puts("    c_lflag: ECHO ECHONL ICANON ISIG IEXTEN");
    puts("    c_iflag: BRKINT ICRNL INPCK ISTRIP IXON");
    puts("    c_oflag: OPOST");
    puts("    c_cflag: sets CS8\n");

    if (fd != STDIN_FILENO) close(fd);
}

/* ── 4. ANSI escape codes ─────────────────────────────────────────────────── */

static void section4_ansi(void) {
    puts("═══ 4. ANSI Escape Codes ════\n");

    if (!isatty(STDOUT_FILENO)) {
        puts("  stdout is not a tty — escape codes would appear as literal bytes.");
        puts("  Run ./terminal directly in a terminal to see colours.\n");
    }

    /* Colours */
    printf("  Foreground: "
           ANSI_RED    "red "    ANSI_RESET
           ANSI_GREEN  "green "  ANSI_RESET
           ANSI_YELLOW "yellow " ANSI_RESET
           ANSI_BLUE   "blue "   ANSI_RESET
           ANSI_CYAN   "cyan "   ANSI_RESET
           ANSI_MAGENTA "magenta " ANSI_RESET
           "\n");

    printf("  Background: "
           ANSI_BG_RED   ANSI_WHITE " red "   ANSI_RESET " "
           ANSI_BG_GREEN ANSI_BLACK " green " ANSI_RESET " "
           ANSI_BG_BLUE  ANSI_WHITE " blue "  ANSI_RESET
           "\n");

    printf("  Attributes: "
           ANSI_BOLD      "bold "      ANSI_RESET
           ANSI_DIM       "dim "       ANSI_RESET
           ANSI_UNDERLINE "underline " ANSI_RESET
           "\n\n");

    /* 256-colour mode */
    printf("  256-colour gradient: ");
    for (int i = 0; i < 24; i++)
        printf("\x1b[48;5;%dm  ", 232 + i);
    printf(ANSI_RESET "\n\n");

    puts("  Common escape sequences:");
    puts("    \\x1b[<n>m        — set attribute/colour (SGR)");
    puts("    \\x1b[<r>;<c>H   — move cursor to row r, col c");
    puts("    \\x1b[<n>A/B/C/D — move cursor up/down/right/left");
    puts("    \\x1b[2J         — clear screen");
    puts("    \\x1b[2K         — erase line");
    puts("    \\x1b[?25l/h     — hide/show cursor\n");
}

/* ── 5. Terminal size ─────────────────────────────────────────────────────── */

static void section5_size(void) {
    puts("═══ 5. Terminal Size (TIOCGWINSZ) ════\n");

    struct winsize ws;
    int fd = STDOUT_FILENO;
    if (!isatty(fd)) fd = open("/dev/tty", O_WRONLY);

    if (fd >= 0 && ioctl(fd, TIOCGWINSZ, &ws) == 0) {
        printf("  rows=%u  cols=%u  xpixel=%u  ypixel=%u\n\n",
               ws.ws_row, ws.ws_col, ws.ws_xpixel, ws.ws_ypixel);
    } else {
        puts("  TIOCGWINSZ not available (not a tty)\n");
    }
    if (fd != STDOUT_FILENO && fd >= 0) close(fd);
    puts("  Handle SIGWINCH to redraw when the window is resized:");
    puts("    signal(SIGWINCH, handle_resize);");
    puts("    // In handler: re-query TIOCGWINSZ, redraw screen\n");
}

/* ── 6. Restore-on-exit pattern ──────────────────────────────────────────── */

static void section6_restore(void) {
    puts("═══ 6. Safe Restore-on-Exit Pattern ════\n");

    puts("  A crashed raw-mode program leaves the terminal unusable.");
    puts("  Guard against this with atexit() and signal handlers:\n");

    puts("  static struct termios g_saved;");
    puts("  static void restore(void) { tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved); }");
    puts("");
    puts("  static void sig_handler(int sig) {");
    puts("      restore();");
    puts("      signal(sig, SIG_DFL);   /* reset to default */");
    puts("      raise(sig);             /* re-deliver for correct exit status */");
    puts("  }");
    puts("");
    puts("  // In main:");
    puts("  tcgetattr(STDIN_FILENO, &g_saved);");
    puts("  atexit(restore);");
    puts("  signal(SIGINT,  sig_handler);");
    puts("  signal(SIGTERM, sig_handler);");
    puts("  signal(SIGHUP,  sig_handler);");
    puts("  // enter raw mode...\n");

    puts("  Additional tips:");
    puts("  - Use write() not printf() in raw mode (fflush can be tricky)");
    puts("  - \\r\\n instead of \\n in raw mode (OPOST/ONLCR is disabled)");
    puts("  - Check isatty(STDIN_FILENO) before entering raw mode");
    puts("  - Libraries: ncurses, linenoise, or Textual (Python) handle this for you\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    section1_concepts();
    section2_inspect();
    section3_raw_mode();
    section4_ansi();
    section5_size();
    section6_restore();
    return 0;
}
