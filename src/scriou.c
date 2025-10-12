/* scriou.c
 * Screen IO Support for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>

#ifndef M_TERMCAP
SCREEN *Screen;
#endif

struct termios shell_ioctl;
int f_shell_ioctl;
int f_curses_open;
int f_restore_screen;

void open_curses();
void close_curses();
void end_pgm();
void sig_prog_mode();
void sig_shell_mode();
int fork_exec(char **);
void display_argv_error_msg(char *, char **);
void SetCursorDefault();
void SetCursorInsert();
char di_getch();
void capture_shell_ioctl();
void restore_shell_ioctl();

void open_curses() {
    capture_shell_ioctl();
    def_shell_mode();
    initscr();
    f_curses_open = TRUE;
    clear();
    nonl(); // don't translate CR to LF
    noecho();
    cbreak(); // raw unbuffered
    keypad(stdscr, TRUE);
    clearok(stdscr, FALSE);
    scrollok(stdscr, TRUE);
    win_init_attrs(option->fg_color, option->bg_color, option->bo_color);
}

void close_curses() {
    if (f_curses_open) {
        wclear(stdscr);
        wrefresh(stdscr);
        endwin();
        f_curses_open = FALSE;
    }
    reset_shell_mode();
    sig_shell_mode();
}

void end_pgm() {
    close_curses();
    exit(0);
}

void sig_prog_mode() {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

void sig_shell_mode() {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

int fork_exec(char **FArgv) {
    int Wstat;
    int pid;

    if (FArgv[0] == 0) {
        display_error_message("Missing argument for execvp");
        return (-1);
    }
    sig_shell_mode();
    reset_shell_mode();
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    switch (pid) {
    case -1:
        reset_prog_mode();
        sig_prog_mode();
        keypad(stdscr, TRUE);
        display_argv_error_msg("fork_exec() Failed to fork", FArgv);
        return (-1);
    case 0:
        restore_shell_ioctl();
        execvp(FArgv[0], FArgv);
        reset_prog_mode();
        sig_prog_mode();
        keypad(stdscr, TRUE);
        display_argv_error_msg("fork_exec() Failed to execvp", FArgv);
        return (-1);
    default:
        pid = waitpid(0, &Wstat, 0);
        break;
    }
    reset_prog_mode();
    sig_prog_mode();
    if ((pid >>= 8) && f_stop_on_error) {
        sprintf(tmp_str, "Program returned %x", Wstat);
        display_argv_error_msg(tmp_str, FArgv);
        return (-1);
    }
    fprintf(stderr, "\n");
    return (0);
}

void display_argv_error_msg(char *emsg, char **argv) {
    int argc;

    argc = 0;
    fprintf(stderr, "\r\n");
    while (*argv != NULL && **argv != '\0')
        fprintf(stderr, "argv[%d] - %s\r\n", argc++, *argv++);
    fprintf(stderr, "%s\r\n", emsg);
    fprintf(stderr, "%s", "Press any key to continue");
    wrefresh(stdscr);
    wgetch(stdscr);
}

void capture_shell_ioctl() {
    if (tcgetattr(2, &shell_ioctl) == -1) {
        fprintf(stderr, "standard error not a tty\n");
        return;
    }
}

void restore_shell_ioctl() {
    if (tcsetattr(2, TCSAFLUSH, &shell_ioctl) == -1) {
        fprintf(stderr, "standard error not a tty\n");
        return;
    }
}

char di_getch() {
    struct termios OrgIOCTL, NewIOCTL;
    char buf;

    if (tcgetattr(2, &OrgIOCTL) == -1) {
        fprintf(stderr, "standard error not a tty\n");
        return (0);
    }
    NewIOCTL = OrgIOCTL;
    NewIOCTL.c_lflag &= ~(ECHO | ICANON);
    NewIOCTL.c_cc[VMIN] = 1;
    NewIOCTL.c_cc[VTIME] = 0;
    tcsetattr(2, TCSAFLUSH, &NewIOCTL);
    read(2, &buf, 1);
    tcsetattr(2, TCSAFLUSH, &OrgIOCTL);
    return (buf);
}
