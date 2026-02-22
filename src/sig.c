/** @file sig.c
    @brief signal handling for interrupt signals
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <cm.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

volatile sig_atomic_t sig_received = 0;

bool f_curses_open = false;

/** @brief Set signal handlers to default behavior */
void sig_dfl_mode() {
    struct sigaction sa;

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
}
/** @brief Set up signal handlers for interrupt signals
    @details Upon receiving an interrupt signal (SIGINT, SIGTERM, SIGQUIT), the
   program will prompt the user to either exit the program or continue
   execution. If the user chooses to exit, a confirmation prompt will be
   displayed. If the user opts to continue, the program will resume normal
   operation. */
void sig_prog_mode() {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGINT failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGTERM failed");
        exit(EXIT_FAILURE);
    };
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGQUIT failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        abend(-1, "sigaction SIGSEGV failed");
        exit(EXIT_FAILURE);
    }
}
/** @brief Signal handler for interrupt signals */
void signal_handler(int sig_num) {
    switch (sig_num) {
    case SIGINT:
        sig_received = SIGINT;
        break;
    case SIGTERM:
        sig_received = SIGTERM;
        break;
    case SIGQUIT:
        sig_received = SIGQUIT;
        break;
    case SIGSEGV:
        sig_received = SIGSEGV;
        break;
    default:
        return;
    }
}
/** @brief Handle received signals and prompt user for action */
bool handle_signal(int sig_num) {
    switch (sig_num) {
    case SIGINT:
        strnz__cpy(em1, "SIGINT - Interrupt from keyboard", MAXLEN - 1);
        break;
    case SIGTERM:
        strnz__cpy(em1, "SIGTERM - Termination signal", MAXLEN - 1);
        break;
    case SIGQUIT:
        strnz__cpy(em1, "SIGQUIT - Quit from keyboard", MAXLEN - 1);
        break;
    case SIGSEGV:
        strnz__cpy(em1, "SIGSEGV - Segmentation fault", MAXLEN - 1);
        break;
    default:
        strnz__cpy(em1, "unknown signal", MAXLEN - 1);
        break;
    }
    if (!f_curses_open)
        restore_shell_tioctl();
    em0[0] = '\0';
    ssnprintf(em0, MAXLEN - 1, "Caught signal %d\n", sig_num);
    strnz__cpy(em2, "Press 'q' or F9 to exit, any other key to continue",
               MAXLEN - 1);
    sig_received = 0;
    return true;
}
