#include <stddef.h>
#include <stdio.h>

void dump_opts_by_use(char *, char *);

int main(int argc, char **argv) {
    dump_opts_by_use("usage: form_exec & form_write", "mpxwv");
    printf("\n");
}
