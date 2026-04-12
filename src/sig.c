/** @file sig.c
    @brief signal handling for interrupt signals
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup signal_handling Signal Handling
    @brief Handles signals  such as SIGINT, SIGTERM, SIGQUIT, SIGUSR1, and
   SIGSEGV.
    @details This module provides functions to set up signal handlers for
   various signals, handle received signals, and reset signal handlers to their
   default behavior. It allows the program to respond to interrupt signals by
   prompting the user for action and ensuring proper cleanup before exiting or
   continuing execution.
 */

#include <cm.h>
#include <execinfo.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_FRAMES 64

volatile sig_atomic_t sig_received = 0;

bool f_curses_open = false;

/** @brief Set signal handlers to default behavior
    @ingroup signal_handling
    @details This function sets the signal handlers for SIGINT, SIGTERM,
   SIGQUIT, SIGUSR1, and SIGSEGV to their default behavior. This is typically
   used when the program needs to exit or reset its signal handling to the
   default state.
 */
void sig_dfl_mode() {
    struct sigaction sa;

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);
    sigaction(SIGUSR1, &sa, nullptr);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, nullptr);
}
/** @brief Set up signal handlers for interrupt signals
    @ingroup signal_handling
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
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        abend(-1, "sigaction SIGINT failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        abend(-1, "sigaction SIGTERM failed");
        exit(EXIT_FAILURE);
    };
    if (sigaction(SIGQUIT, &sa, nullptr) == -1) {
        abend(-1, "sigaction SIGQUIT failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGUSR1, &sa, nullptr) == -1) {
        abend(-1, "sigaction SIGUSR1 failed");
        exit(EXIT_FAILURE);
    }
    sa.sa_flags = SA_SIGINFO; // Set the flag to receive siginfo_t
    if (sigaction(SIGSEGV, &sa, nullptr) == -1) {
        abend(-1, "sigaction SIGSEGV failed");
        exit(EXIT_FAILURE);
    }
}
/** @brief Signal handler for interrupt signals
    @ingroup signal_handling
    @param sig_num The signal number received
  */
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
        set_sane_tioctl(&shell_tioctl);
        char *msg1 = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
        write(STDERR_FILENO, msg1, strlen(msg1));
        char *msg2 = "SIGSEGV - Segmentation fault - Stack trace\n\n";
        write(STDERR_FILENO, msg2, strlen(msg2));
        void *addrlist[MAX_FRAMES];
        char **symbols;
        int frames;
        frames = backtrace(addrlist, MAX_FRAMES);
        symbols = backtrace_symbols(addrlist, frames);
        if (symbols == nullptr) {
            abend(-1, "backtrace_symbols failed");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < frames; i++) {
            char buf[256];
            ssnprintf(buf, sizeof(buf), "%s\n", symbols[i]);
            write(STDERR_FILENO, buf, strlen(buf));
        }
        free(symbols);
        msg2 = "\nSIGSEGV Segmentation fault - Writing core to file\n\n";
        write(STDERR_FILENO, msg2, strlen(msg2));
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGSEGV, &sa, NULL);
        kill(getpid(), SIGSEGV); // Re-raise the signal
        exit(EXIT_FAILURE);
    case SIGUSR1:
        sig_received = SIGUSR1;
        break;
    default:
        return;
    }
}
/** @brief Handle received signals and prompt user for action
    @ingroup signal_handling
    @param sig_num The signal number received
  */
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
    case SIGUSR1:
        strnz__cpy(em1, "SIGUSR1 - User Signal 1", MAXLEN - 1);
        break;
    default:
        strnz__cpy(em1, "unknown signal", MAXLEN - 1);
        break;
    }
    if (!f_curses_open)
        restore_shell_tioctl();
    em0[0] = '\0';
    ssnprintf(em0, MAXLEN - 1, "Caught signal %d\n", sig_num);
#ifdef DEBUG_LOG
    write_cmenu_log(em0);
#endif
    strnz__cpy(em2, "Press 'q' or F9 to exit, any other key to continue",
               MAXLEN - 1);
    sig_received = 0;
    return true;
}
