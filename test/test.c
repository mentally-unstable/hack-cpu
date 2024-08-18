// hello
#include <stdio.h>
#include <string.h>

void printbits(int x) {
    printf("1      8       16\n");
    for (int i = sizeof(x) << 2; i; i--)
        putchar('0'+((x >> (i-1)) & 1));
    putchar('\n');
}

int convert(char *line) {
    int n = 0;

    for (int i = 0; i < strlen(line); i++) {
        n <<= 1;
        if (line[i] == '1')
            n |= 1;
    }

    return n;
}

int main(void) {
    char *num = "0111";
//    printbits(convert(num));
    printbits(0b1101);
    printbits(0x8);
    printbits(0b1101 & 0x8);
    return 0;
}
