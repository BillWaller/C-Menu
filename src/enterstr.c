/// enterstr.c
/// Enter a string from keyboard
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
///
/// Enter a string from the terminal in cooked mode and return it to the caller.
/// This program is designed to be called from shell scripts initiated by the
/// menu system. It writes the prompt to stderr, reads the input from stdin,
/// and writes the user's answer to stdout so it can be captured by shell
/// variables.
///
/// for example:
///
/// USERNAME=$(enterstr "Enter your username: ")
/// PASSWORD=$(enterstr "Enter your password: ")
/// echo "Username: $USERNAME"
/// echo "Password: $PASSWORD"
///
#include "cm.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {
    /// Returns a string entered by the user in cooked mode. The string is
    /// written to stdout, and the prompt is written to stderr. This allows the
    /// caller to capture the string in a shell variable while still displaying
    /// the prompt to the user. The program uses termios to set the terminal to
    /// cooked mode, allowing for line editing and other features. It also
    /// handles signals to ensure that the terminal settings are restored if the
    /// program is interrupted.
    struct termios new_tioctl;
    char in_str[BUFSIZ];
    char *in_ptr = in_str;
    char *msg;
    char errmsg[128];

    if (argc < 2)
        strcpy(errmsg, "input:");
    else
        strcpy(errmsg, argv[1]);
    capture_shell_tioctl();
    sig_prog_mode();
    new_tioctl = shell_tioctl;
    new_tioctl.c_lflag |= ICANON;
    tcsetattr(2, TCSAFLUSH, &new_tioctl);
    while (1) {
        msg = errmsg;
        while (*msg)
            write(2, msg++, 1);
        if (read(2, in_str, BUFSIZ) > -1)
            break;
    }
    while (*in_ptr)
        write(1, in_ptr++, 1);
    sig_dfl_mode();
    restore_shell_tioctl();
    _exit(0);
}
