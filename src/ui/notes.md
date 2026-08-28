#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

struct termios shell_in_tioctl;
bool f_have_shell_tioctl = false;

bool capture_shell_tioctl() {
    if (f_have_shell_tioctl) return true;

    // 1. Flush all standard C streams to ensure TTY synchronization
    fflush(NULL); 

    // 2. Safely capture the pristine shell state
    if (tcgetattr(STDIN_FILENO, &shell_in_tioctl) == 0) {
        f_have_shell_tioctl = true;
        return true;
    }
    return false;
}

