/*
    rsh.c
    rsh
    Author:  Bill Waller <billxwaller@gmail.com>

    Build instructions:

    cc rsh.c -o rsh
    sudo chown root:root rsh
    sudo chmod 4755 rsh
    exit

    Test instructions:

    $ rsh
    $ whoami
    root

 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

void abend(char const *, int e);

int main(int argc, char **argv) {
    char *cargv[10];
    char exec_cmd[PATH_MAX] = "/usr/bin/bash";
    char *p;
    int c, a;

    if ((p = getenv("SHELL")))
        strcpy(exec_cmd, p);
    cargv[0] = strdup(exec_cmd);
    c = 1;
    a = 1;
    if (argc == 1)
        cargv[c++] = "-i";
    while (a < argc)
        cargv[c++] = strdup(argv[a++]);
    cargv[c] = NULL;
    if (setuid(0) || setgid(0))
        abend("setuid(0) fatal error", errno);
    struct rlimit rl;
    getrlimit(RLIMIT_FSIZE, &rl);
    rl.rlim_cur = RLIM_INFINITY;
    rl.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_FSIZE, &rl);
    execvp(exec_cmd, cargv);
    abend("execvp() fatal error", errno);
}

void abend(char const *s, int e) {
    fprintf(stderr, "%s: %d %s\n", s, e, strerror(e));
    exit(e);
}
