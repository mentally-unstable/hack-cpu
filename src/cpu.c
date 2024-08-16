// hello
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "abuf.h"
#include "gui.h"
#include "cpu.h"

void draw_rom(abuf_t *ab, int i);
void draw_ram(abuf_t *ab, int i);
void draw_scr(abuf_t *ab, int i); void draw_reg(abuf_t *ab, int i);

#define ASM_WIDTH 25
#define ARROW_WIDTH 5
#define NUMBER_WIDTH 6
#define ROM_WIDTH (ASM_WIDTH + ARROW_WIDTH + NUMBER_WIDTH)

#define MAX_SCR_WIDTH 100

#define MIN_WIDTH (MAX_SCR_WIDTH + ROM_WIDTH)

#define SCROLLZONE (10)

struct cpu_t {
    // registers
    int a;
    int d;
    // mem
    int *ram;
    char **rom;

    int pc;
} cpu;

struct sim_t {
    int paused;
    char **rombuf;
    int romlines;
    int romoff; // offset, scrolling
    char *screenbuf;
    int ramsize;

    int width;
    int height;
} sim;

void cpu_init(char **prog, int lines) {
    enableRawMode();
    if (getWindowSize(&sim.height, &sim.width))
        die("getWindowSize");

    sim.romlines = lines;
    sim.romoff = 0;

    cpu.a = 0;
    cpu.d = 0;

    sim.ramsize = 1000;
    cpu.ram = (int *) calloc(sim.ramsize, sizeof(int));

    cpu.rom = prog;

    cpu.pc = 0;
}

void cpu_process_key(int c) {
    switch (c) {
        case 'q':
            write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
            printf("quit\n\rdims: %dx%d\r\nlines: %d\r\nromoff: %d\r\n",
                    sim.width, sim.height, sim.romlines, sim.romoff);
            exit(0);
            break;
        case ' ':
            sim.paused = sim.paused ? 0 : 1;
            break;
    }
}

void cpu_update(void) {
    if (sim.paused) return;

    if (cpu.pc != sim.romlines && cpu.pc >= (sim.height - SCROLLZONE))
        sim.romoff++;

    cpu.pc++;
    if (cpu.pc >= sim.romlines)
        sim.paused = 1;
}

void cpu_render(void) {
    write(STDOUT_FILENO, "\x1b[2J", 7);
    abuf_t ab = ABUF_INIT;

    for (int i = 0; i < sim.height; i++) {

        draw_rom(&ab, i + sim.romoff);
        draw_ram(&ab, i);
        // draw_scr(&ab, i);
        // draw_reg(&ab, i);
        abAppend(&ab, "\x1b[K", 3);

        if (i != sim.height - 1)
            abAppend(&ab, "\r\n", 2);
    }

    write(STDOUT_FILENO, "\x1b[H", 3);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void draw_rom(abuf_t *ab, int i) {
    int padding = 0;
    if (i < sim.height) {
        char num[6];
        snprintf(&num[0], 6, "%5d", i);
        abAppend(ab, &num[0], NUMBER_WIDTH);

        abAppend(ab, " ", 1);
        abAppend(ab, cpu.rom[i], strlen(cpu.rom[i]));

        padding = ASM_WIDTH - strlen(cpu.rom[i]);
    } else {
        padding = ROM_WIDTH - ARROW_WIDTH;
    }

    while (padding--) abAppend(ab, " ", 1);

    if (cpu.pc == i)
        abAppend(ab, " <-- ", ARROW_WIDTH);
    else
        abAppend(ab, "     ", ARROW_WIDTH);
}

void draw_ram(abuf_t *ab, int i) {
    char val[9];
    snprintf(&val[0], 9, "%8d", cpu.ram[i]);
    abAppend(ab, &val[0], 9);

    char num[6];
    snprintf(&num[0], 6, "%5d", i);
    abAppend(ab, &num[0], NUMBER_WIDTH);
}

void draw_scr(abuf_t *ab, int i) {
    if (i) abAppend(ab, "SCR", 3);
}

void draw_reg(abuf_t *ab, int i) {
    if (i) abAppend(ab, "REG", 3);
}
