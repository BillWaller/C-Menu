#include "menu.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int full_screen_fork_exec(char **);
int full_screen_shell(char *);
int shell(char *);
int fork_exec(char **);
void abend(int ec, char *s);
void user_end();
int nf_error(int ec, char *s);

int full_screen_fork_exec(char **argv) {
    int rc;

    fprintf(stderr, "\n");
    fflush(stderr);
    wclear(stdscr);
    wmove(stdscr, LINES - 1, 0);
    wrefresh(stdscr);
    rc = fork_exec(argv);
    wclear(stdscr);
    wmove(stdscr, 0, 0);
    wrefresh(stdscr);
    restore_wins();
    return (rc);
}

int full_screen_shell(char *shellCmdPtr) {
    int rc;

    fprintf(stderr, "\n");
    fflush(stderr);
    wclear(stdscr);
    wmove(stdscr, 0, 0);
    wrefresh(stdscr);
    rc = shell(shellCmdPtr);
    restore_wins();
    return (rc);
}

int shell(char *shellCmdPtr) {
    int Eargc;
    char *Eargv[MAXARGS];
    char *shellPtr;
    int rc;

    Eargc = 0;
    shellPtr = getenv("SHELL");
    if (shellPtr == NULL || *shellPtr == '\0')
        shellPtr = DEFAULTSHELL;
    Eargv[Eargc++] = strdup(shellPtr);
    Eargv[Eargc++] = "-c";
    Eargv[Eargc++] = shellCmdPtr;
    Eargv[Eargc++] = NULL;
    rc = fork_exec(Eargv);
    return (rc);
}

int fork_exec(char **argv) {
    pid_t pid;
    int status;
    int rc;

    if (argv[0] == 0) {
        display_error_message("fork_exec: missing argument for execvp");
        return (-1);
    }
    capture_curses_tioctl();
    sig_dfl_mode();
    restore_shell_tioctl();
    pid = fork();
    switch (pid) {
    case -1: // parent fork failed
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(stdscr, true);
        snprintf(tmp_str, sizeof(tmp_str), "fork failed: %s, errno: %d",
                 argv[0], errno);
        display_error_message(tmp_str);
        return (-1);
    case 0: // child
        restore_shell_tioctl();
        execvp(argv[0], argv);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(stdscr, true);
        snprintf(tmp_str, sizeof(tmp_str), "execvp failed: %s, errno: %d",
                 argv[0], errno);
        display_error_message(tmp_str);
        return (-1);
    default: // parent
        rc = 0;
        restore_curses_tioctl();
        sig_prog_mode();
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            rc = WEXITSTATUS(status);
            if (rc != 0) {
                keypad(stdscr, true);
                snprintf(tmp_str, sizeof(tmp_str),
                         "Child %s exited  with status %d", argv[0], rc);
                display_error_message(tmp_str);
            }
        } else {
            if (WIFSIGNALED(status)) {
                rc = WTERMSIG(status);
                keypad(stdscr, true);
                snprintf(tmp_str, sizeof(tmp_str),
                         "Child %s terminated by signal %d", argv[0], rc);
                display_error_message(tmp_str);
            } else {
                keypad(stdscr, true);
                snprintf(tmp_str, sizeof(tmp_str),
                         "Child %s terminated abnormally", argv[0]);
                display_error_message(tmp_str);
            }
        }
        break;
    }
    restore_curses_tioctl();
    sig_prog_mode();
    keypad(stdscr, true);
    restore_wins();
    return (rc);
}
