// enterstr.c
// Bill Waller
// billxwaller@gmail.com
// Enter a string from the terminal in cooked mode and return it to the caller.
// This is used by the menu system to get strings from the user.
// This program is designed to be called from shell scripts initiated by the
// menu system. It writes the prompt to stderr, reads the input from stdin,
// and writes the user's answer to stdout so it can be captured by shell
// variables.
//
// for example:
//
//  USERNAME=$(enterstr "Enter your username: ")
//  PASSWORD=$(enterstr "Enter your password: ")
//  echo "Username: $USERNAME"
//  echo "Password: $PASSWORD"
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
#include "menu.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {
    struct termios new_tioctl;
    char in_str[BUFSIZ];
    char *in_ptr = in_str;
    char *msg;

    capture_shell_tioctl();
    sig_prog_mode();
    new_tioctl = shell_tioctl;
    new_tioctl.c_lflag |= ICANON;
    tcsetattr(2, TCSAFLUSH, &new_tioctl);
    while (1) {
        msg = argv[1];
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
