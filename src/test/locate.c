#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN 256

bool locate_file_in_path(char *file_spec, char *file_name) {
    char path[MAXLEN];
    char fn[MAXLEN];
    char *p, *fnp, *dir;

    strncpy(fn, file_name, MAXLEN - 1);
    fnp = fn;
    while (*fnp && *fnp != '/')
        fnp++;
    if (*fnp == '/')
        return false;
    if ((p = getenv("PATH")) == NULL)
        return false;
    strcpy(path, p);
    dir = strtok(path, ":");
    while (dir != NULL) {
        strncpy(file_spec, dir, MAXLEN - 1);
        strncat(file_spec, "/", MAXLEN - 1);
        strncat(file_spec, file_name, MAXLEN - 1);
        if (access(file_spec, F_OK) == 0) {
            return true;
        }
        dir = strtok(NULL, ":");
    }
    return false;
}

int main(int argc, char **argv) {
    char file_spec[MAXLEN];

    if (locate_file_in_path(file_spec, argv[1])) {
        printf("%s\n", file_spec);
    } else {
        printf("File %s not found in PATH.\n", argv[1]);
    }
    return 0;
}
