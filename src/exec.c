/** @file exec.h
    @brief Functions to execute external commands
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "cm.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int eargc;
char *eargv[MAXARGS];
char earg_str[MAXLEN];

int full_screen_fork_exec(char **);
int full_screen_shell(char *);
int shell(char *);
int fork_exec(char **);
void abend(int ec, char *s);
void user_end();
int nf_error(int ec, char *s);
/** @brief Execute a command in full screen mode
 *  @param argv - array of arguments for the command to execute
 *  @return the return code from the executed command
 *  @note Clear the screen,
 *  @note move the cursor to the bottom, and refresh the screen before executing
 * the command.
 *  @note After the command completes, clear the screen, move the cursor to the
 * top, refresh the screen, and restore the windows.
 */
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
/** @brief Execute a shell command in full screen mode
 *  @param shellCmdPtr - pointer to the shell command string
 *  @return the return code from the executed shell command
 *  @note Clear the screen, move the cursor to the top, and refresh the screen
 * before executing the shell command.
 *  @note After the command completes, restore the windows.
 */
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
/** @brief Execute a shell command
 *  @param shellCmdPtr - pointer to the shell command string
 *  @return the return code from the executed shell command
 *  @note Executes the command string using the user's shell.
 *  @note If the SHELL environment variable is not set, use /bin/sh.
 */
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
    free(Eargv[0]);
    return (rc);
}
/** @brief Fork and exec a command
 *  @param argv - array of arguments for the command to execute
 *  @return the return code from the executed command, or -1 on error
 *  @note Captures and restores terminal settings around the fork and exec.
 *  @note Sets signal handlers to default in the child process.
 *  @note Waits for the child process to complete in the parent process.
 *  @note Handles errors from fork and execvp, and reports child exit status.
 *  @note Restores curses mode and keypad settings after execution.
 *  @note Restores window states after execution.
 *  @note Uses a temporary string buffer tmp_str for error messages.
 *  @note Uses Perror for error reporting.
 *  @note Uses sig_dfl_mode and sig_prog_mode for signal handling.
 *  @note Uses capture_curses_tioctl and restore_curses_tioctl for terminal
 * settings.
 *  @note Uses restore_shell_tioctl for shell terminal settings.
 *  @note Uses waitpid to wait for the child process.
 *  @note Uses WIFEXITED, WEXITSTATUS, WIFSIGNALED, and WTERMSIG to interpret
 * child status.
 *  @note Uses keypad to manage keypad mode in curses.
 *  @note Uses restore_wins to restore window states.
 *  @note Uses errno for error codes.
 *  @note Uses pid_t for process IDs.
 *  @note Uses standard file descriptors STDIN_FILENO, STDOUT_FILENO,
 * STDERR_FILENO.
 *  @note Uses execvp for executing the command.
 *  @note Uses fork for creating a new process.
 *  @note Uses ssnprintf for formatting error messages.
 *  @note Uses switch-case for handling fork results.
 *  @note Uses default shell if SHELL environment variable is not set.
 */
int fork_exec(char **argv) {
    pid_t pid;
    int status;
    int rc;

    if (argv[0] == 0) {
        Perror("fork_exec: missing argument for execvp");
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
        ssnprintf(tmp_str, sizeof(tmp_str), "fork failed: %s, errno: %d",
                  argv[0], errno);
        Perror(tmp_str);
        return (-1);
    case 0: // child
        restore_shell_tioctl();
        execvp(argv[0], argv);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(stdscr, true);
        ssnprintf(tmp_str, sizeof(tmp_str), "execvp failed: %s, errno: %d",
                  argv[0], errno);
        Perror(tmp_str);
        exit(-1);
    default: // parent
        rc = 0;
        restore_curses_tioctl();
        sig_prog_mode();
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            rc = WEXITSTATUS(status);
            if (rc != 0) {
                keypad(stdscr, true);
                ssnprintf(tmp_str, sizeof(tmp_str),
                          "Child %s exited  with status %d", argv[0], rc);
                Perror(tmp_str);
            }
        } else {
            if (WIFSIGNALED(status)) {
                rc = WTERMSIG(status);
                keypad(stdscr, true);
                ssnprintf(tmp_str, sizeof(tmp_str),
                          "Child %s terminated by signal %d", argv[0], rc);
                Perror(tmp_str);
            } else {
                keypad(stdscr, true);
                ssnprintf(tmp_str, sizeof(tmp_str),
                          "Child %s terminated abnormally", argv[0]);
                Perror(tmp_str);
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
