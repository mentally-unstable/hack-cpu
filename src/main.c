// hello
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "args.h"

#define GLOBAL_STATE_DEFINE
#include "global.h"

#include "gui.h"
#include "cpu.h"
#include "file.h"

int main(int argc, char **argv) {
    args_t args = { NULL };
    args_parse(&args, argc, argv);

/*
    if (args.whatever)
        global.whatever = yes;
*/

    int lines = 0;
    char **rom = file_import(args.in, &lines);
    app_init(rom, lines);

    int c, nread;
    for (;;)
    {
        if ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
             if (nread == -1 && errno != EAGAIN)
                 die("getIfKey");
        }

        app_process_key(c);
        app_update();
        app_render();
        // usleep(10000000);
    }

    free(&rom[0][0]);
    return 0;
}
