// hello
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "abuf.h"
#include "file.h"

void die(char *msg); // from gui.c
int read_file(FILE *f, char **out_buffer, size_t *out_length);

char **file_import(char *name, int *lines) {
    FILE *fp;
    if (!(fp = fopen(name, "r")))
        die("fopen");

    char *str = NULL;
    size_t len;
    if (!(read_file(fp, &str, &len))) {
        fprintf(stderr, "[ERR] couldn't read file `%s`\n", strdup(name));
        exit(1);
    }

    int l = 0;
    char **ret = str_split(str, '\n', &l);

    *lines = l;
    return ret;
}

#define CHUNK_SIZE (1024*1024)
// u/daikatana
int read_file(FILE *f, char **out_buffer, size_t *out_length)
{
    *out_buffer = NULL;
    *out_length = 0;

    while (!feof(f))
    {
        // allocate more
        {
            void *new_mem = realloc(*out_buffer, *out_length + CHUNK_SIZE);
            if (new_mem == NULL)
                goto error;

            *out_buffer = new_mem;
        }


        size_t bytes_read = fread(
                &(*out_buffer)[*out_length], 1, CHUNK_SIZE, f);

        if (ferror(f))
            goto error;

        *out_length += bytes_read;


        if (bytes_read < CHUNK_SIZE) {
            void *new_mem = realloc(*out_buffer, *out_length);
            if (new_mem == NULL)
                goto error;

            *out_buffer = new_mem;
        }
    }

    return 1;

error:
    free(*out_buffer);
    return 0;
}

int file_count_lines(char *buf) {
    int lines = 0;
    for (int i = 0; i < (int) strlen(buf); i++) {
        if (buf[i] == '\n')
            lines++;
    }

    return lines;
}

char **str_split(char *src, char c, int *len) {
    abuf_t line = ABUF_INIT;
    const int lines = file_count_lines(src);
    char **buf = (char **) malloc(sizeof(char *) * lines);

    int row = 0;
    for (int i = 0; i < (int) strlen(src); i++) {
        if (src[i] == '\0')
            break;

        if (src[i] != c) {
            abAppend(&line, &src[i], 1);
        } else {
            abAppend(&line, "\0", 1);
            buf[row] = (char *) malloc(line.len);
            memcpy(buf[row], line.b, line.len);
            row++;

            abFree(&line);
            line = (abuf_t) ABUF_INIT;
        }
    }

    *len = row;
    return buf;
}

void file_get_line(char *linebuf, char *buf, int target) {
    int col = 0;
    int row = 0;
    for (int i = 0; i < (int) strlen(buf); i++) {
        if (buf[i] == '\0')
            return;

        if (buf[i] != '\n') {
            if (row == target-1) {
                linebuf[col] = buf[i];
                col++;
            }
        } else {
            row++;
            if (row == target) {
                linebuf[col] = '\0';
                break;
            }
            col = 0;
        }
    }
}
