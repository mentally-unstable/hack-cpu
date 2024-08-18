// hello
// credit to Salvatore Sanfilippo <antirez at gmail dot com>
#ifndef ABUF_INCLUDED
#define ABUF_INCLUDED

struct abuf {
    char *b;
    int len;
};
typedef struct abuf abuf_t;

#define ABUF_INIT {NULL, 0}

void abAppend(abuf_t *ab, const char *s, int len);
void abClean(struct abuf *ab);
void abFree(abuf_t *ab);

#endif
