/** @file enterstr.c
    @brief Enter a string from keyboard
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <cm.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/** @example
    @code
    USERNAME=$(enterstr "Enter your username: ")
    PASSWORD=$(enterstr "Enter your password: ")
    echo "Username: $USERNAME"
    echo "Password: $PASSWORD"
    @endcode
 */

/** @brief allows the user to enter a string in cooked mode
    @note allows line editing and other features
    @note writes the prompt to stderr and the user's input to stdout
    @note handles signals to ensure that the terminal settings are restored if
   the program is interrupted. */
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
