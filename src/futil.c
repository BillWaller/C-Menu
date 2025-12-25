/* futil.c
 * Utility functions for MENU
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>

bool list_files(char *, char *, bool);
bool lf_find_files(char *, char *);
bool lf_find_dirs(char *, char *);
bool lf_write_file(int, char *);

int trim(char *);
int rtrim(char *);
void strip_quotes(char *);
int ssnprintf(char *, size_t, const char *, ...);
bool str_to_bool(const char *);
int str_to_args(char **, char *, int);
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
int canonicalize_file_spec(char *);
char errmsg[MAXLEN];
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ RTRIM                                                             │
    ╰───────────────────────────────────────────────────────────────────╯ */
int rtrim(char *s) {
    char *p = s;
    char *d = s;
    while (*p != '\0')
        *d++ = *p++;
    while (*(d - 1) == ' ' && d > s)
        d--;
    *d = '\0';
    return d - s;
}
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
int str_to_args(char **argv, char *cmd_line, int max_args) {
    int argc = 0;
    char *d = cmd_line;
    char *arg_start;
    bool in_quotes = false;
    char quote_char = '\0';

    while (*d != '\0' && argc < max_args - 1) {
        while (*d == ' ' || *d == '\t')
            d++;
        if (*d == '\0')
            break;
        arg_start = d;
        while (*d != '\0') {
            if (in_quotes) {
                if (*d == quote_char) {
                    in_quotes = false;
                    d++;
                    break;
                }
            } else {
                if (*d == '"' || *d == '\'') {
                    in_quotes = true;
                    quote_char = *d;
                } else if (*d == ' ' || *d == '\t') {
                    break;
                }
            }
            d++;
        }
        size_t arg_length = d - arg_start;
        argv[argc] = (char *)malloc(arg_length + 1);
        strncpy(argv[argc], arg_start, arg_length);
        argv[argc][arg_length] = '\0';
        trim(argv[argc]);
        argc++;
    }
    argv[argc] = NULL;
    return argc;
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
void strnfill(char *s, char c, int n) {
    char *e;

    e = s + n;
    while (s < e)
        *s++ = c;
    *s = '\0';
}
void strip_quotes(char *s) {
    int l = strlen(s);
    if (l > 1 && s[l - 1] == '\"') {
        memmove(s, s + 1, l - 2);
        s[l - 2] = '\0';
    }
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
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ STR_TO_DOUBLE                                                     │
    ╰───────────────────────────────────────────────────────────────────╯ */
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
    │ VERIFY_DIR                                                        │
    │ Returns true if the directory exists and is accessable with the   │
    │ mode specified. Does not throw an error.                          │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool verify_dir(char *spec, int imode) {
    int rc;
    int mode = imode & ~(S_WCOK | S_QUIET);
    expand_tilde(spec, MAXLEN);
    struct stat sb;
    errno = 0;
    src_line = 0;

    if ((rc = faccessat(AT_FDCWD, spec, mode, AT_EACCESS)) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strncpy(fn, "faccessat", MAXLEN - 1);
    } else if ((rc = fstatat(AT_FDCWD, spec, &sb, 0)) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strncpy(fn, "fstatat", MAXLEN - 1);
    }
    if (errno != 0)
        strerror_r(errno, em2, MAXLEN - 1);
    else if ((sb.st_mode & S_IFMT) != S_IFDIR) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strncpy(fn, "verify_file", MAXLEN - 1);
        strncpy(em2, "Not a regular file.", MAXLEN - 1);
    }
    if (src_line != 0) {
        ssnprintf(em0, MAXLEN - 1, "%s failed in %s at line %d", fn, src_name,
                  src_line);
        strncpy(em1, spec, MAXLEN - 1);
        strncpy(em3, "Check the file", MAXLEN - 1);
        display_error(em0, em1, em2, em3);
        return false;
    }
    return true;
}
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ VERIFY_FILE                                                       │
    │ Returns true if the file exists and is accessable with the mode   │
    │ specified.                                                        │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool verify_file(char *spec, int imode) {
    int rc;
    int mode = imode & ~(S_WCOK | S_QUIET);
    expand_tilde(spec, MAXLEN);
    struct stat sb;
    errno = 0;
    src_line = 0;

    canonicalize_file_spec(spec);

    if ((rc = faccessat(AT_FDCWD, spec, mode, AT_EACCESS)) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strncpy(fn, "faccessat", MAXLEN - 1);
    } else if ((rc = fstatat(AT_FDCWD, spec, &sb, 0)) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strncpy(fn, "fstatat", MAXLEN - 1);
    }
    if (errno != 0)
        strerror_r(errno, em2, MAXLEN - 1);
    else if ((sb.st_mode & S_IFMT) != S_IFREG) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strncpy(fn, "verify_file", MAXLEN - 1);
        strncpy(em2, "Not a regular file.", MAXLEN - 1);
    }
    if (src_line != 0) {
        if (imode & S_QUIET)
            return false;
        ssnprintf(em0, MAXLEN - 1, "%s failed in %s at line %d", fn, src_name,
                  src_line);
        strncpy(em1, spec, MAXLEN - 1);
        strncpy(em3, "Check the file", MAXLEN - 1);
        display_error(em0, em1, em2, em3);
        return false;
    }
    return true;
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

    canonicalize_file_spec(file_name);
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
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ LIST_FILES                                                        │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool list_files(char *dir, char *regexp, bool f_recurse) {
    if (f_recurse) {
        lf_find_files(dir, regexp);
        lf_find_dirs(dir, regexp);
    } else {
        lf_find_files(dir, regexp);
    }
    return true;
}
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ LF_FIND_DIRS                                                      │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool lf_find_dirs(char *dir, char *re) {
    struct stat sb;
    struct dirent *dir_st;
    DIR *dirp;
    char dir_s[MAXLEN];
    char file_spec[MAXLEN];

    if ((dirp = opendir(dir)) == 0)
        return false;
    dir_st = readdir(dirp);
    while (dir_st != NULL) {
        if (dir_st->d_ino != 0 && strcmp(dir_st->d_name, ".") != 0 &&
            strcmp(dir_st->d_name, "..") != 0) {
            strcpy(file_spec, dir);
            if (file_spec[strlen(file_spec) - 1] != '/')
                strcat(file_spec, "/");
            strcat(file_spec, dir_st->d_name);
            if (stat(file_spec, &sb) == -1) {
                strnz__cpy(tmp_str, "can't find ", MAXLEN - 1);
                strnz__cat(tmp_str, file_spec, MAXLEN - 1);
                perror(tmp_str);
                return false;
            }
            if ((sb.st_mode & S_IFMT) == S_IFDIR) {
                strcpy(dir_s, file_spec);
                lf_find_files(dir_s, re);
                lf_find_dirs(dir_s, re);
            }
        }
        dir_st = readdir(dirp);
    }
    closedir(dirp);
    return true;
}
/*  ╭───────────────────────────────────────────────────────────────────╮
    │ LF_FIND_FILES                                                     │
    ╰───────────────────────────────────────────────────────────────────╯ */
bool lf_find_files(char *dir, char *re) {
    struct stat sb;
    struct dirent *dir_st;
    DIR *dirp;
    int REG_FLAGS = REG_EXTENDED;
    int reti;
    regmatch_t pmatch[1];
    regex_t compiled_re;
    char file_spec[MAXLEN];
    char *file_spec_p;

    if (re[0] == '\0')
        return false;
    reti = regcomp(&compiled_re, re, REG_FLAGS);
    if (reti) {
        perror("Invalid pattern");
        return false;
    }
    if ((dirp = opendir(dir)) == 0)
        return false;
    dir_st = readdir(dirp);
    while (dir_st != NULL) {
        if (dir_st->d_ino != 0 && strcmp(dir_st->d_name, ".") != 0) {
            strcpy(file_spec, dir);
            if (file_spec[strlen(file_spec) - 1] != '/')
                strcat(file_spec, "/");
            strcat(file_spec, dir_st->d_name);
            if (stat(file_spec, &sb) == -1) {
                strnz__cpy(tmp_str, "can't find ", MAXLEN - 1);
                strnz__cat(tmp_str, file_spec, MAXLEN - 1);
                perror(tmp_str);
                return false;
            }
            if ((sb.st_mode & S_IFMT) == S_IFDIR)
                strcat(file_spec, "/");
            reti = regexec(&compiled_re, file_spec, compiled_re.re_nsub + 1,
                           pmatch, REG_FLAGS);
            if (reti == REG_NOMATCH) {
                // no match
            } else if (reti) {
                char msgbuf[100];
                regerror(reti, &compiled_re, msgbuf, sizeof(msgbuf));
                snprintf(tmp_str, MAXLEN - 1, "Regex match failed: %s\n",
                         msgbuf);
                perror(tmp_str);
                return false;
            } else {
                file_spec_p = file_spec;
                if (file_spec[0] == '.' && file_spec[1] == '/')
                    file_spec_p += 2;
                printf("%s\n", file_spec_p);
            }
        }
        dir_st = readdir(dirp);
    }
    closedir(dirp);
    return true;
}

int canonicalize_file_spec(char *spec) {
    /*  ╭───────────────────────────────────────────────────────╮
        │ trim at first space and remove quotes                 │
        ╰───────────────────────────────────────────────────────╯ */
    char tmp_s[MAXLEN];
    char *s;
    s = spec;
    char *d;
    d = tmp_s;
    int l = 0;
    while (*s != '\0') {
        if (*s == ' ')
            break;
        if (*s == '\"' || *s == '\'') {
            s++;
            continue;
            ;
        }
        *d++ = *s++;
        l++;
    }
    *d = '\0';
    // strnz__cpy(spec, tmp_s, MAXLEN - 1);
    strncpy(spec, tmp_s, MAXLEN - 1);
    l = strlen(spec);
    return l;
}
