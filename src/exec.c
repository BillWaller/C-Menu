//  exec.c
//  Bill Waller Copyright (c) 2025
//  billxwaller@gmail.com
//
/// Functions to fork and exec commands

#include "menu.h"
#include <errno.h>
#include <fcntl.h>
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
int bg_fork_exec_pipe(char **, int *, pid_t);
void abend(int ec, char *s);
void user_end();
int nf_error(int ec, char *s);
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ FULL_SCREEN_FORK_EXEC                                             │
/// ╰───────────────────────────────────────────────────────────────────╯
/// Fork and exec in full screen mode
/// Clear the screen, move the cursor to the bottom, and refresh
/// the screen before forking and executing the command.
/// After the command completes, clear the screen, move the cursor
/// to the top, refresh the screen, and restore the windows.
/// Return the return code from fork_exec.
/// Arguments:
///   argv - array of arguments for the command to execute
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ FULL_SCREEN_SHELL                                                 │
/// ╰───────────────────────────────────────────────────────────────────╯
/// Execute a shell command in full screen mode
/// Clear the screen, move the cursor to the top, and refresh
/// the screen before executing the shell command.
/// After the command completes, restore the windows.
/// Return the return code from shell.
/// Arguments:
///   shellCmdPtr - pointer to the shell command string
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ SHELL                                                             │
/// ╰───────────────────────────────────────────────────────────────────╯
/// Wrapper for fork_exec that takes a Command String
/// Executes the command string using the user's shell.
/// If the SHELL environment variable is not set, use /bin/sh.
/// Arguments:
///   shellCmdPtr - pointer to the shell command string
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ FORK_EXEC                                                         │
/// ╰───────────────────────────────────────────────────────────────────╯
/// Fork and exec a command
/// Arguments:
///  argv - array of arguments for the command to execute
///  Returns the return code from the executed command
///  On error, returns -1
///  Notes:
///   - captures and restores terminal settings around the fork and exec
///   - sets signal handlers to default in the child process
///   - waits for the child process to complete in the parent process
///   - handles errors from fork and execvp, and reports child exit status
///   - restores curses mode and keypad settings after execution
///   - restores window states after execution
///   - uses a temporary string buffer tmp_str for error messages
///   - uses Perror for error reporting
///   - uses sig_dfl_mode and sig_prog_mode for signal handling
///   - uses capture_curses_tioctl and restore_curses_tioctl for terminal
///   settings
///   - uses restore_shell_tioctl for shell terminal settings
///   - uses waitpid to wait for the child process
///   - uses WIFEXITED, WEXITSTATUS, WIFSIGNALED, and WTERMSIG to interpret
///   child status
///   - uses keypad to manage keypad mode in curses
///   - uses restore_wins to restore window states
///   - uses errno for error codes
///   - uses pid_t for process IDs
///   - uses standard file descriptors STDIN_FILENO, STDOUT_FILENO,
///   STDERR_FILENO
///   - uses execvp for executing the command
///   - uses fork for creating a new process
///   - uses ssnprintf for formatting error messages
///   - uses switch-case for handling fork results
///   - uses default shell if SHELL environment variable is not set
///   - uses MAXARGS for maximum number of arguments
///   - uses DEFAULTSHELL for default shell path
///   - uses P_READ and P_WRITE for pipe ends
///   - uses dup2 for duplicating file descriptors
///   - uses pipe for creating pipes
///   - uses fileno and fopen for file descriptor management
///   - uses strnz__cpy and strnz__cat for string manipulation
///   - uses abend for abnormal termination handling
///   - uses user_end for user termination handling
///   - uses nf_error for not found error handling
///   - uses pick structure for input file descriptor
///   - uses tty_fd for terminal file descriptor
///   - uses tmp_str as a temporary string buffer
///   Returns:
///   - return code from the executed command
///   - -1 on error
///
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ BG_FORK_EXEC_PIPE                                                 │
/// ╰───────────────────────────────────────────────────────────────────╯
/// Fork and exec a command in the background with a pipe
/// Arguments:
///  argv - array of arguments for the command to execute
///  pipe_fd - pointer to an array of two integers for the pipe file descriptors
///  pid - process ID of the forked child process
///  Returns the process ID of the forked child process
int bg_fork_exec_pipe(char **argv, int *pipe_fd, pid_t pid) {

    dup2(STDIN_FILENO, pick->in_fd);
    if (pipe(pipe_fd) == -1) {
        abend(-1, "failed to create pipe");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    switch (pid) {
    case -1:
        abend(-1, "fork failed");
        break;
    case 0: // child
        close(pipe_fd[P_READ]);
        if (dup2(pipe_fd[P_WRITE], STDOUT_FILENO) == -1) {
            abend(-1, "dup2 failed");
            break;
        }
        close(pipe_fd[P_WRITE]);
        execvp(argv[0], argv);
        strnz__cpy(tmp_str, "execvp failed: ", sizeof(tmp_str) - 1);
        strnz__cat(tmp_str, argv[0], sizeof(tmp_str) - strlen(tmp_str) - 1);
        Perror(tmp_str);
        break;
    default: // parent
        close(pipe_fd[P_WRITE]);
        ttyname_r(STDERR_FILENO, tmp_str, sizeof(tmp_str));
        FILE *tty_fp = fopen(tmp_str, "r+");
        tty_fd = fileno(tty_fp);
        dup2(tty_fd, STDIN_FILENO);
        fclose(tty_fp);
        restore_curses_tioctl();
        sig_prog_mode();
        keypad(stdscr, true);
        break;
    }
    return pid;
}
