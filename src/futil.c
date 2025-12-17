/* futil.c
 * Utility functions for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>

int trim(char *);
int ssnprintf(char *, size_t, const char *, ...);
bool str_to_bool(const char *);
int str_to_args(char **, char *);
double str_to_double(char *);
void str_to_lower(char *);
void str_to_upper(char *);
void strnz_cpy(char *, char *, int);
void strnz_cat(char *, char *, int);
void strz(char *);
int strnz(char *, int);
char *strz_dup(char *);
char *strzdup(char *);
void str_subc(char *, char *, char, char *, int);
void normalize_file_spec(char *);
void file_spec_path(char *, char *);
void file_spec_name(char *, char *);
bool verify_file(char *, int);
bool verify_dir(char *, int);
bool locate_file_in_path(char *, char *);

char errmsg[MAXLEN];

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ TRIM                                                              │
    ╰───────────────────────────────────────────────────────────────────╯ */
int trim(char *s) {
    char *p = s;
    char *d = s;
    while (*p == ' ')
        p++;
    while (*p != '\0')
        *d++ = *p++;
    while (*(d - 1) == ' ' && d > s)
        d--;
    *d = '\0';
    return d - s;
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ SSNPRINTF                                                         │
    ╰───────────────────────────────────────────────────────────────────╯ */
int ssnprintf(char *buf, size_t buf_size, const char *format, ...) {
    int n;
    va_list args;

    va_start(args, format);
    n = vsnprintf(buf, buf_size, format, args);
    va_end(args);

    return n;
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STR_TO_ARGS                                                       │
    ╰───────────────────────────────────────────────────────────────────╯ */
int str_to_args(char **argv, char *strptr) {
    int i;

    for (i = 0; i < MAXARGS; i++) {
        if ((argv[i] = strtok(strptr, " \t")) == (char *)0)
            break;
        strptr = (char *)0;
    }
    argv[i] = NULL;
    return (i);
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STR_TO_LOWER                                                      │
    ╰───────────────────────────────────────────────────────────────────╯ */
void str_to_lower(char *s) {
    while (*s != '\0') {
        if (*s >= 'A' && *s <= 'Z')
            *s = *s + 'a' - 'A';
        s++;
    }
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STR_TO_UPPER                                                      │
    ╰───────────────────────────────────────────────────────────────────╯ */
void str_to_upper(char *s) {
    while (*s != '\0') {
        if (*s >= 'a' && *s <= 'z')
            *s = *s + 'A' - 'a';
        s++;
    }
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STRNZ_CPY                                                         │
    │ stops at max_len, newline, or carriage return                     │
    │ max_len limits the destination buffer size                        │
    │ returns length of resulting string                                │
    ╰───────────────────────────────────────────────────────────────────╯ */
int strnz__cpy(char *d, char *s, int max_len) {
    char *e;
    int len = 0;

    e = d + max_len;
    while (*s != '\0' && *s != '\n' && *s != '\r' && d < e) {
        *d++ = *s++;
        len++;
    }
    *d = '\0';
    return len;
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STRNZ_CAT                                                         │
    │ stops at max_len, newline, or carriage return                     │
    │ max_len limits the destination buffer size                        │
    │ returns length of resulting string                                │
    ╰───────────────────────────────────────────────────────────────────╯ */
int strnz__cat(char *d, char *s, int max_len) {
    char *e;
    int len = 0;

    e = d + max_len;
    while (*d != '\0' && *d != '\n' && *d != '\r' && d < e) {
        d++;
        len++;
    }
    while (*s != '\0' && *s != '\n' && *s != '\r' && d < e) {
        *d++ = *s++;
        len++;
    }
    *d = '\0';
    return len;
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STRZ                                                              │
    │ Don't use - deprecated                                            │
    │ Use strnz instead                                                 │
    ╰───────────────────────────────────────────────────────────────────╯ */
void strz(char *s) {
    while (*s != '\0' && *s != '\n' && *s != '\r')
        s++;
    *s = '\0';
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STRNZ                                                             │
    │ terminates string at '\n', '\r', or max_len                       │
    │ returns length of resulting string                                │
    ╰───────────────────────────────────────────────────────────────────╯ */
int strnz(char *s, int max_len) {
    char *e;
    int len = 0;

    e = s + max_len;
    while (*s != '\0' && *s != '\n' && *s != '\r' && s < e) {
        s++;
        len++;
    }
    *s = '\0';
    return (len);
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STRNZ_DUP                                                         │
    │ terminates string at '\n', '\r', or l                             │
    │ returns pionter to allocated memory                               │
    ╰───────────────────────────────────────────────────────────────────╯ */
char *strnz_dup(char *s, int l) {
    char *p, *rs, *e;
    int m;

    for (p = s, m = 1; *p != '\0'; p++, m++)
        ;
    rs = p = (char *)malloc(m);
    if (rs != NULL) {
        e = rs + l;
        while (*s != '\0' && *s != '\n' && *s != '\r' && p < e)
            *p++ = *s++;
        *p = '\0';
    }
    return (rs);
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STRZ_DUP                                                          │
    │ Dont use - deprecated                                             │
    │ Use strnz_dup instead                                             │
    ╰───────────────────────────────────────────────────────────────────╯ */
char *strz_dup(char *s) {
    char *p, *rs;
    int m;

    for (p = s, m = 1; *p != '\0'; p++, m++)
        ;
    rs = p = (char *)malloc(m);
    if (rs != NULL) {
        while (*s != '\0')
            *p++ = *s++;
        *p = '\0';
    }
    return (rs);
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STR_SUBC                                                          │
    │ Use strnz_dup instead                                             │
    │ Replaces "ReplaceChr" in "s" with "Withstr" in "d"                │
    │ won't move more than "l" bytes to "d"                            │
    ╰───────────────────────────────────────────────────────────────────╯ */
void str_subc(char *d, char *s, char ReplaceChr, char *Withstr, int l) {
    char *e;

    e = d + l;
    while (*s != '\0' && d < e) {
        if (*s == ReplaceChr) {
            while (*Withstr != '\0' && d < e)
                *d++ = *Withstr++;
            s++;
        } else
            *d++ = *s++;
    }
    *d = '\0';
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ CHREP                                                             │
    │ Replace all occurrences of old_chr with new_chr in string         │
    ╰───────────────────────────────────────────────────────────────────╯ */
// replace old_chr with new_chr in string s
void chrep(char *s, char old_chr, char new_chr) {
    while (*s != '\0') {
        if (*s == old_chr)
            *s = new_chr;
        s++;
    }
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ NORMALIZE_FILE_SPEC                                               │
    │ I forgot what this was supposed to do? Someone suggested it might │
    │ have been to replace backslashes with forward slashes, but why?   │
    │ Supposedly, some deprecated OS used backslashes as directory      │
    │ delimiters. Seems far-fetched.                                    │
    ╰───────────────────────────────────────────────────────────────────╯ */
void normalize_file_spec(char *fs) {
    while (*fs != '\0') {
        if (*fs == '\\')
            *fs = '/';
        fs++;
    }
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ FILE_SPEC_PATH                                                    │
    │ Returns the path component of a file specification.               │
    ╰───────────────────────────────────────────────────────────────────╯ */
void file_spec_path(char *fp, char *fs) {
    char *d, *l, *s;

    // get path component of file spec
    s = fp;
    d = fs;
    l = NULL;
    while (*s != '\0') {
        if (*s == '/')
            l = d;
        *d++ = *s++;
    }
    if (l == NULL)
        *fp = '\0'; /* no slash, so no path */
    else
        *l = '\0';
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ FILE_SPEC_NAME                                                    │
    │ Returns the file name component of a file specification.          │
    ╰───────────────────────────────────────────────────────────────────╯ */
void file_spec_name(char *fn, char *fs) {
    char *d, *l, *s;

    // get name component of file spec
    l = NULL;
    s = fs;
    while (*s != '\0') {
        if (*s == '/')
            l = s;
        s++;
    }
    if (l == NULL)
        s = fs;
    else
        s = ++l;
    d = fn;
    while (*s != '\0')
        *d++ = *s++;
    *d = '\0';
}
double str_to_double(char *s) {
    char *e;
    double d;

    if (!s || !*s)
        return false;
    d = strtod(s, &e);
    return d;
}
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STR_TO_BOOL                                                       │
    │ Converts generalized boolean to true or false.                    │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool str_to_bool(const char *s) {
    if (!s)
        return false;

    switch (s[0]) {
    case 't':
    case 'T':
    case 'y':
    case 'Y':
    case '1':
        return true;
    case 'o':
    case 'O':
        switch (s[1]) {
        case 'n':
        case 'N':
            return true;
        default:
            break;
        }
    default:
        break;
    }
    return false;
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ EXPAND_TILDE                                                      │
    │ Converts ~ to "$HOME"                                             │
    ╰───────────────────────────────────────────────────────────────────╯ */
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ TRIM_PATH                                                         │
    │ Removes extraneous characters from path                           │
    ╰───────────────────────────────────────────────────────────────────╯ */
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ TRIM_EXT                                                          │
    │ Removes characters to the right of the rightmost period           │
    ╰───────────────────────────────────────────────────────────────────╯ */
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ BASE_NAME                                                         │
    │ Returns the base name of a file specification                     │
    │ "buf" must be large enough to receive the result                  │
    ╰───────────────────────────────────────────────────────────────────╯ */
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ DIR_NAME                                                          │
    │ Returns the directory name of a file specification                │
    │ "buf" must be large enough to receive the result                  │
    ╰───────────────────────────────────────────────────────────────────╯ */
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

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ DIR_NAME                                                          │
    │ Returns true if the directory exists and is accessable with the   │
    │ mode specified                                                    │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool verify_dir(char *spec, int mode) {
    char tmp_str[MAXLEN];
    expand_tilde(spec, MAXLEN);
    if (faccessat(AT_FDCWD, spec, mode, AT_EACCESS) == 0)
        return true;
    else {
        if (errno == EACCES) {
            if (mode == W_OK) {
                strncpy(tmp_str, "Directory ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not writable (permission denied).",
                        MAXLEN - 1);
                Perror(tmp_str);
            }
            if (mode == R_OK) {
                strncpy(tmp_str, "Directory ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not readable (permission denied).",
                        MAXLEN - 1);
                Perror(tmp_str);
            }
        } else if (errno == ENOENT) {
            strncpy(tmp_str, "Directory ", MAXLEN - 1);
            strncat(tmp_str, spec, MAXLEN - 1);
            strncat(tmp_str, " does not exist.", MAXLEN - 1);
            Perror(tmp_str);
        } else
            return true;
        return false;
    }
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ VERIFY_DIR_Q (quietly)                                            │
    │ Returns true if the directory exists and is accessable with the   │
    │ mode specified. Does not throw an error.                          │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool verify_dir_q(char *spec, int mode) {
    expand_tilde(spec, MAXLEN);
    if (faccessat(AT_FDCWD, spec, mode, AT_EACCESS) == 0)
        return true;
    return false;
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ VERIFY_FILE                                                       │
    │ Returns true if the file exists and is accessable with the mode   │
    │ specified.                                                        │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool verify_file(char *spec, int imode) {
    char dirbuf[MAXLEN];
    int mode = imode & ~0x1000;
    expand_tilde(spec, MAXLEN);
    if (faccessat(AT_FDCWD, spec, mode, AT_EACCESS) == 0)
        return true;
    else {
        if (errno == EACCES) {
            if (mode & W_OK) {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not writable (permission denied).",
                        MAXLEN - 1);
            }
            if (mode & R_OK) {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not readable (permission denied).",
                        MAXLEN - 1);
            }
            if (mode & X_OK) {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " is not executable (permission denied).",
                        MAXLEN - 1);
            }
            Perror(tmp_str);
            return false;
        } else if (errno == ENOENT) {
            if (imode & WC_OK) {
                dir_name(dirbuf, spec);
                if (faccessat(AT_FDCWD, dirbuf, W_OK, AT_EACCESS) == 0)
                    return true;
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, "does not exist and cannot be created",
                        MAXLEN - 1);
                Perror(tmp_str);
                return false;
            } else {
                strncpy(tmp_str, "File ", MAXLEN - 1);
                strncat(tmp_str, spec, MAXLEN - 1);
                strncat(tmp_str, " does not exist.", MAXLEN - 1);
                Perror(tmp_str);
                return false;
            }
        }
        strncpy(tmp_str, "Error accessing file ", MAXLEN - 1);
        strncat(tmp_str, spec, MAXLEN - 1);
        strncat(tmp_str, ": ", MAXLEN - 1);
        strncat(tmp_str, strerror(errno), MAXLEN - 1);
        Perror(tmp_str);
        return false;
    }
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ VERIFY_FILE_Q (Quietly)                                           │
    │ Returns true if the directory exists and is accessable with the   │
    │ mode specified. Does not throw an error.                          │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool verify_file_q(char *spec, int imode) {
    int mode = imode & ~0x1000;
    expand_tilde(spec, MAXLEN);
    if (faccessat(AT_FDCWD, spec, mode, AT_EACCESS) == 0)
        return true;
    return false;
}

/*  ╭───────────────────────────────────────────────────────────────────╮
    │ LOCATE_FILE_IN_PATH                                               │
    │ Searches all directories in the PATH environment variable and     │
    │ returns true, along with the first matching file in "file_spec"   │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool locate_file_in_path(char *file_spec, char *file_name) {
    char path[MAXLEN];
    char fn[MAXLEN];
    char *p, *fnp, *dir;

    strncpy(fn, file_name, MAXLEN - 1);
    fnp = fn;
    while (*fnp && *fnp != '/')
        fnp++;
    if (*fnp == '/')
        return false;
    if ((p = getenv("PATH")) == NULL)
        return false;
    strcpy(path, p);
    dir = strtok(path, ":");
    while (dir != NULL) {
        strncpy(file_spec, dir, MAXLEN - 1);
        strncat(file_spec, "/", MAXLEN - 1);
        strncat(file_spec, file_name, MAXLEN - 1);
        if (access(file_spec, F_OK) == 0) {
            return true;
        }
        dir = strtok(NULL, ":");
    }
    return false;
}
