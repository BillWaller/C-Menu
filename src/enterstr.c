//  enterstr.c
//  Bill Waller
//  MIT License
//  billxwaller@gmail.com
/// enterstr - enter a string from keyboard
/// Enter a string from the terminal in cooked mode and return it to the caller.
/// This is used by the menu system to get strings from the user.
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
#include "menu.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {
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
