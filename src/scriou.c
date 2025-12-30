//  scriou.c
//  Bill Waller Copyright (c) 2025
//  Screen IO Support for MENU
//  billxwaller@gmail.com
//
//  This file contains terminal ioctl handling functions for MENU. It provide
//  sane terminal settings for MENU applications, and to to restore the terminal
//  to its original state when the MENU application exits.

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

//  ╭───────────────────────────────────────────────────────────────╮
//  │ CAPTURE_SHELL_IOCTL                                           │
//  │ Yes, this is gross overkill implemented in the wee hours of   │
//  │ the morning while trying to resolve an issue with what        │
//  │ appeared to be a randomly occurring event in which the bottom │
//  │ line of my terminal emulator would assume an xoff condition.  │
//  │ I don't think it was ever resolved. Any ideas? Let me know.   │
//  ╰───────────────────────────────────────────────────────────────╯
bool capture_shell_tioctl() {
    if (f_have_shell_tioctl)
        return true;
    tcgetattr(0, &shell_tioctl);
    f_have_shell_tioctl = true;
    return true;
}
bool restore_shell_tioctl() {
    if (!f_have_shell_tioctl)
        return false;
    tcsetattr(0, TCSANOW, &shell_tioctl);
    return true;
}
bool capture_curses_tioctl() {
    if (f_have_curses_tioctl)
        return true;
    tcgetattr(0, &curses_tioctl);
    f_have_curses_tioctl = true;
    return true;
}
bool restore_curses_tioctl() {
    if (!f_have_curses_tioctl)
        return false;
    tcsetattr(0, TCSANOW, &curses_tioctl);
    return true;
}

//  ╭───────────────────────────────────────────────────────────────╮
//  │ SET_SANE_IOCTL                                                │
//  │ I like ISIG and IXANY.                                        │
//  ╰───────────────────────────────────────────────────────────────╯
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

//  ╭───────────────────────────────────────────────────────────────╮
//  │ MK_RAW_IOCTL                                                  │
//  │ unlike cfmakeraw(), this leaves ISIG enabled.                 │
//  ╰───────────────────────────────────────────────────────────────╯
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

//  ╭───────────────────────────────────────────────────────────────╮
//  │ DI_GETCH                                                      │
//  │ accepts a single character                                    │
//  ╰───────────────────────────────────────────────────────────────╯
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
