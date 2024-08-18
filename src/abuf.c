// hello
// credit to Salvatore Sanfilippo <antirez at gmail dot com>
#include <stdlib.h>
#include <string.h>

#include "abuf.h"

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL)
        return;

    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abClean(struct abuf *ab) {
    memset(ab->b, 0, ab->len);
    ab->len = 0;
}

void abFree(struct abuf *ab) {
    free(ab->b);
}
