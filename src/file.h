// hello
#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

char **file_import(char *name, int *lines);
int file_count_lines(char *buf);
void file_get_line(char *linebuf, char *buf, int target);
char **str_split(char *src, char c, int *len);

#endif
