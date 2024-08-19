// hello
#ifndef TERMGUI_INCLUDE
#define TERMGUI_INCLUDE

void enableRawMode(void);
int getWindowSize(int *rows, int *cols);

/*
   waits for a keypress
*/
int readKey(void);

/*
   like readKey but without the wait
*/
void getIfKey(int *c);

void die(const char *str);

#endif
