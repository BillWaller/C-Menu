#include <stddef.h>
#include <stdio.h>

void dump_opts_by_use(char *, char *);

#define MASK "mpfv"

int main(int argc, char **argv) {
    dump_opts_by_use("usage: menu/pick/form/view", "mpfv");
    printf("\n");
}
