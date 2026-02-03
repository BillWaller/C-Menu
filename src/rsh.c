//  rsh.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  <billxwaller@gmail.com>
//
/// rsh - restricted shell to run bash as root
/// Usage: rsh [args]
/// If executed as 'rsh', this program sets the user ID and group ID to 0 (root)
/// and then executes the user's default shell (or /usr/bin/bash if SHELL is not
/// set) with the provided arguments. If no arguments are given, it runs the
/// shell in interactive mode.
///
/// Build instructions:
///
/// To work properly, this program must be compiled and set with the setuid bit:
/// $ sudo -s
/// cc rsh.c -o rsh
/// sudo chown root:root rsh
/// sudo chmod 4755 rsh
/// exit
///
/// Test instructions:
///
/// $ rsh
/// $ whoami
/// root

#include "cm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#ifndef MAXLEN
#define MAXLEN 256
#endif

#ifdef VERBOSE
bool f_verbose = true;
#else
bool f_verbose = false;
#endif

void ABEND(int e, char const *);

int main(int argc, char **argv) {
    char *cargv[30];
    char exec_cmd[MAXLEN] = "/usr/bin/bash";
    char *p;
    int c, a;
    int rc;
    int status;
    pid_t pid;

    if ((p = getenv("SHELL")))
        strnz__cpy(exec_cmd, p, MAXLEN - 1);
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
        ABEND(EXIT_FAILURE, "fork() fatal error");
        break;
    case 0: // Child
        if (argv[0] && strstr(argv[0], "rsh")) {
            if (setuid(0) || setgid(0))
                ABEND(EXIT_FAILURE, "setuid(0) fatal error");
            struct rlimit rl;
            getrlimit(RLIMIT_FSIZE, &rl);
            rl.rlim_cur = RLIM_INFINITY;
            rl.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_FSIZE, &rl);
        }
        execvp(exec_cmd, cargv);
        ABEND(EXIT_FAILURE, "execvp() fatal error");
        break;
    default: // Parent
        waitpid(pid, &status, 0);
        if (f_verbose) {
            if (WIFEXITED(status)) {
                rc = WEXITSTATUS(status);
                if (rc != 0)
                    ABEND(rc, "Child process exited");
            } else {
                if (WIFSIGNALED(status)) {
                    rc = WTERMSIG(status);
                    ABEND(rc, "Child process terminated by signal");
                } else
                    ABEND(EXIT_FAILURE, "Child process terminated abnormally");
            }
        }
        break;
    }
    exit(EXIT_SUCCESS);
}

void ABEND(int e, char const *s) {
    fprintf(stderr, "%s: %d %s\n", s, e, strerror(e));
    exit(EXIT_FAILURE);
}
