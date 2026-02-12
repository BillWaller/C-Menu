/** @file enterchr.c
    @brief Enter a single character from keyboard
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "cm.h"
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/** @example
    @code
    key=$(enterchr "Are you sure?: ")
    if [ "$key" = "Y" ]; then
        echo "You entered Y"
    else
        echo "You entered N"
    fi
    @endcode
 */

/** @brief Capture the current terminal settings for later restoration.
    @note can be used in scripts to get a single character response from a
    user. For example: response=$(./enterchr "Enter Y or N: ") if [ "$response"
    = "Y" ]; then echo "You entered Yes" else echo "You entered No" fi
    @note This function saves the current terminal settings into a global
   variable so that they can be restored later. It should be called before
   modifying the terminal settings to ensure that the original settings can be
   restored when the program exits or is interrupted.
 */
int main(int argc, char **argv) {
    char c = 'Y';
    char *msg;
    struct termios raw_tioctl;
    char errmsg[128];

    if (argc < 2)
        strcpy(errmsg, "Press any key");
    else
        strcpy(errmsg, argv[1]);
    capture_shell_tioctl();
    raw_tioctl = shell_tioctl;
    mk_raw_tioctl(&raw_tioctl);
    tcflush(0, TCOFLUSH);
    while (1) {
        msg = errmsg;
        while (*msg) {
            write(2, msg++, 1);
        }
        if (read(0, &c, 1) > 0) {
            write(1, &c, 1);
            break;
        }
    }
    restore_shell_tioctl();
    if (c == (char)0x1b)
        return (1);
    return (0);
}
