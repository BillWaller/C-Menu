/// scriou.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  Screen IO Support for MENU
//  billxwaller@gmail.com
//
///  This file contains terminal ioctl handling functions for C-MENU. It
///  provide sane terminal settings for C-MENU applications, and restores
///  the terminal to its original state when the C-MENU application exits.

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

/// Terminal IOCTL structures
struct termios shell_tioctl, curses_tioctl;
struct termios shell_in_tioctl, curses_in_tioctl;
struct termios shell_out_tioctl, curses_out_tioctl;
struct termios shell_err_tioctl, curses_err_tioctl;

///  capture_shell_tioctl() - capture shell terminal settings
///  restore_shell_tioctl() - restore shell terminal settings
///  capture_curses_tioctl() - capture curses terminal settings
///  restore_curses_tioctl() - restore curses terminal settings
///  Uses global flag variables
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

///  set_sane_tioctl(struct termios *t_p) - set terminal to sane settings
///  @param t_p - pointer to termios structure to modify
///  @return - true on success
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
///  mk_raw_tioctl(struct termios *t_p) - set terminal to raw mode
///  @param t_p - pointer to termios structure to modify
///  @return - true on success
///  @note - unlike cfmakeraw(), this leaves ISIG enabled.
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
///  di_getch() - get single character from terminal in raw mode
///  @return - the character read from terminal
char di_getch() {
    struct termios org_tioctl, new_tioctl;
    char buf;

    if (tcgetattr(2, &org_tioctl) == -1) {
        fprintf(stderr, "\ndi_getch: tcgetattr failed\n");
        return (0);
    }
    new_tioctl = org_tioctl;
    new_tioctl.c_lflag &= ~(ECHO | ICANON);
    new_tioctl.c_cc[VMIN] = 1;
    new_tioctl.c_cc[VTIME] = 0;
    tcsetattr(2, TCSAFLUSH, &new_tioctl);
    read(2, &buf, 1);
    tcsetattr(2, TCSAFLUSH, &org_tioctl);
    return (buf);
}
