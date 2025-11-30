#include <stdio.h>

#define MAXLEN 256

int main() {
    int i;
    char tmp_str[MAXLEN];
    for (i = 0; i < 95; i++)
        tmp_str[i] = ' ' + i;
    for (i = 0; i < 200; i++)
        printf("%04d %s\n", i, tmp_str);
    return 0;
}
