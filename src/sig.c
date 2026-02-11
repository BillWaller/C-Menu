/** @file sig.c
    @brief signal handling for interrupt signals
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "cm.h"
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

volatile sig_atomic_t sig_received = 0;

void signal_handler(int);
void sig_prog_mode();
void sig_dfl_mode();
bool f_curses_open = false;
void sig_dfl_mode() {
    struct sigaction sa;

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
}
/** @brief Set up signal handlers for interrupt signals
    @details Upon receiving an interrupt signal (SIGINT, SIGTERM, SIGQUIT), the
   program will prompt the user to either exit the program or continue
   execution. If the user chooses to exit, a confirmation prompt will be
   displayed. If the user opts to continue, the program will resume normal
   operation. */
void sig_prog_mode() {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGINT failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGTERM failed");
        exit(EXIT_FAILURE);
    };
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGQUIT failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGSEGV failed");
        exit(EXIT_FAILURE);
    }
}
/** @brief Signal handler for interrupt signals */
void signal_handler(int sig_num) {
    if (sig_num == SIGINT) {
        sig_received = SIGINT;
    } else if (sig_num == SIGTERM) {
        sig_received = SIGTERM;
    } else if (sig_num == SIGQUIT) {
        sig_received = SIGQUIT;
    } else if (sig_num == SIGSEGV) {
        sig_received = SIGSEGV;
    }
}
/** @brief Handle received signals and prompt user for action */
int handle_signal(int sig_num) {
    int c;
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
    case SIGSEGV:
        msg = "SIGSEGV - Segmentation fault";
        break;
    default:
        msg = "unknown signal";
        break;
    }
    ssnprintf(tmp_str, MAXLEN - 1, "\nCaught signal %d - %s\n", sig_num, msg);
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
            if (c == 'Y')
                exit(EXIT_FAILURE);
        }
    } else {
        strnz__cpy(tmp_str, "Caught signal - Press any key", MAXLEN - 1);
        c = (char)Perror(tmp_str);
        _exit(1);
    }
    sig_prog_mode();
    return c;
}
