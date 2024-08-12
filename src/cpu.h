// hello
#ifndef CPU_INCLUDED
#define CPU_INCLUDED

void cpu_init(char **prog, int lines);
void cpu_process_key(int c);
void cpu_update(void);
void cpu_render(void);

#endif
