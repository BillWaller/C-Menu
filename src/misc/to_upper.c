/* to_upper.c
 * Bill Waller
 * billxwaller@gmail.com
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

FILE *in_fp;

void process_stdin();
void process_file(char *file_name);

int main(int argc, char **argv) {
    if (argc == 1)
        process_stdin();
    else
        while (*++argv)
            process_file(*argv);
}

void process_file(char *file_name) {
    register int c;

    if ((in_fp = fopen(file_name, "r")) == NULL) {
        printf("toupper: unable to open %s\n", file_name);
        exit(1);
    }
    c = getc(in_fp);
    while (!feof(in_fp)) {
        putc(toupper(c), stdout);
        c = getc(in_fp);
    }
    fclose(in_fp);
}

void process_stdin() {
    register int c;

    c = getc(stdin);
    while (!feof(stdin)) {
        putc(toupper(c), stdout);
        c = getc(stdin);
    }
}
