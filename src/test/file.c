// FILE/PATH UTILITIES
#include "menu.h"
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

bool expand_tilde(char *path, int path_maxlen) {
    char *e;
    char ts[MAXLEN];
    char *tp;

    if (!path || !*path || !path_maxlen)
        return false;
    tp = path;
    if (*tp == '~') {
        tp++;
        while (*tp == '/') {
            tp++;
        }
        e = getenv("HOME");
        if (e) {
            strncpy(ts, e, path_maxlen - 1);
            strncat(ts, "/", path_maxlen - 1);
            strncat(ts, tp, path_maxlen - 1);
            strncpy(path, ts, path_maxlen - 1);
        }
    }
    return true;
}

bool trim_path(char *dir) {
    char *p;

    if (!dir || !*dir)
        return false;
    p = dir;
    while (*p++ != '\0') {
        if (*p == ' ' || *p == '\t' || *p == '\n') {
            *p = '\0';
            break;
        }
    }
    --p;
    while (--p > dir && *p == '/') {
        if (*(p - 1) != '~')
            *p = '\0';
    }
    return true;
}

bool trim_ext(char *buf, char *filename) {

    if (!filename || !*filename || !buf)
        return false;
    char *s = filename;
    char *d = buf;
    *d = '\0';
    while (*s)
        s++;
    while (filename < --s) {
        if (*s == '.') {
            break;
        }
    }
    if (*s != '.') {
        while (*filename)
            *d++ = *filename++;
    } else {
        while (filename < s) {
            *d++ = *filename++;
        }
    }
    *d = '\0';
    if (d == buf)
        return false;
    return true;
}

bool base_name(char *buf, char *path) {
    if (!path || !*path || !buf)
        return false;
    char *s = path;
    char *d = buf;
    *d = '\0';
    while (*s) {
        if (*s == '/' || *s == '\\') {
            d = buf;
        } else {
            *d++ = *s;
        }
        s++;
    }
    *d = '\0';
    if (d == buf)
        return false;
    return true;
}

bool dir_name(char *buf, char *path) {
    if (!path || !*path || !buf)
        return false;
    char tmp_str[MAXLEN];
    strcpy(tmp_str, path);
    char *s = tmp_str;
    while (*s++)
        ;
    while (tmp_str < --s) {
        if (*s == '/' || *s == '\\') {
            *s = '\0';
            break;
        }
    }
    while (tmp_str < --s && (*s == '/' || *s == '\\'))
        *s = '\0';
    char *d = buf;
    *d = '\0';
    s = tmp_str;
    while (*s) {
        *d++ = *s++;
    }
    *d = '\0';
    if (d == buf)
        return false;
    return true;
}

bool verify_exe(char *file_spec) {
    expand_tilde(file_spec, MAXLEN);
    if (access(file_spec, X_OK)) {
        return false;
    }
    return true;
}

bool verify_dir_write(char *dir_spec) {
    int fd = open(dir_spec, O_WRONLY | __O_TMPFILE, S_IWUSR);
    if (fd >= 0) {
        close(fd);
        return true;
    }
    return false;
}

bool verify_dir_read(char *dir_spec) {
    expand_tilde(dir_spec, MAXLEN);
    if (access(dir_spec, R_OK)) {
        return false;
    }
    return true;
}

bool verify_read(char *file_spec) {
    expand_tilde(file_spec, MAXLEN);
    if (access(file_spec, R_OK))
        return false;
    return true;
}

// Terminal I/O control for raw mode and signal handling
// --------------------------------------------------------------

void sig_dfl_mode() {
    struct sigaction sa;

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}

void sig_prog_mode() {
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}

void signal_handler(int sig_num) {
    char c;
    char *msg;
    switch (sig_num) {
    case SIGINT:
        msg = "SIGINT - Interrupt from keyboard";
        break;
    case SIGTERM:
        msg = "SIGTERM - Termination signal";
        break;
    case SIGQUIT:
        msg = "SIGQUIT - Quit from keyboard";
        break;
    default:
        msg = "unknown signal";
        break;
    }
    fprintf(stderr, "\nCaught signal %d - %s\n", sig_num, msg);
    msg = "\nPress 'X' to exit program, any other key to continue:";
    while (*msg)
        write(2, msg++, 1);
    read(0, &c, 1);
    to_uppercase(c);
    if (c == 'X') {
        msg = "\nAre you sure? 'Y' or 'N': ";
        while (*msg)
            write(2, msg++, 1);
        read(0, &c, 1);
        to_uppercase(c);
        if (c == 'Y') {
            msg = "\nExiting program now.\n";
            while (*msg)
                write(2, msg++, 1);
            tcsetattr(0, TCSAFLUSH, &shell_tioctl);
            sig_dfl_mode();
            _exit(1);
        }
    }
    msg = "\nResuming program...\n";
    while (*msg)
        write(2, msg++, 1);
    if (f_curses_open && f_have_curses_tioctl)
        tcsetattr(0, TCSAFLUSH, &curses_tioctl);
    else if (f_have_shell_tioctl)
        tcsetattr(0, TCSAFLUSH, &shell_tioctl);
    sig_prog_mode();
    return;
}
