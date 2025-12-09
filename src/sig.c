// sig.c
// signal handling for interrupt signals
//
// Upon receiving an interrupt signal (SIGINT, SIGTERM, SIGQUIT),
// the program will prompt the user to either exit the program or
// continue execution. If the user chooses to exit, a confirmation
// prompt will be displayed. If the user opts to continue, the program
// will resume normal operation.
//
#include "menu.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

void signal_handler(int);
void sig_prog_mode();
void sig_dfl_mode();
bool f_curses_open = false;

void sig_dfl_mode() {
    struct sigaction sa;

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}

void sig_prog_mode() {
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}

void signal_handler(int sig_num) {
    char c;
    char *msg;
    char tmp_str[MAXLEN];
    switch (sig_num) {
    case SIGINT:
        msg = "SIGINT - Interrupt from keyboard";
        break;
    case SIGTERM:
        msg = "SIGTERM - Termination signal";
        break;
    case SIGQUIT:
        msg = "SIGQUIT - Quit from keyboard";
        break;
    default:
        msg = "unknown signal";
        break;
    }
    snprintf(tmp_str, MAXLEN - 1, "\nCaught signal %d - %s\n", sig_num, msg);
    if (!f_curses_open) {
        msg = tmp_str;
        while (*msg)
            write(2, msg++, 1);
        msg = "\nPress 'X' to exit, any other key to continue:";
        while (*msg)
            write(2, msg++, 1);
        read(0, &c, 1);
        to_uppercase(c);
        if (c == 'X') {
            msg = "\nAre you sure? 'Y' or 'N': ";
            while (*msg)
                write(2, msg++, 1);
            read(0, &c, 1);
            to_uppercase(c);
            if (c == 'Y') {
                msg = "\nExiting program now.\n";
                while (*msg)
                    write(2, msg++, 1);
                tcsetattr(0, TCSAFLUSH, &shell_tioctl);
                sig_dfl_mode();
                _exit(1);
            }
        }
        restore_shell_tioctl();
    } else {
        strcpy(tmp_str, "Caught signal - Press any key");
        c = (char)display_error_message(tmp_str);
        close_curses();
        sig_dfl_mode();
        restore_shell_tioctl();
        _exit(1);
    }
    sig_prog_mode();
    return;
}
