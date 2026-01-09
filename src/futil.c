//  futil.c
//  Bill Waller Copyright (c) 2025
//  Utility functions for MENU
//  billxwaller@gmail.com

#include "menu.h"
#include <ctype.h>
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
bool stripz_quotes(char *);
void strip_quotes(char *);
int ssnprintf(char *, size_t, const char *, ...);
bool str_to_bool(const char *);
int str_to_args(char **, char *, int);
double str_to_double(char *);
void str_to_lower(char *);
void str_to_upper(char *);
int strnz__cpy(char *, const char *, int);
int strnz__cat(char *, const char *, int);
void strz(char *);
int strnz(char *, int);
char *strz_dup(char *);
char *strzdup(char *);
void str_subc(char *, char *, char, char *, int);
char *rep_substring(const char *, const char *, const char *);
void normalize_file_spec(char *);
void file_spec_path(char *, char *);
void file_spec_name(char *, char *);
bool verify_file(char *, int);
bool verify_dir(char *, int);
bool locate_file_in_path(char *, char *);
int canonicalize_file_spec(char *);
char errmsg[MAXLEN];
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ RTRIM - Removes Trailing Whitespace                               │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to trim
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ TRIM - Removes Leading and Trailing Whitespace                    │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to trim
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
/// ╭───────────────────────────────────────────────────────────────╮
/// │ SSNPRINTF                                                     │
/// ╰───────────────────────────────────────────────────────────────╯
/// A safe snprintf function that ensures the buffer is not overflowed.
/// Returns the number of characters that would have been written if
/// enough space had been available.
int ssnprintf(char *buf, size_t buf_size, const char *format, ...) {
    int n;
    va_list args;

    va_start(args, format);
    n = vsnprintf(buf, buf_size, format, args);
    va_end(args);

    return n;
}
/// ╭───────────────────────────────────────────────────────────────╮
/// │ STR_TO_ARGS - Break String Into Arguments                     │
/// ╰───────────────────────────────────────────────────────────────╯
/// @params argv - array of pointers to arguments
/// @params arg_str - string containing arguments
/// @params max_args - maximum number of arguments to parse
/// @returns number of arguments parsed
int str_to_args(char *argv[], char *arg_str, int max_args) {
    int argc = 0;
    char *p = arg_str;
    char tmp_str[MAXLEN];
    char *arg_start;
    int in_quotes = 0;
    char *d = tmp_str;

    while (*p != '\0' && argc < max_args) {
        while (isspace((unsigned char)*p))
            p++;
        if (*p == '\0')
            break;
        if (*p == '"') {
            in_quotes = 1;
            p++;
        }
        while (*p != '\0') {
            if (in_quotes) {
                if (*p == '\\' && *(p + 1) == '"') {
                    *d++ = '"';
                    p += 2;
                } else if (*p == '"') {
                    *d++ = '\0';
                    p++;
                    in_quotes = 0;
                    break;
                } else
                    *d++ = *p++;
            } else {
                if (isspace((unsigned char)*p)) {
                    *d++ = '\0';
                    p++;
                    break;
                } else
                    *d++ = *p++;
            }
        }
        *d = '\0';
        d = tmp_str;
        arg_start = (char *)malloc(strlen(tmp_str) + 1);
        strnz__cpy(arg_start, tmp_str, MAXLEN - 1);
        argv[argc++] = arg_start;
    }
    return argc;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STR_TO_LOWER - Replace Upper with Lower Case                      │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to convert
void str_to_lower(char *s) {
    while (*s != '\0') {
        if (*s >= 'A' && *s <= 'Z')
            *s = *s + 'a' - 'A';
        s++;
    }
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STR_TO_UPPER - Replace Lower with Upper Case                      │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to convert
void str_to_upper(char *s) {
    while (*s != '\0') {
        if (*s >= 'a' && *s <= 'z')
            *s = *s + 'A' - 'a';
        s++;
    }
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRNZ_CPY                                                         │
/// │ stops at max_len, newline, or carriage return                     │
/// │ max_len limits the destination buffer size                        │
/// │ returns length of resulting string                                │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params d - destination string
/// @params s - source string
/// @params max_len - maximum length to copy
/// @returns length of resulting string
int strnz__cpy(char *d, const char *s, int max_len) {
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRNZ_CAT                                                         │
/// │ stops at max_len, newline, or carriage return                     │
/// │ max_len limits the destination buffer size                        │
/// │ returns length of resulting string                                │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params d - destination string
/// @params s - source string
/// @params max_len - maximum length to copy
/// @returns length of resulting string
int strnz__cat(char *d, const char *s, int max_len) {
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRZ                                                              │
/// │ Don't use - deprecated                                            │
/// │ Use strnz instead                                                 │
/// ╰───────────────────────────────────────────────────────────────────╯
void strz(char *s) {
    while (*s != '\0' && *s != '\n' && *s != '\r')
        s++;
    *s = '\0';
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRNZ                                                             │
/// │ terminates string at '\n', '\r', or max_len                       │
/// │ returns length of resulting string                                │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to terminate
/// @params max_len - maximum length to scan
/// @returns length of resulting string
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRNZ_DUP                                                         │
/// │ terminates string at '\n', '\r', or l                             │
/// │ returns pionter to allocated memory                               │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to duplicate
/// @params l - maximum length to copy
/// @returns pointer to allocated memory
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRZ_DUP - Duplicate String Allocating Memory                     │
/// │ Dont use - deprecated                                             │
/// │ Use strnz_dup instead                                             │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to duplicate
/// @returns pointer to allocated memory
/// @note - caller must free returned memory
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STR_SUBC - Substitute Character with String                       │
/// │ Replaces "ReplaceChr" in "s" with "Withstr" in "d"                │
/// │ won't move more than "l" bytes to "d"                             │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params d - destination string
/// @params s - source string
/// @params ReplaceChr - character to replace
/// @params Withstr - string to insert
/// @params l - maximum length to copy
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRNFILL - Fill String With Character                             │
/// │ Replace all occurrences of old_chr with new_chr in string         │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to fill
/// @params c - character to fill with
/// @params n - number of characters to fill
void strnfill(char *s, char c, int n) {
    char *e;

    e = s + n;
    while (s < e)
        *s++ = c;
    *s = '\0';
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRIP_QUOTES - Remove Quotes from String                          │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to strip quotes from
/// removes leading and trailing double quotes if present
/// void strip_quotes(char *s);
void strip_quotes(char *s) {
    int l = strlen(s);
    if (l > 1 && s[l - 1] == '\"') {
        memmove(s, s + 1, l - 2);
        s[l - 2] = '\0';
    }
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STRIPZ_QUOTES - Remove Quotes from String                         │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to strip quotes from
/// removes leading and trailing double quotes if present
/// @note Same as STRIP_QUOTES but returns true if quotes were removed
bool stripz_quotes(char *s) {
    int l = strlen(s);
    if (l > 1 && s[l - 1] == '\"') {
        memmove(s, s + 1, l - 2);
        s[l - 2] = '\0';
        return true;
    }
    return false;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ CHREP                                                             │
/// │ Replace all occurrences of old_chr with new_chr in string         │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to modify
/// @params old_chr - character to replace
/// @params new_chr - character to insert
void chrep(char *s, char old_chr, char new_chr) {
    while (*s != '\0') {
        if (*s == old_chr)
            *s = new_chr;
        s++;
    }
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ NORMALIZE_FILE_SPEC                                               │
/// │ I forgot what this was supposed to do? Someone suggested it might │
/// │ have been to replace backslashes with forward slashes, but why?   │
/// │ Supposedly, some deprecated OS used backslashes as directory      │
/// │ delimiters. Seems far-fetched.                                    │
/// ╰───────────────────────────────────────────────────────────────────╯
void normalize_file_spec(char *fs) {
    while (*fs != '\0') {
        if (*fs == '\\')
            *fs = '/';
        fs++;
    }
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ FILE_SPEC_PATH                                                    │
/// │ Returns the path component of a file specification.               │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params fp - path component to return
/// @params fs - full file specification
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
        *fp = '\0'; // no slash, so no path
    else
        *l = '\0';
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ FILE_SPEC_NAME                                                    │
/// │ Returns the file name component of a file specification.          │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params fn - name component to return
/// @params fs - full file specification
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STR_TO_DOUBLE - Convert String to Double                          │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to convert
/// @returns converted double value
double str_to_double(char *s) {
    char *e;
    double d;

    if (!s || !*s)
        return false;
    d = strtod(s, &e);
    return d;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ STR_TO_BOOL - Converts String to Boolean true or false            │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params s - string to convert
/// @returns converted boolean value
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

/// ╭───────────────────────────────────────────────────────────────────╮
/// │ EXPAND_TILDE - Replace Leading Tilde With Home Directory          │
/// │ Converts ~ to "$HOME"                                             │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params path - path to expand
/// @params path_maxlen - maximum length of path
/// @returns true if successful
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
            strnz__cpy(ts, e, path_maxlen - 1);
            strnz__cat(ts, "/", path_maxlen - 1);
            strnz__cat(ts, tp, path_maxlen - 1);
            strnz__cpy(path, ts, path_maxlen - 1);
        }
    }
    return true;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ TRIM_PATH - Removes Extraneous Characters From Path               │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params dir - directory path to trim
/// @returns true if successful
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ TRIM_EXT - Trim File Extension                                    │
/// │ Removes characters to the right of the rightmost period           │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params buf - buffer to receive result
/// @params filename - filename to trim
/// @returns true if successful
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ BASE_NAME - Extracts File Name from File Specification            │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params buf - buffer to receive result
/// @params path - file specification
/// @returns true if successful
/// @note - "buf" must be large enough to receive the result
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ DIR_NAME - Returns the Directory Name of a File Specification     │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params buf - buffer to receive result
/// @params path - file specification
/// @returns true if successful
/// @note "buf" must be large enough to receive the result
bool dir_name(char *buf, char *path) {
    if (!path || !*path || !buf)
        return false;
    char tmp_str[MAXLEN];
    strnz__cpy(tmp_str, path, MAXLEN);
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
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ VERIFY_DIR                                                        │
/// │ Returns true if the directory exists and is accessable with the   │
/// │ mode specified. Does not throw an error.                          │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params spec - directory specification
/// @params imode - access mode
/// @returns true if successful
/// note - imode can include S_WCOK and S_QUIET flags
/// these flags are stripped before calling faccessat
/// S_WCOK - Write or Create
/// S_QUIET - Suppress Error Messages
bool verify_dir(char *spec, int imode) {
    int mode = imode & ~(S_WCOK | S_QUIET);
    expand_tilde(spec, MAXLEN);
    struct stat sb;
    errno = 0;
    src_line = 0;

    if (faccessat(AT_FDCWD, spec, mode, AT_EACCESS) != 0) {
        src_line = __LINE__ - 2;
        src_name = __FILE__;
        strnz__cpy(fn, "faccessat", MAXLEN - 1);
    } else if (fstatat(AT_FDCWD, spec, &sb, 0) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strnz__cpy(fn, "fstatat", MAXLEN - 1);
    }
    if (errno != 0)
        strerror_r(errno, em2, MAXLEN - 1);
    else if ((sb.st_mode & S_IFMT) != S_IFDIR) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strnz__cpy(fn, "verify_file", MAXLEN - 1);
        strnz__cpy(em2, "Not a regular file.", MAXLEN - 1);
    }
    if (src_line != 0) {
        ssnprintf(em0, MAXLEN - 1, "%s failed in %s at line %d", fn, src_name,
                  src_line);
        strnz__cpy(em1, spec, MAXLEN - 1);
        strnz__cpy(em3, "Check the file", MAXLEN - 1);
        display_error(em0, em1, em2, em3);
        return false;
    }
    return true;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ VERIFY_FILE                                                       │
/// │ Returns true if the file exists and is accessable with the mode   │
/// │ specified.                                                        │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params in_spec - file specification
/// @params imode - access mode
/// @returns true if successful
bool verify_file(char *in_spec, int imode) {
    int rc;
    struct stat sb;
    char spec[MAXLEN];
    strnz__cpy(spec, in_spec, MAXLEN - 1);
    int mode = imode & ~(S_WCOK | S_QUIET);
    errno = 0;
    src_line = 0;
    canonicalize_file_spec(spec);
    expand_tilde(spec, MAXLEN);
    if ((rc = faccessat(AT_FDCWD, spec, mode, AT_EACCESS)) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strnz__cpy(fn, "faccessat", MAXLEN - 1);
    } else if ((rc = fstatat(AT_FDCWD, spec, &sb, 0)) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strnz__cpy(fn, "fstatat", MAXLEN - 1);
    }
    if (errno != 0)
        strerror_r(errno, em2, MAXLEN - 1);
    else if ((sb.st_mode & S_IFMT) != S_IFREG) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strnz__cpy(fn, "verify_file", MAXLEN - 1);
        strnz__cpy(em2, "Not a regular file.", MAXLEN - 1);
    }
    if (src_line != 0) {
        if (imode & S_QUIET)
            return false;
        ssnprintf(em0, MAXLEN - 1, "%s failed in %s at line %d", fn, src_name,
                  src_line);
        strnz__cpy(em1, spec, MAXLEN - 1);
        strnz__cpy(em3, "Check the file", MAXLEN - 1);
        display_error(em0, em1, em2, em3);
        return false;
    }
    return true;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ LOCATE_FILE_IN_PATH                                               │
/// │ Searches all directories in the PATH environment variable and     │
/// │ returns true, along with the first matching file in "file_spec"   │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params file_spec - buffer to receive located file specification
/// @params file_name - name of file to locate
/// @returns true if file is located
bool locate_file_in_path(char *file_spec, char *file_name) {
    char path[MAXLEN];
    char fn[MAXLEN];
    char *p, *fnp, *dir;

    canonicalize_file_spec(file_name);
    strnz__cpy(fn, file_name, MAXLEN - 1);
    fnp = fn;
    while (*fnp && *fnp != '/')
        fnp++;
    if (*fnp == '/')
        return false;
    if ((p = getenv("PATH")) == NULL)
        return false;
    strnz__cpy(path, p, MAXLEN - 1);
    dir = strtok(path, ":");
    while (dir != NULL) {
        strnz__cpy(file_spec, dir, MAXLEN - 1);
        strnz__cat(file_spec, "/", MAXLEN - 1);
        strnz__cat(file_spec, file_name, MAXLEN - 1);
        if (access(file_spec, F_OK) == 0) {
            return true;
        }
        dir = strtok(NULL, ":");
    }
    return false;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ LIST_FILES                                                        │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params dir - directory to list files from
/// @params regexp - regular expression to match files
/// @params f_recurse - true to recurse into subdirectories
bool list_files(char *dir, char *regexp, bool f_recurse) {
    if (f_recurse) {
        lf_find_files(dir, regexp);
        lf_find_dirs(dir, regexp);
    } else {
        lf_find_files(dir, regexp);
    }
    return true;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ LF_FIND_DIRS                                                      │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params dir - directory to search
/// @params re - regular expression to match
/// @returns true if successful
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
            strnz__cpy(file_spec, dir, MAXLEN - 1);
            if (file_spec[strlen(file_spec) - 1] != '/')
                strnz__cat(file_spec, "/", MAXLEN - 1);
            strnz__cat(file_spec, dir_st->d_name, MAXLEN - 1);
            if (stat(file_spec, &sb) == -1) {
                strnz__cpy(tmp_str, "can't find ", MAXLEN - 1);
                strnz__cat(tmp_str, file_spec, MAXLEN - 1);
                perror(tmp_str);
                return false;
            }
            if ((sb.st_mode & S_IFMT) == S_IFDIR) {
                strnz__cpy(dir_s, file_spec, MAXLEN - 1);
                lf_find_files(dir_s, re);
                lf_find_dirs(dir_s, re);
            }
        }
        dir_st = readdir(dirp);
    }
    closedir(dirp);
    return true;
}
/// ╭───────────────────────────────────────────────────────────────────╮
/// │ LF_FIND_FILES                                                     │
/// ╰───────────────────────────────────────────────────────────────────╯
/// @params dir - directory to search
/// @params re - regular expression to match
/// @returns true if successful
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
            strnz__cpy(file_spec, dir, MAXLEN - 1);
            if (file_spec[strlen(file_spec) - 1] != '/')
                strnz__cat(file_spec, "/", MAXLEN - 1);
            strnz__cat(file_spec, dir_st->d_name, MAXLEN - 1);
            if (stat(file_spec, &sb) == -1) {
                strnz__cpy(tmp_str, "can't find ", MAXLEN - 1);
                strnz__cat(tmp_str, file_spec, MAXLEN - 1);
                perror(tmp_str);
                return false;
            }
            if ((sb.st_mode & S_IFMT) == S_IFDIR)
                strnz__cat(file_spec, "/", MAXLEN - 1);
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

/// ╭───────────────────────────────────────────────────────╮
/// │ trim at first space and remove quotes                 │
/// ╰───────────────────────────────────────────────────────╯
/// @params spec - file specification to canonicalize
/// @returns length of resulting string
int canonicalize_file_spec(char *spec) {
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
    strnz__cpy(spec, tmp_s, MAXLEN - 1);
    l = strlen(spec);
    return l;
}
/// ╭───────────────────────────────────────────────────────╮
/// │ REP_SUBSTRING - Replace Substring                     │
/// ╰───────────────────────────────────────────────────────╯
/// @params org_s - original string
/// @params tgt_s - target substring to replace
/// @params rep_s - replacement substring
/// @returns pointer to newly allocated string with replacements
char *rep_substring(const char *org_s, const char *tgt_s, const char *rep_s) {
    char *out_s, *ip, *tmp;
    int tgt_l = strlen(tgt_s);
    int rep_l = strlen(rep_s);
    int head_l;
    int n = 0;
    ip = (char *)org_s;
    while ((tmp = strstr(ip, tgt_s)) != NULL) {
        n++;
        ip = tmp + tgt_l;
    }
    out_s = malloc(strlen(org_s) + (rep_l - tgt_l) * n + 1);
    if (!out_s)
        return NULL;
    tmp = out_s;
    ip = (char *)org_s;
    while (n--) {
        char *p = strstr(ip, tgt_s);
        head_l = p - ip;
        strnz__cpy(tmp, ip, head_l);
        tmp += head_l;
        strnz__cpy(tmp, rep_s, MAXLEN - 1);
        tmp += rep_l;
        ip += head_l + tgt_l;
    }
    strnz__cpy(tmp, ip, MAXLEN - 1);
    return out_s;
}
