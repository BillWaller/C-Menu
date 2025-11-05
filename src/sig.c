// sig.c
// signal handling for interrupt signals
//
// Copyright (c) 2024 Bill Waller
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// ------------------------------------------------------------
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
    char *eargv[MAXARGS];
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
        msg = "\nPress 'X' to exit program, any other key to continue:";
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
        // Curses mode
        eargv[0] = strdup(tmp_str);
        eargv[1] = "Press 'X' to exit program, any other key to continue: ";
        eargv[2] = NULL;
        c = (char)error_message(eargv);
        free(eargv[0]);
        eargv[1] = NULL;
        if (c == 'X') {
            eargv[0] = "\nAre you sure? 'Y' or 'N': ";
            eargv[1] = NULL;
            c = (char)error_message(eargv);
            to_uppercase(c);
            if (c == 'Y') {
                free(eargv[0]);
                eargv[0] = "\nExiting program now.\n";
                eargv[1] = NULL;
                close_curses();
                sig_dfl_mode();
                restore_shell_tioctl();
                _exit(1);
            }
        }
    }
    sig_prog_mode();
    return;
}
