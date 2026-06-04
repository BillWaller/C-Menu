/** @file rsh.c
    @brief restricted shell to run bash as root
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#define _GNU_SOURCE
#include <errno.h>
#ifdef RSH_SSH
#include <libssh/libssh.h>
#endif
#define RSH_PAM
#ifdef RSH_PAM
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#endif
#include <cm.h>
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

#define HOST "localhost"
#ifndef MAXLEN
#define MAXLEN 256
#endif

#ifdef VERBOSE
bool F_VERBOSE = true;
#else
bool F_VERBOSE = false;
#endif

#ifdef RSH_PAM
static struct pam_conv conv = {
    misc_conv,
    NULL};
#endif

/** @brief Abnormal termination - print an error message and exit
    @param e Error code
    @param s Error message
    @details This function prints the provided error message along with the
   error code and its corresponding string representation, then exits the
   program with a failure status.
 */
void ABEND(int e, char const *);

/** @brief Main function for rsh
    @param argc Argument count
    @param argv Argument vector
    @details If executed as 'rsh', this program sets the user ID and group ID to
   0 (root) and then executes the user's default shell (or /usr/bin/bash if
   SHELL is not set) with the provided arguments. If no arguments are given, it
   runs the shell in interactive mode. To work properly, this program must be
   compiled and set with the setuid bit:
    @code
    $ sudo -s
    cc rsh.c -o rsh
    sudo chown root:root rsh
    sudo chmod 4755 rsh
    exit
    @endcode
    to verify proper operation:
    @code
    $ rsh
    $ whoami
    root
    @endcode
    */
int main(int argc, char **argv) {
    char *cargv[30];
    char exec_cmd[MAXLEN] = "/usr/bin/bash";
#ifdef RSH_LOG
    char rsh_user[MAXLEN];
#endif
    char *p;
    int c;
    bool ssh_login = false;
    int status;
    pid_t pid;
#ifdef RSH_SSH
#define RSH_LOG
#endif
#ifdef RSH_LOG
    char ttyname[MAXLEN];
    p = getenv("USER");
    strncpy(rsh_user, p ? p : "unknown", sizeof(rsh_user));
    openlog("rsh", LOG_PID | LOG_CONS, LOG_AUTH);
    if (ttyname_r(STDERR_FILENO, ttyname, sizeof(ttyname)) != 0) {
        syslog(LOG_ERR, "Error getting terminal name: %s", strerror(errno));
        fprintf(stderr, "Error getting terminal name: %s", strerror(errno));
        strncpy(ttyname, "unknown", sizeof(ttyname));
    }
#endif
#define RSH_PAM
#ifdef RSH_PAM
    pam_handle_t *pamh = NULL;
    int retval;
    char *username = getenv("USER");
    if (username == NULL) {
        syslog(LOG_ERR, "USER environment variable not set");
        fprintf(stderr, "Error: USER environment variable not set\n");
        return 1;
    }
    retval = pam_start("rsh-auth", username, &conv, &pamh);
    if (retval != PAM_SUCCESS) {
        syslog(LOG_ERR, "PAM start failed: %s", pam_strerror(pamh, retval));
        fprintf(stderr, "PAM start failed: %s\n", pam_strerror(pamh, retval));
        return 1;
    }

    // Authenticate the user using PAM. The configuration for this PAM service should
    // be set to allow passwordless authentication, for example by using pam_permit.so.
    // You can create a PAM configuration file named /etc/pam.d/pam_nopass with the
    // following content:
    //
    // /etc/pam.d/pam_nopass:
    // auth required pam_permit.so
    //
    //
    retval = pam_authenticate(pamh, 0);
    if (retval == PAM_SUCCESS) {
        syslog(LOG_AUTH, "Authentication succeeded for user '%s': %s", username, pam_strerror(pamh, retval));
    } else {
        syslog(LOG_AUTH, "Authentication failed for user '%s': %s", username, pam_strerror(pamh, retval));
        fprintf(stderr, "Failure: Authentication failed: %s\n", pam_strerror(pamh, retval));
    }

    // It's a wrap
    pam_end(pamh, retval);

    if (retval != PAM_SUCCESS) {
        fprintf(stderr, "PAM authentication failed: %s\n", pam_strerror(pamh, retval));
        exit(EXIT_FAILURE);
    }

#endif

#ifdef RSH_SSH
    ssh_session _ssh_session = ssh_new();
    if (_ssh_session == nullptr) {
        syslog(LOG_ERR, "SSH session initialization failed: %s", ssh_get_error(_ssh_session));
        fprintf(stderr, "SSH session initialization failed: %s", ssh_get_error(_ssh_session));
        ssh_free(_ssh_session);
        exit(EXIT_FAILURE);
    }
    int rc = 0;
    ssh_options_set(_ssh_session, SSH_OPTIONS_HOST, HOST);
    if ((rc = ssh_connect(_ssh_session)) != SSH_OK) {
        syslog(LOG_ERR, "SSH connection failed: %s", ssh_get_error(_ssh_session));
        fprintf(stderr, "SSH connection failed: %s", ssh_get_error(_ssh_session));
        ssh_free(_ssh_session);
        ssh_free(_ssh_session);
        exit(EXIT_FAILURE);
    }
    if (ssh_userauth_publickey_auto(_ssh_session, NULL, NULL) !=
        SSH_AUTH_SUCCESS) {
        syslog(LOG_AUTH, "SSH authentication failed: %s", ssh_get_error(_ssh_session));
        fprintf(stderr, "SSH authentication failed: %s", ssh_get_error(_ssh_session));
        exit(EXIT_FAILURE);
    }
    ssh_disconnect(_ssh_session);
    ssh_free(_ssh_session);
    syslog(LOG_AUTH, "rsh SSH authentication succeeded for user '%s' on terminal '%s'", rsh_user, ttyname);
#endif
#ifdef RSH_LOG
    closelog();
#endif
    if ((p = getenv("SHELL")))
        strncpy(exec_cmd, p, MAXLEN - 1);
    cargv[0] = strdup(exec_cmd);
    c = 1;
    if (argc > 0) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-i") == 0) {
                cargv[c++] = strdup("-i");
            } else if (strcmp(argv[i], "-D1") == 0 && ssh_login) {
                fprintf(stderr, "SSH authentication succeeded\n");
            } else {
                cargv[c++] = strdup(argv[i]);
            }
        }
    }
    cargv[c] = nullptr;
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
        for (int i = 0; i < c; i++)
            free(cargv[i]);
#ifdef F_VERBOSE
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
#endif
        break;
    }
    exit(EXIT_SUCCESS);
}

void ABEND(int e, char const *s) {
    fprintf(stderr, "%s: %d %s\n", s, e, strerror(e));
    exit(EXIT_FAILURE);
}
