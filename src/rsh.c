/** @file rsh.c
    @brief restricted shell to run bash as root
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <cm.h>
#include <libssh/libssh.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
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

/** @brief Abnormal termination - print an error message and exit
    @param e Error code
    @param s Error message
    @note This function prints the provided error message along with the error
   code and its corresponding string representation, then exits the program with
   a failure status.
 */
void ABEND(int e, char const *);

/** @brief Main function for rsh
    @param argc Argument count
    @param argv Argument vector
    @note If executed as 'rsh', this program sets the user ID and group ID to 0
   (root) and then executes the user's default shell (or /usr/bin/bash if SHELL
   is not set) with the provided arguments. If no arguments are given, it runs
   the shell in interactive mode.
    @note To work properly, this program must be compiled and set with the
   setuid bit:
    @code
    $ sudo -s
    cc rsh.c -o rsh
    sudo chown root:root rsh
    sudo chmod 4755 rsh
    exit
    @endcode
    @note to verify proper operation:
    @code
    $ rsh
    $ whoami
    root
    @endcode
    */
int main(int argc, char **argv) {
    char *cargv[30];
    char exec_cmd[MAXLEN] = "/usr/bin/bash";
    char rsh_user[MAXLEN];
    char *p;
    int c, a;
    int rc;
    int status;
    pid_t pid;
#ifdef RSH_SSH
    ssh_session _ssh_session = ssh_new();
    if (_ssh_session == nullptr)
        exit(EXIT_FAILURE);
    // "$HOME"/.ssh/authorized_keys
    ssh_options_set(_ssh_session, SSH_OPTIONS_HOST, "localhost");
    if ((rc = ssh_connect(_ssh_session)) != SSH_OK) {
        fprintf(stderr, "Error: %s\n", ssh_get_error(_ssh_session));
        ssh_free(_ssh_session);
        exit(EXIT_FAILURE);
    }
    // Authenticate using public key
    if (ssh_userauth_publickey_auto(_ssh_session, NULL, NULL) !=
        SSH_AUTH_SUCCESS) {
        fprintf(stderr, "SSH Public key auth failed: %s\n",
                ssh_get_error(_ssh_session));
        syslog(LOG_ERR, "SSH Error: %s", ssh_get_error(_ssh_session));
        exit(EXIT_FAILURE);
    }
    ssh_disconnect(_ssh_session);
    ssh_free(_ssh_session);
#endif
#ifdef RSH_LOG
    char ttyname[MAXLEN];
    p = getenv("USER");
    strncpy(rsh_user, p ? p : "unknown", sizeof(rsh_user));
    openlog("rsh", LOG_PID | LOG_CONS, LOG_AUTH);
    if (ttyname_r(STDERR_FILENO, ttyname, sizeof(ttyname)) == 0)
        syslog(LOG_INFO, "rsh started by user '%s' on terminal '%s'", rsh_user,
               ttyname);
    closelog();
#endif
    if ((p = getenv("SHELL")))
        strncpy(exec_cmd, p, MAXLEN - 1);
    cargv[0] = strdup(exec_cmd);
    c = 1;
    a = 1;
    if (argc == 1)
        cargv[c++] = "-i";
    while (a < argc)
        cargv[c++] = strdup(argv[a++]);
    cargv[c] = (char *)'\0';
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
    openlog("rsh", LOG_PID | LOG_CONS, LOG_AUTH);
    syslog(LOG_INFO, "rsh exited by user '%s'", rsh_user);
    closelog();
    exit(EXIT_SUCCESS);
}

void ABEND(int e, char const *s) {
    fprintf(stderr, "%s: %d %s\n", s, e, strerror(e));
    exit(EXIT_FAILURE);
}
