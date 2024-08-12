// hello
#ifndef ARGS_INCLUDE
#define ARGS_INCLUDE

typedef struct {
    char *in;
} args_t;

void args_parse(args_t *args, int argc, char **argv);
void usage(char *cmd);

#endif
