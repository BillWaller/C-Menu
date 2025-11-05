#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXLEN 256
#define WC_OK (W_OK | 0x1000)

char tmp_str[MAXLEN];
bool verify_file_writable(char *, int);
void expand_tilde(char *, size_t);
void display_error_message(const char *);
bool dir_name(char *, char *);

int main(int argc, char **argv) {
    char spec[MAXLEN];

    strncpy(spec, argv[1], MAXLEN - 1);
    if (verify_file_writable((char *)spec, W_OK)) {
        printf("Success\n");
    } else {
        printf("Fail\n");
    }

    return 0;
}

void expand_tilde(char *path, size_t size) {
    if (path[0] == '~') {
        const char *home = getenv("HOME");
        if (home) {
            char expanded[MAXLEN];
            snprintf(expanded, sizeof(expanded), "%s%s", home, path + 1);
            strncpy(path, expanded, size - 1);
            path[size - 1] = '\0';
        }
    }
}
void display_error_message(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}

bool verify_file(char *spec, int mode) {
    char dirbuf[MAXLEN];
    expand_tilde(spec, MAXLEN);
    if (faccessat(AT_FDCWD, spec, mode, AT_EACCESS) == 0)
        return true;
    else {
        if (errno == EACCES) {
            if (mode == W_OK) {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not writable (permission denied).",
                        MAXLEN - 1);
            }
            if (mode == R_OK) {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not readable (permission denied).",
                        MAXLEN - 1);
            }
            if (mode == X_OK) {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not executable (permission denied).",
                        MAXLEN - 1);
            }
            display_error_message(tmp_str);
            return false;
        } else if (errno == ENOENT) {
            if (mode == WC_OK) {
                dir_name(dirbuf, spec);
                if (faccessat(AT_FDCWD, dirbuf, W_OK, AT_EACCESS) == 0)
                    return true;
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, "does not exist and cannot be created",
                        MAXLEN - 1);
                display_error_message(tmp_str);
                return false;
            } else {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " does not exist.", MAXLEN - 1);
                display_error_message(tmp_str);
                return false;
            }
        }
        strncpy(tmp_str, "Error accessing file ", MAXLEN - 1);
        strncat(tmp_str, spec, MAXLEN - 1);
        strncat(tmp_str, ": ", MAXLEN - 1);
        strncat(tmp_str, strerror(errno), MAXLEN - 1);
        display_error_message(tmp_str);
        return false;
    }
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
