/// futil.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
/// Utility functions for MENU

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

// for lf_find_files and lf_find_dirs
#define ALL 0x01
#define RECURSE 0x02
#define ICASE 0x04

bool list_files(char *, char *, bool);
bool lf_find_dirs(char *, char *, int, int);
bool lf_find_files(char *, char *, int);
int strip_ansi(char *, char *);
int a_toi(char *, bool *);
bool chrep(char *, char, char);
size_t trim(char *);
size_t rtrim(char *);
bool stripz_quotes(char *);
bool strip_quotes(char *);
bool str_to_bool(const char *);
int str_to_args(char **, char *, int);
double str_to_double(char *);
bool str_to_lower(char *);
bool str_to_upper(char *);
size_t strz(char *);
size_t strnz(char *, int);
char *strzdup(char *);
bool str_subc(char *, char *, char, char *, int);
char *rep_substring(const char *, const char *, const char *);
bool normalize_file_spec(char *);
bool file_spec_path(char *, char *);
bool file_spec_name(char *, char *);
bool verify_file(char *, int);
bool verify_dir(char *, int);
bool locate_file_in_path(char *, char *);
size_t canonicalize_file_spec(char *);
size_t ssnprintf(char *, size_t, const char *, ...);
size_t strnz__cpy(char *, const char *, size_t);
size_t strnz__cat(char *, const char *, size_t);
size_t string_cpy(String *, const String *);
size_t string_cat(String *, const String *);
size_t string_ncat(String *, const String *, size_t);
size_t string_ncpy(String *, const String *, size_t);
String to_string(const char *);
String mk_string(size_t);
bool free_string(String);
char errmsg[MAXLEN];
size_t rtrim(char *s) {
    ///  Trims trailing spaces from string s in place.
    ///  @param s - string to trim
    ///  @returns length of trimmed string
    if (s == NULL || *s == '\0')
        return 0;
    char *p = s;
    char *d = s;
    while (*p != '\0')
        *d++ = *p++;
    while (*(d - 1) == ' ' && d > s)
        d--;
    *d = '\0';
    return d - s;
}
size_t trim(char *s) {
    /// Trims leading and trailing spaces from string s in place.
    ///  @param s - string to trim
    ///  @returns length of trimmed string
    if (s == NULL || *s == '\0')
        return 0;
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
size_t ssnprintf(char *buf, size_t buf_size, const char *format, ...) {
    /// ssnprintf was designed to be a safer alternative to snprintf. It ensures
    /// that the buffer is not overflowed by taking the buffer size as a
    /// parameter and using vsnprintf internally. It also returns the number of
    /// characters that would have been written if enough space had been
    /// available, allowing the caller to detect truncation. This function is
    /// particularly useful in situations where the formatted string may exceed
    /// the buffer size, as it prevents buffer overflows and provides a way to
    /// handle such cases gracefully.
    /// @param buf - buffer to receive formatted string
    /// @param buf_size - size of buffer
    /// @param format - printf-style format string
    /// @param ... - arguments
    /// @returns number of characters that would have been written if enough
    /// space had been available
    int n;
    va_list args;

    va_start(args, format);
    n = vsnprintf(buf, buf_size, format, args);
    va_end(args);

    return n;
}
int str_to_args(char **argv, char *arg_str, int max_args) {
    ///  Converts a string into an array of argument strings.
    ///  Handles quoted strings and escaped quotes, preserving
    ///  text inside quotes as individual arguments. It has been
    ///  in service for many years without problems.
    ///  @param argv - array of pointers to arguments
    ///  @param arg_str - string containing arguments
    ///  @param max_args - maximum number of arguments to parse
    ///  @returns argc, a count of allocated vectors in argv
    ///  @note the caller is responsible for deallocating the strings in argv
    if (arg_str == NULL || *arg_str == '\0')
        return 0;
    int argc = 0;
    char *p = arg_str;
    char tmp_str[MAXLEN];
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
        argv[argc++] = strdup(tmp_str);
    }
    argv[argc] = NULL;
    return argc;
}
bool str_to_lower(char *s) {
    /// Converts a string to lowercase.
    ///  @param s - string to convert
    ///  @returns true if successful, false if s is NULL or empty
    if (s == NULL || *s == '\0')
        return false;
    while (*s != '\0') {
        if (*s >= 'A' && *s <= 'Z')
            *s = *s + 'a' - 'A';
        s++;
    }
    return true;
}
bool str_to_upper(char *s) {
    /// Converts a string to uppercase in place.
    ///  @param s - string to convert
    ///  @returns true if successful, false if s is NULL or empty
    if (s == NULL || *s == '\0')
        return false;
    while (*s != '\0') {
        if (*s >= 'a' && *s <= 'z')
            *s = *s + 'A' - 'a';
        s++;
    }
    return true;
}
size_t strnz__cpy(char *d, const char *s, size_t max_len) {
    /// strnz__cpy is designed to replace strncpy as a safer alternative. It
    /// copies string s to d, ensuring that the total length of d does not
    /// exceed max_len, and that the resulting string is null-terminated. It
    /// also treats newline and carriage return characters as string
    /// terminators, preventing them from being included in the result. This is
    /// particularly useful when copying user input or file data, where embedded
    /// newlines could cause issues.
    ///  @param d - destination string
    ///  @param s - source string
    ///  @param max_len - maximum length to copy
    ///  @returns length of resulting string
    char *e;
    int len = 0;
    if (s == NULL || d == NULL || max_len == 0) {
        if (d != NULL && max_len > 0)
            *d = '\0';
        return 0;
    }
    e = d + max_len;
    while (*s != '\0' && *s != '\n' && *s != '\r' && d < e) {
        *d++ = *s++;
        len++;
    }
    *d = '\0';
    return len;
}
size_t strnz__cat(char *d, const char *s, size_t max_len) {
    /// strnz__cat is designed to replace strncat as a safer alternative. It
    /// appends string s to d, ensuring that the total length of d does not
    /// exceed max_len, and that the resulting string is null-terminated. It
    /// also treats newline and carriage return characters as string
    /// terminators, preventing them from being included in the result. This is
    /// particularly useful when concatenating user input or file data, where
    /// embedded newlines could cause issues.
    /// @param d - destination string
    /// @param s - source string
    /// @param max_len - maximum length to copy
    /// @returns length of resulting string
    char *e;
    int len = 0;
    if (s == NULL || d == NULL || max_len == 0) {
        if (d != NULL && max_len > 0)
            *d = '\0';
        return 0;
    }
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
size_t strz(char *s) {
    ///  Terminates string at '\n' or '\r'
    int l = 0;
    if (s == NULL || *s == '\0')
        return 0;
    while (*s != '\0' && *s != '\n' && *s != '\r') {
        s++;
        l++;
    }
    *s = '\0';
    return l;
}
size_t strnz(char *s, int max_len) {
    ///  Terminates string at '\n', '\r', or max_len
    ///  The use case is to ensure that strings read from
    ///  files or user input do not contain embedded newlines or
    ///  carriage returns.
    ///  @param s string to terminate
    ///  @param max_len - maximum length to scan
    ///  @returns length of resulting string
    char *e;
    int len = 0;
    if (s == NULL || *s == '\0' || max_len == 0)
        return 0;
    e = s + max_len;
    while (*s != '\0' && *s != '\n' && *s != '\r' && s < e) {
        s++;
        len++;
    }
    *s = '\0';
    return (len);
}
char *strnz_dup(char *s, int l) {
    ///  Allocates memory for and duplicates string s up to
    ///  length l or until '\n' or '\r'
    ///  @param s - string to duplicate
    ///  @param l - maximum length to copy
    ///  @returns pointer to allocated memory
    char *p, *rs, *e;
    int m;
    if (s == NULL || *s == '\0' || l == 0)
        return NULL;
    for (p = s, m = 1; *p != '\0'; p++, m++)
        ;
    rs = p = (char *)malloc(m);
    if (rs != NULL) {
        e = rs + l;
        while (*s != '\0' && *s != '\n' && *s != '\r' && p < e)
            *p++ = *s++;
        *p = '\0';
    }
    return rs;
}
bool str_subc(char *d, char *s, char ReplaceChr, char *Withstr, int l) {
    ///  Replaces "ReplaceChr" in "s" with "Withstr" in "d"
    ///  won't move more than "l" bytes to "d"
    ///  @param d - destination string
    ///  @param s - source string
    ///  @param ReplaceChr - character to replace
    ///  @param Withstr - string to insert
    ///  @param l - maximum length to copy
    ///  @returns true if successful, false if any parameter is invalid
    ///  @note The caller must ensure that "d" has enough space to receive the
    ///  result, and that "l" is sufficient to hold the result. This function
    ///  does not perform any bounds checking on "d" or "Withstr", so it is the
    ///  caller's responsibility to ensure that they are valid and that "l" is
    ///  appropriate for the operation.
    ///  @returns true if successful, false if any parameter is invalid
    char *e;
    if (s == NULL || d == NULL || Withstr == NULL || l == 0) {
        if (d != NULL && l > 0)
            *d = '\0';
        return false;
    }
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
    return true;
}
bool strnfill(char *s, char c, int n) {
    ///  Fills string s with character c n
    ///  @param s - string to fill
    ///  @param c - character to fill with
    ///  @param n - number of characters to fill
    ///  @returns true if successful, false if s is NULL or n is non-positive
    if (s == NULL || n <= 0)
        return false;
    char *e;
    e = s + n;
    while (s < e)
        *s++ = c;
    *s = '\0';
    return true;
}
bool strip_quotes(char *s) {
    ///  removes leading and trailing double quotes if present
    ///  @param s - string to strip quotes from
    ///  @returns true if successful, false if s is NULL or empty
    if (s == NULL)
        return false;
    int l = strlen(s);
    if (l > 1 && s[l - 1] == '\"') {
        memmove(s, s + 1, l - 2);
        s[l - 2] = '\0';
    }
    return true;
}
bool stripz_quotes(char *s) {
    ///  @param s - string to strip quotes from
    ///  removes leading and trailing double quotes if present
    ///  @note Same as STRIP_QUOTES but returns true if quotes were removed
    if (s == NULL || strlen(s) < 2)
        return false;
    int l = strlen(s);
    if (l > 1 && s[0] == '\"' && s[l - 1] == '\"') {
        memmove(s, s + 1, l - 2);
        s[l - 2] = '\0';
        return true;
    }
    return false;
}
bool chrep(char *s, char old_chr, char new_chr) {
    ///  @param s - string to modify
    ///  @param old_chr - character to replace
    ///  @param new_chr - character to insert
    if (s == NULL)
        return false;
    while (*s != '\0') {
        if (*s == old_chr)
            *s = new_chr;
        s++;
    }
    return true;
}
int a_toi(char *s, bool *a_toi_error) {
    ///  ASCII to Integer Conversion with Error Checking
    ///  Accepts positive integers only
    ///  Negative numbers return an error (-1) in a_toi_error
    ///  @param s is the input string
    ///  @param a_toi_error is set to true if an error occurs, false otherwise
    int rc = -1;
    errno = 0;
    if (s && *s != 0)
        rc = (int)strtol(s, NULL, 10);
    if (rc < 0 || errno) {
        rc = -1;
        *a_toi_error = true;
    }
    return rc;
}
int strip_ansi(char *d, char *s) {
    /// Strips ANSI SGR escape sequences (ending in 'm') from string s to d
    /// @param d Destination string
    /// @param s Source string
    /// @return Length of stripped string
    /// Returns length of stripped string
    /// Example:
    /// char dest[1024];
    /// char src[] = "\033[31mThis is red text\033[0m
    /// int len = strip_ansi(dest, src);
    /// Result: dest = "This is red text", len = 17
    /// @note Only handles SGR sequences ending in 'm'
    /// @note Skips non-ASCII characters
    /// @note The caller must ensure that d has enough space to hold the
    /// stripped string
    /// @note This function does not allocate memory; it assumes d is
    /// pre-allocated
    /// @note This function processes the entire string until the null
    /// terminator
    /// @note This function does not modify the source string s
    int l = 0;
    while (*s) {
        if (*s == '\033') {
            while (*s && *s != 'm' && *s != 'K')
                s++;
            if (*s == 'm' || *s == 'K')
                s++;
            continue;
        } else {
            if ((unsigned char)*s <= 127) {
                *d++ = *s++;
                l++;
            } else
                s++;
        }
    }
    *d = '\0';
    return l;
}
bool normalize_file_spec(char *fs) {
    ///  replace backslashes with forward lashes
    ///  @param fs - file specification to normalize
    ///  @returns true if successful, false if fs is NULL or empty
    if (fs == NULL || *fs == '\0')
        return false;
    while (*fs != '\0') {
        if (*fs == '\\')
            *fs = '/';
        fs++;
    }
    return true;
}
bool file_spec_path(char *fp, char *fs) {
    ///  Returns the path component of a file specification.
    ///  get path component of file spec
    ///  @param fp - path component to return
    ///  @param fs - full file specification
    if (fs == NULL || *fs == '\0' || fp == NULL) {
        if (fp != NULL)
            *fp = '\0';
        return false;
    }
    char *d, *l, *s;
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
    return true;
}
bool file_spec_name(char *fn, char *fs) {
    ///  Returns the file name component of a file specification.
    ///  The user is responsible for ensuring that "fn" has enough space to
    ///  receive the result.
    ///  @param fn - name component to return
    ///  @param fs - full file specification
    if (fs == NULL || *fs == '\0' || fn == NULL) {
        if (fn != NULL)
            *fn = '\0';
        return false;
    }
    char *d, *l, *s;
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
    return true;
}
///  @returns converted double value
double str_to_double(char *s) {
    ///  This function is deprecated. Do not use in new code, and
    ///  replace in old code if possible.
    ///  It has an inherent flaw. It cannot signal conversion errors.
    ///  If the string is invalid, it returns 0.0, but that is also
    ///  a valid conversion result. The caller must ensure that the
    ///  string is a valid representation of a double before calling
    ///  this function.
    ///  @param s - string to convert
    char *e;
    double d;

    if (!s || !*s)
        return false;
    d = strtod(s, &e);
    return d;
}
bool str_to_bool(const char *s) {
    ///  Converts String to Boolean true or false
    ///  bool str_to_bool(const char *s);
    ///  @param s - string to convert
    ///  @returns converted boolean value
    if (s == NULL || *s == '\0')
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

bool expand_tilde(char *path, int path_maxlen) {
    ///  Replace Leading Tilde With Home Directory
    ///  Converts ~ to "$HOME"
    ///  @param path - path to expand
    ///  @param path_maxlen - maximum length of path
    ///  @returns true if successful
    if (path == NULL || *path == '\0' || path_maxlen == 0)
        return false;
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
bool trim_path(char *dir) {
    /// Trims trailing spaces and slashes from directory path in place.
    ///  @param dir - directory path to trim
    ///  @returns true if successful
    if (!dir)
        return false;
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
    /// trims the file extension from "filename" and copies the result to "buf"
    /// The user is responsible for ensuring that "buf" has enough space to
    /// receive the result.
    ///  @param buf - buffer to receive result
    ///  @param filename - filename to trim
    ///  @returns true if successful
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
    ///  Returns the base name of a file specification.
    ///  The user is responsible for ensuring that "buf" has enough space to
    ///  receive the result.
    ///  @param buf - buffer to receive result
    ///  @param path - file specification
    ///  @returns true if successful
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
    ///  Returns the directory name of a file specification.
    ///  The user is responsible for ensuring that "buf" has enough space to
    ///  receive the result.
    ///  @param buf - buffer to receive result
    ///  @param path - file specification
    ///  @returns true if successful
    ///  @note "buf" must be large enough to receive the result
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
bool verify_dir(char *spec, int imode) {
    ///  Verifies that the directory specified by "spec" exists and is
    ///  accessible with the permissions specified by "imode".
    ///  @param spec - directory specification
    ///  @param imode - access mode
    ///         F_OK - existence
    ///         R_OK - read
    ///         W_OK - Write
    ///         X_OK - Execute
    ///         S_WCOK - Write or Create
    ///         S_QUIET - Suppress Error Messages
    ///  @note S_WCOK and S_QUIET are stripped before calling faccessat
    ///  @returns true if successful
    if (spec == NULL || *spec == '\0')
        return false;
    int mode = imode & ~(S_WCOK | S_QUIET);
    expand_tilde(spec, MAXLEN);
    struct stat sb;
    errno = 0;
    src_line = 0;

    if (faccessat(AT_FDCWD, spec, mode, AT_EACCESS) != 0) {
        src_line = __LINE__ - 2;
        src_name = __FILE__;
        strnz__cpy(fn, "faccessat", MAXLEN - 1);
    } else {
        if (fstatat(AT_FDCWD, spec, &sb, 0) != 0) {
            src_line = __LINE__ - 1;
            src_name = __FILE__;
            strnz__cpy(fn, "fstatat", MAXLEN - 1);
        } else {
            if ((sb.st_mode & S_IFMT) != S_IFDIR) {
                src_line = __LINE__ - 1;
                src_name = __FILE__;
                strnz__cpy(fn, "verify_file", MAXLEN - 1);
                strnz__cpy(em2, "Not a regular file.", MAXLEN - 1);
            }
        }
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
bool verify_file(char *in_spec, int imode) {
    ///  Verifies that the file specified by "in_spec" exists and is
    ///  accessible with the permissions specified by "imode".
    ///  @param in_spec - directory specification
    ///  @param imode - access mode
    ///         F_OK - existence
    ///         R_OK - read
    ///         W_OK - Write
    ///         X_OK - Execute
    ///         S_WCOK - Write or Create
    ///         S_QUIET - Suppress Error Messages
    ///  @note S_WCOK and S_QUIET are stripped before calling faccessat
    ///  @returns true if successful
    if (in_spec == NULL || *in_spec == '\0')
        return false;
    struct stat sb;
    char spec[MAXLEN];
    strnz__cpy(spec, in_spec, MAXLEN - 1);
    int mode = imode & ~(S_WCOK | S_QUIET);
    errno = 0;
    src_line = 0;
    canonicalize_file_spec(spec);
    expand_tilde(spec, MAXLEN);
    if ((faccessat(AT_FDCWD, spec, mode, AT_EACCESS)) != 0) {
        src_line = __LINE__ - 1;
        src_name = __FILE__;
        strnz__cpy(fn, "faccessat", MAXLEN - 1);
    } else {
        if ((fstatat(AT_FDCWD, spec, &sb, 0)) != 0) {
            src_line = __LINE__ - 1;
            src_name = __FILE__;
            strnz__cpy(fn, "fstatat", MAXLEN - 1);
        } else {
            if ((sb.st_mode & S_IFMT) != S_IFREG) {
                src_line = __LINE__ - 1;
                src_name = __FILE__;
                strnz__cpy(fn, "verify_file", MAXLEN - 1);
                strnz__cpy(em2, "Not a regular file.", MAXLEN - 1);
            }
        }
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
bool locate_file_in_path(char *file_spec, char *file_name) {
    ///  Locates a file in the system PATH.
    ///  @param file_spec - buffer to receive located file specification
    ///  @param file_name - name of file to locate
    ///  @note file_spec must be large enough to receive the result
    ///  @returns true if file is located
    if (file_name == NULL || *file_name == '\0' || file_spec == NULL)
        return false;
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
bool list_files(char *dir, char *regexp, bool f_recurse) {
    ///  Lists files in a directory matching a regular expression, optionally
    ///  recursing into subdirectories.
    ///  @param dir - directory to list files from
    ///  @param regexp - regular expression to match files
    ///  @param f_recurse - true to recurse into subdirectories
    int flags = RECURSE;
    int depth = 3;
    if (dir == NULL || *dir == '\0' || regexp == NULL || *regexp == '\0')
        return false;
    normalize_file_spec(dir);
    if (f_recurse) {
        lf_find_files(dir, regexp, flags);
        lf_find_dirs(dir, regexp, depth, flags);
    } else {
        lf_find_files(dir, regexp, flags);
    }
    return true;
}
bool lf_find_dirs(char *dir, char *re, int depth, int flags) {
    /// Recursively find directories and call lf_find_files on each
    /// directory found
    /// @param dir   starting directory
    /// @param re    regular expression to match files
    /// @param depth current recursion depth
    /// @param flags search flags
    /// return      true if successful, false otherwise
    struct stat sb;
    struct dirent *dir_st;
    DIR *dirp;
    char dir_s[MAXLEN];
    char file_spec[MAXLEN];

    if (depth == MAX_DEPTH)
        return true;
    depth++;
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
                //  strnz__cpy(tmp_str, "can't stat ", MAXLEN - 1);
                //  strnz__cat(tmp_str, file_spec, MAXLEN - strlen(tmp_str));
                //  perror(tmp_str);
                return false;
            }
            if ((sb.st_mode & S_IFMT) == S_IFDIR) {
                strnz__cpy(dir_s, file_spec, MAXLEN - 1);
                lf_find_files(dir_s, re, flags);
                lf_find_dirs(dir_s, re, depth, flags);
            }
        }
        dir_st = readdir(dirp);
    }
    closedir(dirp);
    depth--;
    return true;
}
bool lf_find_files(char *dir, char *re, int flags) {
    /// Find files in a directory matching a regular expression
    /// @param dir   directory to search
    /// @param re    regular expression to match files
    /// @param flags search flags
    /// return      true if successful, false otherwise
    struct stat sb;
    struct dirent *dir_st;
    DIR *dirp;
    int REG_FLAGS = REG_EXTENDED;
    int reti;
    regmatch_t pmatch[1];
    regex_t compiled_re;
    char file_spec[MAXLEN];
    char *file_spec_p;

    if (flags & ICASE)
        REG_FLAGS |= REG_ICASE;
    reti = regcomp(&compiled_re, re, REG_FLAGS);
    if (reti) {
        printf("lf: \'%s\' Invalid pattern\n", re);
        printf("for example: \'.*\\.c$\'\n\n");
        return false;
    }
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
                // strnz__cpy(tmp_str, "can't stat ", MAXLEN - 1);
                // strnz__cat(tmp_str, file_spec, MAXLEN - 1);
                // perror(tmp_str);
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
                strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
                strnz__cat(tmp_str, msgbuf, MAXLEN - 1);
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
size_t canonicalize_file_spec(char *spec) {
    ///  canonicalize_file_spec(char *spec)
    ///  Removes quotes and trims at first space
    ///  @param spec - file specification to canonicalize
    ///  @returns length of resulting string
    if (spec == NULL || *spec == '\0')
        return 0;
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
char *rep_substring(const char *org_s, const char *tgt_s, const char *rep_s) {
    /// Replace all occurrences of "tgt_s" in "org_s" with "rep_s"
    /// @param org_s - original string
    /// @param tgt_s - target substring to replace
    /// @param rep_s - replacement substring
    /// @returns pointer to newly allocated string with replacements
    /// @note If any parameter is NULL, the function returns NULL.
    /// @note The function allocates memory for the new string, which must be
    /// freed by the caller to avoid memory leaks.
    /// @note The function replaces all occurrences of "tgt_s" in "org_s" with
    /// "rep_s". If "tgt_s" is not found in "org_s", the function returns a copy
    /// of "org_s".
    /// @note The function does not modify the original string "org_s".
    /// @note The function assumes that "tgt_s" and "rep_s" are null-terminated
    /// strings. If they are not, the behavior is undefined.
    /// @note The function does not perform any bounds checking on the input
    /// strings, so it is the caller's responsibility to ensure that they are
    /// valid and that the resulting string does not exceed available memory.
    /// @note The function uses the standard library functions strlen, strstr,
    /// malloc, and strcpy, which may have their own limitations and behaviors
    /// that the caller should be aware of.
    /// @note The function returns a pointer to a newly allocated string
    /// containing the result of the replacement. The caller is responsible for
    /// freeing this memory when it is no longer needed to avoid memory leaks.
    /// @note The function does not handle overlapping occurrences of "tgt_s" in
    /// "org_s". If "tgt_s" can overlap with itself in "org_s", the behavior may
    /// be unexpected. The caller should ensure that "tgt_s" does not contain
    /// overlapping patterns to avoid this issue.
    /// @note The function does not handle cases where "tgt_s" is a substring of
    /// "rep_s", which could lead to unintended consequences if "tgt_s" appears
    /// in "rep_s". The caller should ensure that "tgt_s" and "rep_s" are
    /// distinct to avoid this issue.
    /// @example:
    /// char *result = rep_substring("Hello, World!", "World", "Universe");
    /// printf("%s\n", result); // Output: "Hello, Universe!"
    ///
    if (org_s == NULL || tgt_s == NULL || rep_s == NULL)
        return NULL;
    if (*org_s == '\0' || *tgt_s == '\0')
        return NULL;
    if (*rep_s == '\0')
        return strdup(org_s); // Return a copy of the empty string
    if (strstr(org_s, tgt_s) == NULL)
        return strdup(org_s); // Return a copy of the original string if target
                              // substring is not found
    if (strstr(rep_s, tgt_s) != NULL)
        return NULL; // Avoid unintended consequences if target substring
                     // appears in replacement string
    if (tgt_s == rep_s || tgt_s == org_s || rep_s == org_s)
        return strdup(org_s); // Return a copy of the original string if
    // (target and replacement strings) or (target and original string) or
    // (replacement and original strings) are the same
    if (strcmp(org_s, tgt_s) == 0)
        return strdup(rep_s); // Return a copy of the replacement string if
                              // original string is the same as target string
                              // This is a special case that allows for .
                              // replacing the entire original string
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
    if (!out_s) {
        return NULL;
    }
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
/// String Functions
/// @use:  These functions manage strings with dynamic memory allocation,
/// encapsulated in a String struct.
String to_string(const char *s) {
    /// Convert C string to String struct
    /// @param: s C string
    /// @return String struct containing dynamically allocated copy of input
    /// string
    /// @note: the caller is responsible for freeing the allocated memory.
    /// @example:
    /// String str = to_string("Hello, World!");
    /// str = free_string(str);
    /// typedef struct {
    ///     char *s;   // pointer to string
    ///     size_t l;  // length of string including null terminator
    /// } String;
    if (s == NULL) {
        String str;
        str.l = 0;
        str.s = NULL;
        return str;
    }
    String str;
    str.l = strlen(s) + 1;
    str.s = (char *)malloc(str.l);
    strcpy(str.s, s);
    return str;
}
String mk_string(size_t l) {
    /// Create a String struct with a dynamically allocated string of length l
    /// @param: l length of string to create (including null terminator)
    /// @note: The returned String struct contains a dynamically allocated
    /// string of the specified length, initialized to an empty string.
    /// @see: free_string
    /// @return: String struct
    /// @note: the caller is responsible for freeing the allocated memory.
    /// @example:
    /// String str = mk_string(20);
    /// str = free_string(str);
    if (l == 0) {
        String str;
        str.l = 0;
        str.s = NULL;
        return str;
    }
    String str;
    str.l = l + 1;
    str.s = (char *)malloc(str.l);
    str.s[0] = '\0';
    return str;
}
bool free_string(String str) {
    /// Free the dynamically allocated string in a String struct
    /// @param: String str struct to free
    /// @note: Frees the dynamically allocated string and sets length to 0.
    /// @return: String struct with NULL pointer and length 0
    /// @note: the caller is responsible for freeing the allocated memory.
    if (str.s == NULL)
        return false;
    free(str.s);
    str.l = 0;
    str.s = NULL;
    return true;
}
size_t string_cpy(String *dest, const String *src) {
    /// Copy src String to dest String, allocating additional memory
    /// for dest String if necessary
    /// @param dest - destination String struct
    /// @param src - source String struct
    /// @returns length of dest String
    /// @note: the caller is responsible for freeing the allocated memory.
    if (dest == NULL || src == NULL || src->s == NULL)
        return 0;
    if (dest->l < src->l) {
        dest->s = (char *)realloc(dest->s, src->l);
        dest->l = src->l;
    }
    strcpy(dest->s, src->s);
    return src->l;
}
size_t string_cat(String *dest, const String *src) {
    /// concatenates src String to dest String, allocating additional memory
    /// for dest String if necessary
    /// @param dest - destination String struct
    /// @param src - source String struct
    /// @returns new length of dest String after concatenation
    /// @note: the caller is responsible for freeing the allocated memory.
    if (dest == NULL || src == NULL || src->s == NULL)
        return 0;
    size_t new_len = strlen(dest->s) + strlen(src->s) + 1;
    if (dest->l < new_len) {
        dest->s = (char *)realloc(dest->s, new_len);
        dest->l = new_len;
    }
    strcat(dest->s, src->s);
    return new_len;
}
size_t string_ncat(String *dest, const String *src, size_t n) {
    /// concatenates up to n characters from src String to dest String,
    /// allocating additional memory for dest String if necessary
    /// @param dest - destination String struct
    /// @param src - source String struct
    /// @param n - maximum number of characters to concatenate
    /// @returns new length of dest String after concatenation
    /// @note: the caller is responsible for freeing the allocated memory.
    if (dest == NULL || src == NULL || src->s == NULL)
        return 0;
    size_t dest_len = strlen(dest->s);
    size_t src_len = strlen(src->s);
    size_t cat_len = (n < src_len) ? n : src_len;
    size_t new_len = dest_len + cat_len + 1;
    if (dest->l < new_len) {
        dest->s = (char *)realloc(dest->s, new_len);
        dest->l = new_len;
    }
    strncat(dest->s, src->s, cat_len);
    return new_len;
}
size_t string_ncpy(String *dest, const String *src, size_t n) {
    /// copies up to n characters from src String to dest String,
    /// allocating additional memory for dest String if necessary
    /// @param dest - destination String struct
    /// @param src - source String struct
    /// @param n - maximum number of characters to copy
    /// @note: the caller is responsible for freeing the allocated memory.
    if (dest == NULL || src == NULL || src->s == NULL)
        return 0;
    size_t src_len = strlen(src->s);
    size_t cpy_len = (n < src_len) ? n : src_len;
    size_t new_len = cpy_len + 1;
    if (dest->l < new_len) {
        dest->s = (char *)realloc(dest->s, new_len);
        dest->l = new_len;
    }
    strncpy(dest->s, src->s, cpy_len);
    dest->s[cpy_len] = '\0';
    return new_len;
}
