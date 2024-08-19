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
void draw_reg(abuf_t *ab, int i);
void draw_numbers(abuf_t *ab, int i);

void cpu_process_line(char *line);

#define ASM_WIDTH 20
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

    int alu_zero;
    int alu_neg;
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

void app_init(char **prog, int lines) {
    enableRawMode();
    if (getWindowSize(&sim.height, &sim.width))
        die("getWindowSize");

    sim.romlines = lines;
    sim.romoff = 0;

    cpu.a = 0;
    cpu.d = 0;

    sim.ramsize = 1000;
    cpu.ram = (int *) calloc(sim.ramsize, sizeof(int));

    cpu.rom = prog; // is this safe lol

    cpu.pc = 0;
}

void app_process_key(int c) {
    switch (c) {
        case 'q':
            free(cpu.ram);

            write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
            printf("quit\n\rdims: %dx%d\r\nlines: %d\r\nromoff: %d\r\n",
                    sim.width, sim.height, sim.romlines, sim.romoff);
            exit(0);
            break;
        case ' ':
            sim.paused = sim.paused ? 0 : 1;
            break;
        case 'r':
            cpu.pc = 0;
            memset(cpu.ram, 0, sim.ramsize);
            cpu.a = 0;
            cpu.d = 0;
            break;
        case 'n':
            cpu.pc++;
            break;
    }
}

void app_update(void) {
    if (sim.paused) return;

    if (cpu.pc >= (sim.height - SCROLLZONE))
        sim.romoff++;

    cpu_process_line(cpu.rom[cpu.pc]);

    cpu.pc++;
    if (cpu.pc == sim.romlines) sim.paused = 1;
}

void app_render(void) {
    abuf_t ab = ABUF_INIT;
    abAppend(&ab, "\x1b[2J", 4);

    for (int i = 0; i < sim.height; i++) {

        draw_rom(&ab, i + sim.romoff);
        if (i < sim.height - 2) {
            draw_ram(&ab, i);
            draw_ram(&ab, i + 255);
        }
        draw_reg(&ab, i);
        abAppend(&ab, "\x1b[K", 3);

        if (i != sim.height - 1)
            abAppend(&ab, "\r\n", 2);
    }

    abAppend(&ab, "\x1b[H", 3);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void draw_rom(abuf_t *ab, int i) {
    int padding = 0;

    int len = 0;
    if ((i - sim.romoff) < sim.height && i < sim.romlines) {
        len = strlen(cpu.rom[i]);
        if (len > ASM_WIDTH) len = ASM_WIDTH;

        draw_numbers(ab, i);

        abAppend(ab, " ", 1);
        abAppend(ab, cpu.rom[i], len);

        padding = ASM_WIDTH - len;
    } else {
        abAppend(ab, "~", 1);
        padding = ROM_WIDTH - ARROW_WIDTH - 1;
    }

    while (padding--) abAppend(ab, " ", 1);

    if (cpu.pc == i)
        abAppend(ab, " <-- ", ARROW_WIDTH);
    else
        abAppend(ab, "     ", ARROW_WIDTH);
}

void draw_ram(abuf_t *ab, int i) {
    draw_numbers(ab, i);

    if (i == cpu.a)
        abAppend(ab, "\x1b[7m", 5);

    char val[9];
    snprintf(&val[0], 9, "%8d", cpu.ram[i]);
    abAppend(ab, &val[0], 9);

    abAppend(ab, "\x1b[27m", 5);
}

void draw_numbers(abuf_t *ab, int i) {
    abAppend(ab, "\x1b[38;2;155;155;155m", 19);
    char num[6];
    snprintf(&num[0], 6, "%5d", i);
    abAppend(ab, &num[0], NUMBER_WIDTH);
    abAppend(ab, "\x1b[0m", 4);
}

void draw_reg(abuf_t *ab, int i) {
    int padding = 3;
    while (padding--) abAppend(ab, " ", 1);
    if (i != sim.height - 1) return;

    abAppend(ab, "A: ", 3);
    char a[8];
    snprintf(&a[0], 8, "%7d", cpu.a);
    abAppend(ab, &a[0], 8);

    abAppend(ab, "  ", 2);

    abAppend(ab, "D: ", 3);
    char d[9];
    snprintf(&d[0], 9, "%8d", cpu.d);
    abAppend(ab, &d[0], 9);
}

// XXX CPU LOGIC

void cpu_process_line(char *line) {
    int n = 0;

    for (int i = 0; i < 16; i++) {
        n <<= 1;
        if (line[i] == '1')
            n |= 1;
    }

    int t = n & 0x8000; // type of instruction

    if (!t) {
        int val = n & 0x7FFF;
        cpu.a = val;
        return;
    }

    int a = n & 0x1000;
    int comp = n & 0x0FC0;

    comp >>= 6;
    int zx = comp & 0b100000;
    int nx = comp & 0b010000;
    int zy = comp & 0b001000;
    int ny = comp & 0b000100;
    int f  = comp & 0b000010;
    int no = comp & 0b000001;

    // x = D, y = A/M
    if (zx) cpu.d = 0;
    if (nx) cpu.d = !cpu.d;
    int *res, *y;

    if (a) {
        if (cpu.a > sim.ramsize) {
            sim.paused = 1;
            // set_error("can't access memory at %d\r\n", cpu.a);
            return;
        }

        y = &cpu.ram[cpu.a];
    } else {
        y = &cpu.a;
    }

    if (zy) *y = 0;
    if (ny) *y = !(*y)

    if (f)
        res = cpu.d + *y;
    else
        res = cpu.d & *y;

    if (no) res = !res;

    int dest = n & 0x0058;
    dest >>= 3;
    int d1 = dest & 0b100;
    int d2 = dest & 0b010;
    int d3 = dest & 0b001;

    if (d1) cpu.a = res;
    if (d2) cpu.d = res;
    if (d3) cpu.ram[cpu.a] = res;

    int jump = n & 0x0007;
    int j1 = jump & 0b100;
    int j2 = jump & 0b010;
    int j3 = jump & 0b001;

    int gt = 0;
    int eq = 0;
    int lt = 0;
    if (j1 && !cpu.alu_zero && !cpu.alu_neg)
        lt = 1;

    if (j2 && cpu.alu_zero && !cpu.alu_neg)
        eq = 1;

    if (j3 && !cpu.alu_zero && cpu.alu_neg)
        gt = 1;

    if (lt || eq || gt) cpu.pc = cpu.a;
}
