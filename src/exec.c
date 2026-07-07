/** @file exec.c
    @brief Functions to execute external commands
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup exec External Commands
    @brief This module provides functions to execute external commands
    @details Handles terminal settings, signal handling, and error reporting to
   ensure a smooth user experience when executing commands from within the
   application. The main functions include full_screen_fork_exec,
   full_screen_shell, and fork_exec, which manage the execution of commands
   while maintaining the integrity of the application's user interface.
 */

#include <cm.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int full_screen_fork_exec(char **);
int full_screen_shell(char *);
int shell(char *);
int fork_exec(char **);
int nf_error(int ec, char *s);
int fork_detach_execvp(char **);
/** @brief Execute a command in full screen mode
    @ingroup exec
    @param argv - array of arguments for the command to execute
    @return the return code from the executed command
    @details Clear the screen,
    move the cursor to the bottom, and update the screen before executing
   the command.
    After the command completes, clear the screen, move the cursor to the
   top, update the screen, and restore the windows. */
int full_screen_fork_exec(char **argv) {
    int rc;

    fflush(stderr);
    wmove(stdscr, LINES - 1, 0);
    rc = fork_exec(argv);
    return (rc);
}
/** @brief Execute a shell command in full screen mode
    @ingroup exec
    @param shellCmdPtr - pointer to the shell command string
    @return the return code from the executed shell command
    @details Clear the screen, move the cursor to the top, and update the
   screen before executing the shell command. After the command completes,
   restore the windows.
 */
int full_screen_shell(char *shellCmdPtr) {
    int rc;

    fflush(stderr);
    werase(stdscr);
    wmove(stdscr, 0, 0);
    rc = shell(shellCmdPtr);
    restore_wins();
    return (rc);
}
/** @brief Execute a shell command
    @ingroup exec
    @param shellCmdPtr - pointer to the shell command string
    @return the return code from the executed shell command
    @details Executes the command string using the user's shell.
    If the SHELL environment variable is not set, use /bin/sh. */
int shell(char *shellCmdPtr) {
    int Eargc;
    char *Eargv[MAXARGS];
    char *shellPtr;
    int rc;

    Eargc = 0;
    shellPtr = getenv("SHELL");
    if (shellPtr == nullptr || *shellPtr == '\0')
        shellPtr = DEFAULTSHELL;
    Eargv[Eargc++] = strdup(shellPtr);
    Eargv[Eargc++] = "-c";
    Eargv[Eargc++] = shellCmdPtr;
    Eargv[Eargc++] = nullptr;
    rc = fork_exec(Eargv);
    free(Eargv[0]);
    return (rc);
}
/** @brief Fork and exec a command
    @ingroup exec
    @param argv - array of arguments for the command to execute
    @return the return code from the executed command, or -1 on error
    @details Captures and restores terminal settings around the fork and exec.
    Sets signal handlers to default in the child process.
    Waits for the child process to complete in the parent process.
    Handles errors from fork and execvp, and reports child exit status.
    Restores curses mode and keypad settings after execution.
    Restores window states after execution.
    Uses a temporary string buffer tmp_str for error messages.
    Uses Perror for error reporting.
    Uses sig_dfl_mode and sig_prog_mode for signal handling.
    Uses capture_curses_tioctl and restore_curses_tioctl for terminal
   settings.
    Uses restore_shell_tioctl for shell terminal settings.
    Uses waitpid to wait for the child process.
    Uses WIFEXITED, WEXITSTATUS, WIFSIGNALED, and WTERMSIG to interpret
   child status.
    Uses keypad to manage keypad mode in curses.
    Uses restore_wins to restore window states.
    Uses errno for error codes.
    Uses pid_t for process IDs.
    Uses standard file descriptors STDIN_FILENO, STDOUT_FILENO,
   STDERR_FILENO.
    Uses execvp for executing the command.
    Uses fork for creating a new process.
    Uses ssnprintf for formatting error messages.
    Uses switch-case for handling fork results.
    Uses default shell if SHELL environment variable is not set. */
int fork_exec(char **argv) {
    char tmp_str[MAXLEN];
    pid_t pid;
    int status;
    int rc;

    if (argv[0] == 0) {
        Perror("fork_exec: missing argument for execvp");
        return (-1);
    }
    capture_curses_tioctl();
    curs_set(1);
    sig_dfl_mode();
    stdio_names(stdio_names_str, "exec.c:139");
    endwin();
    stdio_names(stdio_names_str, "exec.c:141");
    tmp_str[0] = '\0';
    pid = fork();
    if (pid < 0) {
        sig_prog_mode();
        keypad(stdscr, true);
        ssnprintf(tmp_str, sizeof(tmp_str), "fork failed: %s, errno: %d",
                  argv[0], errno);
        Perror(tmp_str);
        return (-1);
    } else if (pid == 0) {
        restore_shell_tioctl();
        sig_dfl_mode();
        execvp(argv[0], argv);
        fprintf(stderr, "execvp failed: %s, errno: %d\n", argv[0], errno);
        exit(EXIT_FAILURE);
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        rc = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        ssnprintf(tmp_str, sizeof(tmp_str), "Child process terminated by signal: %d",
                  WTERMSIG(status));
        Perror(tmp_str);
        rc = -1;
    } else {
        ssnprintf(tmp_str, sizeof(tmp_str), "Child process terminated abnormally");
        Perror(tmp_str);
        rc = -1;
    }
    reset_prog_mode();
    restore_wins();
    return (rc);
}
/** @brief Fork, detach, and exec a command
    @ingroup exec
    @param eargv - array of arguments for the command to execute
    @return 0 on success, or exits on failure
    @details Forks a new process, detaches it from the terminal, and executes
   the specified command using execvp.
    Closes standard input, output, and error file descriptors in the child
   process.
    Redirects standard input, output, and error to /dev/null in the child
   process.
    Closes all other file descriptors in the child process.
    Sets the session ID for the child process to detach it from the terminal.
    Restores curses mode and signal handling in the parent process after
   forking.
    Restores window states in the parent process after forking. */
int fork_detach_execvp(char **eargv) {
    pid_t pid = fork();
    capture_curses_tioctl();
    curs_set(1);
    sig_dfl_mode();

    if (pid < 0) {
        fprintf(stderr, "First fork failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        if (setsid() < 0) {
            fprintf(stderr, "Set session ID failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        int dev_null = open("/dev/null", O_RDWR);
        if (dev_null != -1) {
            dup2(dev_null, STDIN_FILENO);
            dup2(dev_null, STDOUT_FILENO);
            dup2(dev_null, STDERR_FILENO);
            if (dev_null > 2) {
                close(dev_null);
            }
        }
        long max_fd = sysconf(_SC_OPEN_MAX);
        for (long fd = 3; fd < max_fd; fd++)
            close(fd);
        execvp(eargv[0], eargv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    restore_curses_tioctl();
    sig_prog_mode();
    restore_wins();
    return 0;
}
