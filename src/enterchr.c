/* enterchr.c
 * Bill Waller
 * Accepts a single character, converts it upper case, and sends it to stdout.
 * The calling program should usually supply a new line upon return.
 *
 * Although this program is simple, it is useful for testing menu systems.
 * It can also be used in scripts to get a single character response from a
 * user. For example: response=$(./enterchr "Enter Y or N: ") if [ "$response" =
 * "Y" ]; then echo "You entered Yes" else echo "You entered No" fi
 *
 * stderr is used for the prompt so that stdout can be captured.
 * This program puts the terminal into raw mode to capture a single character
 * without waiting for a newline. It restores the terminal settings before
 * exiting. It also handles SIGINT to ensure terminal settings are restored if
 * the user interrupts the program.
 *
 * stdout returns the single character entered, converted to upper casell. If
 * the user presses the Escape key, the program returns -1.
 *
 * stdin - input stream (usually keyboard)
 *
 */
#include "menu.h"
#include <fcntl.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char c = 'Y';
    char *msg;
    struct termios raw_tioctl;

    capture_shell_tioctl();
    // sig_prog_mode();
    raw_tioctl = shell_tioctl;
    mk_raw_tioctl(&raw_tioctl);
    tcflush(0, TCOFLUSH);
    // tcflush(1, TCOFLUSH);
    // tcflush(2, TCOFLUSH);
    while (1) {
        msg = argv[1];
        while (*msg) {
            write(2, msg++, 1);
        }
        if (read(0, &c, 1) > 0) {
            write(1, &c, 1);
            break;
        }
    }
    // sig_dfl_mode();
    restore_shell_tioctl();
    if (c == (char)0x1b)
        return (1);
    return (0);
}
