// hello
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printbits(int x) {
    printf("1      8       16\n");
    for (int i = sizeof(x) << 2; i; i--)
        putchar('0'+((x >> (i-1)) & 1));
    putchar('\n');
}

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

void app_init(char **prog, int lines) {
    sim.romlines = lines;
    sim.romoff = 0;
    sim.paused = 1;

    cpu.a = 0;
    cpu.d = 0;

    sim.ramsize = 1000;
    cpu.ram = (int *) calloc(sim.ramsize, sizeof(int));

    cpu.rom = prog; // is this safe lol
    cpu.pc = 0;
}

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
    printbits(comp);

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
    int res, *y;

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
    if (ny) *y = !(*y);

    if (f)
        res = cpu.d + *y;
    else
        res = cpu.d & *y;

    if (no) res = !res;

    int dest = n & 0x0038;
    dest >>= 3;
    printf("dest:\n");
    printbits(dest);
    int d1 = dest & 0b100;
    printbits(d1);
    int d2 = dest & 0b010;
    printbits(d2);
    int d3 = dest & 0b001;
    printbits(d3);

    if (d1) {cpu.a = res; printf("set a\n");}
    if (d2) {cpu.d = res; printf("set d\n");}
    if (cpu.a >= sim.ramsize) {
        printf("error, cpu.a (%d) out of ram range\n", cpu.a);
        exit(0);
    }
    if (d3) {cpu.ram[cpu.a] = res; printf("set mem\n");}

    puts("hello 2");
    int jump = n & 0x0007;
    printf("jump:\n");
    printbits(jump);
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

int main(void) {
    app_init(NULL, 0);
    char *num = "1110111111001000";
    printf("%s\n", num);
    cpu_process_line(num);
    return 0;
}
