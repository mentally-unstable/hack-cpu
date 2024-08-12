// hello
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LOCAL
#include "args.h"

#define ARGS_INIT (args_t) {NULL}

void args_parse(args_t *args, int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(1);
        }

        // check dir

        else if (!strchr(argv[i], '-')) {
            args->in = argv[i];
            continue;
        }

        else {
            fprintf(stderr, "Command line error: unknown argument `%s`\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    if (!args->in) {
        fprintf(stderr, "Command line error: no input file provided\n");
        usage(argv[0]);
        exit(1);
    }
}

void usage(char *cmd) {
    fprintf(stderr,
"Usage:\n$ %s file [options]\n\
options:\n\
\t-o <file>\toutput file name\n\
\t-h|--help\tdisplays this message\n", cmd);
}

