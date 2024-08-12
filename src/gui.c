// hello
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "gui.h"

struct termios _orig_termios;
void disableRawMode(void);

void disableRawMode(void) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &_orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(void) {
    if (tcgetattr(STDIN_FILENO, &_orig_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);

    struct termios raw = _orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        die("getWindowSize");
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

    return -1;
}

int readKey(void) {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("readKey");
    }

    return c;
}

// int getIfKey(void) {
//     int nread;
//     int c;
//     if ((nread = read(STDIN_FILENO, &c, 1)) != 1)
//         if (nread == -1 && errno != EAGAIN)
//             die("getIfKey");
//
//     return c;
// }

void die(const char *str) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(str);
    exit(1);
}
