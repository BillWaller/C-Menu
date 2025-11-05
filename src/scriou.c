/*  scriou.c
    Screen IO Support for MENU
    Bill Waller
    billxwaller@gmail.com

    Copyright (c) 2024 Bill Waller
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

   This file contains terminal ioctl handling functions for MENU. It provide
   sane terminal settings for MENU applications, and to to restore the terminal
   to its original state when the MENU application exits.

 */

#include "menu.h"
#include <stdbool.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#ifndef M_TERMCAP
SCREEN *Screen;
#endif

struct termios shell_tioctl;
struct termios curses_tioctl;
bool f_have_shell_tioctl = false;
bool f_have_curses_tioctl = false;
bool capture_shell_tioctl();
bool restore_shell_tioctl();
bool capture_curses_tioctl();
bool restore_curses_tioctl();
bool set_sane_tioctl(struct termios *);
bool mk_raw_tioctl(struct termios *);
void set_cursor_dfl();
void set_cursor_ins();
char di_getch();

// Terminal IOCTL structures
struct termios shell_tioctl, curses_tioctl;
struct termios shell_in_tioctl, curses_in_tioctl;
struct termios shell_out_tioctl, curses_out_tioctl;
struct termios shell_err_tioctl, curses_err_tioctl;

bool capture_shell_tioctl() {
    if (f_have_shell_tioctl)
        return true;
    tcgetattr(0, &shell_tioctl);
    // tcgetattr(0, &shell_in_tioctl);
    // tcgetattr(1, &shell_out_tioctl);
    // tcgetattr(2, &shell_err_tioctl);
    f_have_shell_tioctl = true;
    return true;
}
bool restore_shell_tioctl() {
    if (!f_have_shell_tioctl)
        return false;
    tcsetattr(0, TCSANOW, &shell_tioctl);
    // tcsetattr(0, TCSANOW, &shell_in_tioctl);
    // tcsetattr(1, TCSANOW, &shell_out_tioctl);
    // tcsetattr(2, TCSANOW, &shell_err_tioctl);
    return true;
}
bool capture_curses_tioctl() {
    if (f_have_curses_tioctl)
        return true;
    tcgetattr(0, &curses_tioctl);
    // tcgetattr(0, &curses_in_tioctl);
    // tcgetattr(1, &curses_out_tioctl);
    // tcgetattr(2, &curses_err_tioctl);
    f_have_curses_tioctl = true;
    return true;
}
bool restore_curses_tioctl() {
    if (!f_have_curses_tioctl)
        return false;
    tcsetattr(0, TCSANOW, &curses_tioctl);
    // tcsetattr(0, TCSANOW, &curses_in_tioctl);
    // tcsetattr(1, TCSANOW, &curses_out_tioctl);
    // tcsetattr(2, TCSANOW, &curses_err_tioctl);
    return true;
}

// NOMINAL SANE VALUES
// -----------------------------------------------------------------------------
bool set_sane_tioctl(struct termios *t_p) {
    tcgetattr(0, t_p);
    t_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | INPCK | ISTRIP | INLCR |
                      IGNCR | ICRNL | IXON | IXOFF);
    t_p->c_iflag |= IUTF8;
    t_p->c_oflag |= OPOST | OCRNL;
    t_p->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
    t_p->c_lflag |= (ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK);
    t_p->c_cflag &= ~(CSIZE | PARENB);
    t_p->c_cflag |= CS8 | CLOCAL | CREAD;
    tcsetattr(0, TCSANOW, t_p);
    return true;
}

// RAW MODE
// Based on code from "man termios"
// unlike cfmakeraw(), this leaves ISIG enabled
bool mk_raw_tioctl(struct termios *t_p) {
    tcgetattr(0, t_p);
    // t_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | INPCK | ISTRIP | INLCR |
    //                IGNCR | ICRNL | IXON | IXOFF);
    // t_p->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
    // t_p->c_cflag &= ~(CSIZE | PARENB);
    // t_p->c_cflag |= CS8 | CLOCAL;
    //
    t_p->c_lflag |= ISIG;
    t_p->c_lflag &= ~(ECHO | ICANON);
    t_p->c_cc[VMIN] = 1;
    t_p->c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, t_p);
    return true;
}

char di_getch() {
    struct termios org_tioctl, new_tioctl;
    char buf;

    if (tcgetattr(0, &org_tioctl) == -1) {
        fprintf(stderr, "standard error not a tty\n");
        return (0);
    }
    new_tioctl = org_tioctl;
    new_tioctl.c_lflag &= ~(ECHO | ICANON);
    new_tioctl.c_cc[VMIN] = 1;
    new_tioctl.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &new_tioctl);
    read(0, &buf, 1);
    tcsetattr(0, TCSAFLUSH, &org_tioctl);
    return (buf);
}
