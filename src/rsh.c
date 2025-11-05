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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef MAXLEN
#define MAXLEN 256
#endif

#ifdef VERBOSE
bool f_verbose = true;
#else
bool f_verbose = false;
#endif

void abend(int e, char const *);

int main(int argc, char **argv) {
    char *cargv[30];
    char exec_cmd[MAXLEN] = "/usr/bin/bash";
    char *p;
    int c, a;
    int rc;
    int status;
    pid_t pid;

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
    pid = fork();
    switch (pid) {
    case -1:
        abend(EXIT_FAILURE, "fork() fatal error");
        break;
    case 0:
        if (setuid(0) || setgid(0))
            abend(EXIT_FAILURE, "setuid(0) fatal error");
        struct rlimit rl;
        getrlimit(RLIMIT_FSIZE, &rl);
        rl.rlim_cur = RLIM_INFINITY;
        rl.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_FSIZE, &rl);
        execvp(exec_cmd, cargv);
        abend(EXIT_FAILURE, "execvp() fatal error");
        break;
    default:
        waitpid(pid, &status, 0);
        if (f_verbose) {
            if (WIFEXITED(status)) {
                rc = WEXITSTATUS(status);
                if (rc != 0)
                    abend(rc, "Child process exited");
            } else {
                if (WIFSIGNALED(status)) {
                    rc = WTERMSIG(status);
                    abend(rc, "Child process terminated by signal");
                } else
                    abend(EXIT_FAILURE, "Child process terminated abnormally");
            }
        }
        break;
    }
    exit(EXIT_SUCCESS);
}

void abend(int e, char const *s) {
    fprintf(stderr, "%s: %d %s\n", s, e, strerror(e));
    exit(EXIT_FAILURE);
}
