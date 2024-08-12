// hello
#include <stdio.h>
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
    cpu_init(rom, lines);

    int c, nread;
    for (;;)
    {
        if ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
            if (nread == -1 && errno != EAGAIN)
                die("getIfKey");
        }

        cpu_process_key(c);
        cpu_update();
        cpu_render();
    }

    return 0;
}
