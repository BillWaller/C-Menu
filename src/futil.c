/** @file futil.c
    @brief General utility functions
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09 */

/** @defgroup utility_functions Utility functions
    @brief string manipulation, file handling, and error reporting.
    @details These functions provide common operations such as trimming strings,
   converting case, safely copying and concatenating strings, verifying file and
   directory access, and locating files in the system PATH. They are designed to
   be robust and handle edge cases gracefully, making them useful for a wide
   range of applications.
 */

#include <cm.h>
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

#define LF_LNK 1
#define LF_DIR 2
#define LF_REG 4

char earg_str[MAXLEN];
int eargc;
char *eargv[MAXARGS];

bool lf_find(const char *, const char *, const char *, int, int);
bool lf_process(const char *, regex_t *, regex_t *, int, int, int);
size_t strip_ansi(char *, char *);
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
size_t strnz(char *, size_t);
size_t strnlf(char *, size_t);
bool str_subc(char *, char *, char, char *, int);
char *rep_substring(const char *, const char *, const char *);
bool normalize_file_spec(char *);
bool file_spec_path(char *, char *);
bool file_spec_name(char *, char *);
bool verify_file(char *, int);
bool verify_dir(char *, int);
bool locate_file_in_path(char *, char *);
size_t canonicalize_file_spec(char *);
bool is_directory(const char *);
bool is_valid_regex(const char *);
size_t ssnprintf(char *, size_t, const char *, ...);
size_t strnz__cpy(char *, const char *, size_t);
size_t strnz__cat(char *, const char *, size_t);
size_t string_cpy(String *, const String *);
size_t string_cat(String *, const String *);
size_t string_ncat(String *, const String *, size_t);
size_t string_ncpy(String *, const String *, size_t);
String to_string(const char *);
String mk_string(size_t);
String free_string(String);

/** Global variables for error reporting */
char errmsg[MAXLEN];
typedef struct {
    char fn[MAXLEN];
    char em0[MAXLEN];
    char em1[MAXLEN];
    char em2[MAXLEN];
    char em3[MAXLEN];
} error_info_t;
typedef struct {
    char *src_name;
    int src_line;
} error_source_t;
error_info_t error_info;
error_source_t error_source;
int wait_timer;
/**  @brief Trims trailing spaces from string s in place.
     @param s - string to trim
     @returns length of trimmed string */
size_t rtrim(char *s) {
    if (s == nullptr || *s == '\0')
        return 0;
    char *p = s;
    char *d = s;
    while (*p != '\0')
        *d++ = *p++;
    while (*(d - 1) == ' ' && d > s)
        d--;
    *d = '\0';
    return (size_t)(d - s);
}
/** @brief Trims leading and trailing spaces from string s in place.
    @ingroup utility_functions
    @param s - string to trim
    @returns length of trimmed string */
size_t trim(char *s) {
    if (s == nullptr || *s == '\0')
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
    return (size_t)(d - s);
}
/** @brief ssnprintf was designed to be a safer alternative to snprintf.
    @ingroup utility_functions
    @details It ensures that the buffer is not overflowed by taking the buffer
   size as a parameter and using vsnprintf internally. It also returns the
   number of characters that would have been written if enough space had been
   available, allowing the caller to detect truncation. This function is
   particularly useful in situations where the formatted string may exceed the
   buffer size, as it prevents buffer overflows and provides a way to handle
   such cases gracefully.
    @param buf - buffer to receive formatted string
    @param buf_size - size of buffer
    @param format - printf-style format string
    @param ... - arguments
    @returns number of characters that would have been written if enough space
   had been available */
size_t ssnprintf(char *buf, size_t buf_size, const char *format, ...) {
    size_t n;
    va_list args;

    va_start(args, format);
    n = vsnprintf(buf, buf_size, format, args);
    va_end(args);

    return n;
}
/** @brief Converts a string into an array of argument strings.
    @ingroup utility_functions
    @note Handles quoted strings and escaped quotes, preserving text inside
   quotes as individual arguments. It has been in service for many years without
   problems.
    @param argv - array of pointers to arguments
    @param arg_str - string containing arguments
    @param max_args - maximum number of arguments to parse
    @returns argc, a count of allocated vectors in argv
    @note the caller is responsible for deallocating the strings in argv */
int str_to_args(char **argv, char *arg_str, int max_args) {
    if (arg_str == nullptr || *arg_str == '\0')
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
    argv[argc] = nullptr;
    return argc;
}
/** @brief Deallocates memory allocated for argument strings in argv.
    @ingroup utility_functions
    @param argc - count of allocated vectors in argv
    @param argv - array of pointers to arguments
    @note the caller must ensure that argc accurately reflects the number of
   allocated strings in argv, and that argv is not null. After calling this
   function, the pointers in argv will be set to nullptr to prevent dangling
   pointers. */
void destroy_argv(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            free(argv[i]);
            argv[i] = nullptr;
        }
    }
}
/** @brief Converts a string to lowercase.
    @ingroup utility_functions
    @param s - string to convert
    @returns true if successful, false if s is nullptr or empty */
bool str_to_lower(char *s) {
    if (s == nullptr || *s == '\0')
        return false;
    while (*s != '\0') {
        if (*s >= 'A' && *s <= 'Z')
            *s = *s + 'a' - 'A';
        s++;
    }
    return true;
}
/** @brief Converts a string to uppercase.
    @ingroup utility_functions
    @param s - string to convert
    @returns true if successful, false if s is nullptr or empty */
bool str_to_upper(char *s) {
    if (s == nullptr || *s == '\0')
        return false;
    while (*s != '\0') {
        if (*s >= 'a' && *s <= 'z')
            *s = *s + 'A' - 'a';
        s++;
    }
    return true;
}
/** @brief safer alternative to strncpy
    @ingroup utility_functions
    @details copies string s to d, ensuring that the total length of d does not
   exceed max_len, and that the resulting string is null-terminated. It also
   treats newline and carriage return characters as string terminators,
   preventing them from being included in the result. This is particularly
   useful when copying user input or file data, where embedded newlines could
   cause issues.
    @param d - destination string
    @param s - source string
    @param max_len - maximum length to copy
    @returns length of resulting string */
size_t strnz__cpy(char *d, const char *s, size_t max_len) {
    char *e;
    size_t len = 0;
    if (s == nullptr || d == nullptr || max_len == 0) {
        if (d != nullptr && max_len > 0)
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
/** @brief safer alternative to strncat
    @ingroup utility_functions
    @note It appends string s to d, ensuring that the total length of d does not
   exceed max_len, and that the resulting string is null-terminated. It also
   treats newline and carriage return characters as string terminators,
   preventing them from being included in the result. This is particularly
   useful when concatenating user input or file data, where embedded newlines
   could cause issues.
    @param d - destination string
    @param s - source string
    @param max_len - maximum length to copy
    @returns length of resulting string
 */
size_t strnz__cat(char *d, const char *s, size_t max_len) {
    char *e;
    size_t len = 0;
    if (s == nullptr || d == nullptr || max_len == 0) {
        if (d != nullptr && max_len > 0)
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
/** @brief Terminates string at new line or carriage return
    @ingroup utility_functions
    @param s string to terminate
 */
size_t strz(char *s) {
    size_t l = 0;
    if (s == nullptr || *s == '\0')
        return 0;
    while (*s != '\0' && *s != '\n' && *s != '\r') {
        s++;
        l++;
    }
    *s = '\0';
    return l;
}
/**  @brief terminates string at New Line, Carriage Return, or max_len
    @ingroup utility_functions
     @note The use case is to ensure that strings read from files or user input
   do not contain embedded newlines or carriage returns.
     @param s string to terminate
     @param max_len - maximum length to scan
     @returns length of resulting string */
size_t strnz(char *s, size_t max_len) {
    char *e;
    size_t len = 0;
    if (s == nullptr || *s == '\0' || max_len == 0)
        return 0;
    e = s + max_len;
    while (*s != '\0' && *s != '\n' && *s != '\r' && s < e) {
        s++;
        len++;
    }
    *s = '\0';
    return (len);
}
/** @brief terminates string with line feed
    @ingroup utility_functions
    @param s string to terminate
    @param max_len maximum length to scan
    @returns length of resulting string */
size_t strnlf(char *s, size_t max_len) {
    char *e;
    size_t len = 0;
    if (s == nullptr || *s == '\0' || max_len == 0)
        return 0;
    e = s + max_len;
    while (*s != '\0' && *s != '\n' && *s != '\r' && s < e) {
        s++;
        len++;
    }
    *s++ = '\n';
    len++;
    *s = '\0';
    return (len);
}
/**  @brief Allocates memory for and duplicates string s up to length l or until
   line feed or carriage return
    @ingroup utility_functions
     @param s - string to duplicate
     @param l - maximum length to copy
     @returns pointer to allocated memory */
char *strnz_dup(char *s, size_t l) {
    char *p, *rs, *e;
    size_t m;
    if (s == nullptr || *s == '\0' || l == 0)
        return nullptr;
    for (p = s, m = 1; *p != '\0'; p++, m++)
        ;
    rs = p = (char *)malloc(m);
    if (rs != nullptr) {
        e = rs + l;
        while (*s != '\0' && *s != '\n' && *s != '\r' && p < e)
            *p++ = *s++;
        *p = '\0';
    }
    return rs;
}
/** @brief Replaces "ReplaceChr" in "s" with "Withstr" in "d" won't copy more
   than "l" bytes to "d" Replaces all occurrences of a character in a string
   with another string, copying the result to a destination buffer.
    @ingroup utility_functions
    @param d - destination string
    @param s - source string
    @param ReplaceChr - character to replace
    @param Withstr - string to insert
    @param l - maximum length to copy
    @returns true if successful, false if any parameter is invalid
    @details This function ensures that the total length of the resulting string
   does not exceed the specified limit, and that the result is null-terminated.
   This function is useful for simple string substitutions where you want to
   replace a single character with a longer string, such as replacing spaces
   with underscores or tabs with spaces.
    @note The caller must ensure that "d" has enough space to receive the
   result, and that "l" is sufficient to hold the result. This function does not
   perform any bounds checking on "d" or "Withstr", so it is the caller's
   responsibility to ensure that they are valid and that "l" is appropriate for
   the operation. */
bool str_subc(char *d, char *s, char ReplaceChr, char *Withstr, int l) {
    char *e;
    if (s == nullptr || d == nullptr || Withstr == nullptr || l == 0) {
        if (d != nullptr && l > 0)
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
/**  @brief Fills string s with character c n
    @ingroup utility_functions
     @param s - string to fill
     @param c - character to fill with
     @param n - number of characters to fill
     @returns true if successful, false if s is nullptr or n is non-positive */
bool strnfill(char *s, char c, int n) {
    if (s == nullptr || n <= 0)
        return false;
    char *e;
    e = s + n;
    while (s < e)
        *s++ = c;
    *s = '\0';
    return true;
}
/**  @brief removes leading and trailing double quotes if present
    @ingroup utility_functions
     @param s - string to strip quotes from
     @returns true if successful, false if s is nullptr or empty */
bool strip_quotes(char *s) {
    if (s == nullptr)
        return false;
    int l = strlen(s);
    if (l > 1 && s[l - 1] == '\"') {
        memmove(s, s + 1, l - 2);
        s[l - 2] = '\0';
    }
    return true;
}
/** @brief removes leading and trailing double quotes if present
    @ingroup utility_functions
    @param s - string to strip quotes from
    @returns true if quotes were removed
    @note Same as STRIP_QUOTES but returns true if quotes were removed */
bool stripz_quotes(char *s) {
    if (s == nullptr || strlen(s) < 2)
        return false;
    int l = strlen(s);
    if (l > 1 && s[0] == '\"' && s[l - 1] == '\"') {
        memmove(s, s + 1, l - 2);
        s[l - 2] = '\0';
        return true;
    }
    return false;
}
/** @brief Replaces all occurrences of old_chr in s with new_chr in place.
    @ingroup utility_functions
    @param s - string to modify
    @param old_chr - character to replace
    @param new_chr - character to insert
    @returns true if successful or false if string s is null */
bool chrep(char *s, char old_chr, char new_chr) {
    if (s == nullptr)
        return false;
    while (*s != '\0') {
        if (*s == old_chr)
            *s = new_chr;
        s++;
    }
    return true;
}
/** @brief a safer alternative to atoi() for converting ASCII strings to
   integers.
    @ingroup utility_functions
    @param s is the input string
    @param a_toi_error is a pointer to a boolean that will be set to true if an
   error occurs during conversion, or false if the conversion is successful.
    @note accepts positive integers only.
    @note sets a_toi_error to (-1) on error
    @returns converted integer value, or -1 if an error occurs */
int a_toi(char *s, bool *a_toi_error) {
    int rc = -1;
    *a_toi_error = false;
    errno = 0;
    if (s && *s != 0)
        rc = (int)strtol(s, nullptr, 10);
    if (rc < 0 || errno) {
        rc = -1;
        *a_toi_error = true;
    }
    return rc;
}
/** @brief Strips ANSI SGR escape sequences (ending in 'm') from string s to d
    @ingroup utility_functions
    @param d Destination string
    @param s Source string
    @returns Length of stripped string
    @code
        char dest[1024];
        char src[] = "\033[31mThis is red text\033[0m
        size_t len = strip_ansi(dest, src);
        Result: dest = "This is red text", len = 17
    @example stripansi.c
    @endcode
    @note Only handles SGR sequences ending in 'm' or 'K'
    @note Skips non-ASCII characters
    @note The caller must ensure that d has enough space to hold the
    stripped string
    @note This function does not allocate memory; it assumes d is
    pre-allocated
    @note This function processes the entire string until the null
    terminator
    @note This function does not modify the source string s */
size_t strip_ansi(char *d, char *s) {
    size_t l = 0;
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
/**  @brief replace backslashes with forward lashes
    @ingroup utility_functions
     @param fs - file specification to normalize
     @returns true if successful, false if fs is nullptr or empty */
bool normalize_file_spec(char *fs) {
    if (fs == nullptr || *fs == '\0')
        return false;
    while (*fs != '\0') {
        if (*fs == '\\')
            *fs = '/';
        fs++;
    }
    return true;
}
/** @brief extracts the path component of a file specification
    @ingroup utility_functions
    @param fp - path component to return
    @param fs - full file specification
    @returns true if successful
    @note The caller is responsible for ensuring that "fp" has enough space to
    receive the result. */
bool file_spec_path(char *fp, char *fs) {
    if (fs == nullptr || *fs == '\0' || fp == nullptr) {
        if (fp != nullptr)
            *fp = '\0';
        return false;
    }
    char *d, *l, *s;
    s = fp;
    d = fs;
    l = nullptr;
    while (*s != '\0') {
        if (*s == '/')
            l = d;
        *d++ = *s++;
    }
    if (l == nullptr)
        *fp = '\0'; // no slash, so no path
    else
        *l = '\0';
    return true;
}
/**  @brief extracts the file name component of a file specification
    @ingroup utility_functions
     @param fn - name component to return
     @param fs - full file specification
     @note The caller is responsible for ensuring that "fn" has enough space to
     receive the result. */
bool file_spec_name(char *fn, char *fs) {
    if (fs == nullptr || *fs == '\0' || fn == nullptr) {
        if (fn != nullptr)
            *fn = '\0';
        return false;
    }
    char *d, *l, *s;
    l = nullptr;
    s = fs;
    while (*s != '\0') {
        if (*s == '/')
            l = s;
        s++;
    }
    if (l == nullptr)
        s = fs;
    else
        s = ++l;
    d = fn;
    while (*s != '\0')
        *d++ = *s++;
    *d = '\0';
    return true;
}
/**  @brief converts string to double
    @ingroup utility_functions
     @param s - string to convert
     @returns converted double value, or 0.0 if s is nullptr, empty, or invalid
     @deprecated If the string is invalid, this function returns 0.0, with no
   indication of error.
     @note The caller must ensure that the string is a valid representation
     of a double before calling this function. */
double str_to_double(char *s) {
    char *e;
    double d;

    if (!s || !*s)
        return false;
    d = strtod(s, &e);
    return d;
}
/**  @brief Converts String to boolean true or false
     @param s - string to convert
     @returns boolean true or false */
bool str_to_bool(const char *s) {
    if (s == nullptr || *s == '\0')
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
/**  @brief Replace Leading Tilde With Home Directory
    @ingroup utility_functions
     @param path - path to expand
     @param path_maxlen - maximum length of path
     @note The caller is responsible for ensuring that "path" has enough space
   to receive the result, and that "path_maxlen" is sufficient to hold the
   result. This function does not perform any bounds checking on "path", so it
   is the caller's responsibility to ensure that it is valid and that
   "path_maxlen" is appropriate for the operation.
     @returns true if successful
 */
bool expand_tilde(char *path, int path_maxlen) {
    if (path == nullptr || *path == '\0' || path_maxlen == 0)
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
/** @brief Trims trailing spaces and slashes from directory path in place.
    @ingroup utility_functions
    @param dir - directory path to trim
    @returns true if successful */
bool trim_path(char *dir) {
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
/** @brief trims the file extension from "filename" and copies the result to
   "buf"
    @ingroup utility_functions
    @param buf - buffer to receive result
    @param filename - filename to trim
    @note The caller is responsible for ensuring that "buf" has enough space to
   receive the result. */
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
/**  @brief Returns the base name of a file specification.
    @ingroup utility_functions
     @param buf - buffer to receive result
     @param path - file specification
     @returns true if successful
     @note The caller is responsible for ensuring that "buf" has enough space to
   receive the result.
*/
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
/**  @brief Returns the directory name of a file specification.
    @ingroup utility_functions
     @param buf - buffer to receive result
     @param path - file specification
     @returns true if successful
     @note The caller is responsible for ensuring that "buf" has enough space to
   receive the result. */
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
/**  @brief Verifies that the directory specified by "spec" exists and is
   accessible with the permissions specified by "imode".
    @ingroup utility_functions
     @param spec - directory specification
     @param imode - access mode
            F_OK - existence
            R_OK - read
            W_OK - Write
            X_OK - Execute
            S_WCOK - Write or Create
            S_QUIET - Suppress Error Messages
     @note S_WCOK and S_QUIET are stripped before calling faccessat
     @returns true if successful */
bool verify_dir(char *spec, int imode) {
    if (spec == nullptr || *spec == '\0')
        return false;
    expand_tilde(spec, MAXLEN);
    struct stat sb;
    errno = 0;
    src_line = 0;
    int mode = imode & ~(S_WCOK | S_QUIET);
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
        if (!(mode & S_QUIET)) {
            ssnprintf(em0, MAXLEN - 1, "%s failed in %s at line %d", fn,
                      src_name, src_line);
            strnz__cpy(em1, spec, MAXLEN - 1);
            strnz__cpy(em3, "Check the file", MAXLEN - 1);
            display_error(em0, em1, em2, em3);
        }
        return false;
    }
    return true;
}
/**  @brief Verifies that the file specified by "in_spec" exists and is
   accessible with the permissions specified by "imode".
    @ingroup utility_functions
     @param in_spec - directory specification
     @param imode - access mode
            F_OK - existence
            R_OK - read
            W_OK - Write
            X_OK - Execute
            S_WCOK - Write or Create
            S_QUIET - Suppress Error Messages
     @note S_WCOK and S_QUIET are stripped before calling faccessat
     @returns true if successful */
bool verify_file(char *in_spec, int imode) {
    if (in_spec == nullptr || *in_spec == '\0')
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
/**  @brief Locates a file in the system PATH.
    @ingroup utility_functions
     @param file_spec - buffer to receive located file specification
     @param file_name - name of file to locate
     @returns true if file is located
     @note file_spec must be large enough to receive the result */
bool locate_file_in_path(char *file_spec, char *file_name) {
    if (file_name == nullptr || *file_name == '\0' || file_spec == nullptr)
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
    if ((p = getenv("PATH")) == nullptr)
        return false;
    strnz__cpy(path, p, MAXLEN - 1);
    dir = strtok(path, ":");
    while (dir != nullptr) {
        strnz__cpy(file_spec, dir, MAXLEN - 1);
        strnz__cat(file_spec, "/", MAXLEN - 1);
        strnz__cat(file_spec, file_name, MAXLEN - 1);
        if (access(file_spec, F_OK) == 0) {
            return true;
        }
        dir = strtok(nullptr, ":");
    }
    return false;
}
/** @brief Find files in a directory matching a regular expression
    @ingroup utility_functions
    @param base_path  directory to search
    @param ere        regular expression match to exclude
    @param re         regular expression match to include
    @param max_depth  depth of directories to scan
    @param flags      search flags
    @see lf_process() for flag definitions
    return      true if successful, false otherwise */
bool lf_find(const char *base_path, const char *re, const char *ere,
             int max_depth, int flags) {
    int reti;
    regex_t compiled_re;
    regex_t compiled_ere;
    int REG_FLAGS = REG_EXTENDED;
    if (flags & LF_ICASE)
        REG_FLAGS |= REG_ICASE;
    if (flags & LF_REGEX) {
        reti = regcomp(&compiled_re, re, REG_FLAGS);
        if (reti) {
            ssnprintf(em0, MAXLEN - 1, "lf: \'%s\' Invalid pattern\n", re);
            ssnprintf(em1, MAXLEN - 1, "for example: \'.*\\.c$\'\n\n");
            regerror(reti, &compiled_re, em2, sizeof(em2));
            display_error(em0, em1, em2, nullptr);
            regfree(&compiled_re);
            return false;
        }
    }
    if (flags & LF_EXC_REGEX) {
        regcomp(&compiled_ere, "^$", REG_FLAGS);
        reti = regcomp(&compiled_ere, ere, REG_FLAGS);
        if (reti) {
            ssnprintf(em0, MAXLEN - 1, "lf: \'%s\' Invalid exclude pattern\n",
                      re);
            ssnprintf(em1, MAXLEN - 1, "for example: \'.*\\.c$\'\n\n");
            regerror(reti, &compiled_ere, em2, sizeof(em2));
            display_error(em0, em1, em2, nullptr);
            regfree(&compiled_ere);
            return false;
        }
    }
    reti =
        lf_process(base_path, &compiled_re, &compiled_ere, 0, max_depth, flags);
    if (flags &= LF_REGEX)
        regfree(&compiled_re);
    if (flags & LF_EXC_REGEX)
        regfree(&compiled_ere);
    if (reti)
        return false;
    return true;
}
/** @brief logic for lf_find()
    @ingroup utility_functions
    @param base_path   directory to search
    @param compiled_ere compiled regular expression to exclude
    @param compiled_re compiled regular expression to include
    @param depth recursion counter
    @param max_depth how deep to descend into the directory structure
    @param flags
    @return true if successful, false otherwise
    @verbatim

    LF_ALL = 1,       List all files including hidden files
    LF_ICASE = 2,     Ignore case in search
    LF_EXC_REGEX = 4, Exclude files matching regular expression
    LF_REGEX = 8,     Include files matching regular expression
    LF_BLK = 256,
    LF_CHR = 512,
    LF_DIR = 1024,
    LF_FIFO = 2048,
    LF_LNK = 4096,
    LF_REG = 8192,
    LF_SOCK = 16384,
    LF_UNKNOWN = 32768,
                      Include     Exclude
                     ----------  ----------
    LF_BLK        1  0 00000001  7 11111110 block device
    LF_CHR        2  1 00000010  6 11111101 character device
    LF_DIR        4  2 00000100  5 11111011 directory
    LF_FIFO       8  3 00001000  4 11110111 named pipe
    LF_LNK       16  4 00010000  3 11101111 link
    LF_REG       32  5 00100000  2 11011111 regular file
    LF_SOCK      64  6 01000000  1 10111111 socket
    LF_UNKNOWN  128  7 10000000  0 01111111 unknown

    bool s_blk;
    bool s_chr;
    bool s_dir;
    bool s_fifo;
    bool s_lnk;
    bool s_reg;
    bool s_sock;
    bool s_unknown;
    @endverbatim
*/
bool lf_process(const char *base_path, regex_t *compiled_re,
                regex_t *compiled_ere, int depth, int max_depth, int flags) {
    char tmp_str[MAXLEN];
    struct dirent *dir_st;
    DIR *dir;
    int REG_FLAGS = REG_EXTENDED;
    int reti;
    regmatch_t pmatch[1];
    char file_spec[1024];
    bool suppress;
    int f_include;
    int f_suppress;
    bool suppress_hidden = flags & LF_ALL ? false : true;
    char bname[MAXLEN];
    if ((dir = opendir(base_path)) == 0)
        return false;
    while ((dir_st = readdir(dir)) != nullptr) {
        strnz__cpy(bname, dir_st->d_name, MAXLEN - 1);
        if (bname[0] == '.' && bname[1] == '\0')
            continue;
        else if (bname[0] == '.' && bname[1] == '.' && bname[2] == '\0')
            continue;
        else if (bname[0] == '.' && suppress_hidden)
            continue;
        f_suppress = 0;
        f_include = (flags >> 8) & 0xff;
        if (f_include)
            f_suppress = f_include ^ 0xff;
        bool s_blk = f_suppress & FT_BLK ? 1 : 0;
        bool s_chr = f_suppress & FT_CHR ? 1 : 0;
        bool s_dir = f_suppress & FT_DIR ? 1 : 0;
        bool s_fifo = f_suppress & FT_FIFO ? 1 : 0;
        bool s_lnk = f_suppress & FT_LNK ? 1 : 0;
        bool s_reg = f_suppress & FT_REG ? 1 : 0;
        bool s_sock = f_suppress & FT_SOCK ? 1 : 0;
        bool s_unknown = f_suppress & FT_UNKNOWN ? 1 : 0;
        suppress = false;
        switch (dir_st->d_type) {
        case DT_BLK:
            if (s_blk)
                suppress = true;
            break;
        case DT_CHR:
            if (s_chr)
                suppress = true;
            break;
        case DT_DIR:
            if (s_dir)
                suppress = true;
            break;
        case DT_FIFO:
            if (s_fifo)
                suppress = true;
            break;
        case DT_LNK:
            if (s_lnk)
                suppress = true;
            break;
        case DT_REG:
            if (s_reg)
                suppress = true;
            break;
        case DT_SOCK:
            if (s_sock)
                suppress = true;
            break;
        case DT_UNKNOWN:
            if (s_unknown)
                suppress = true;
            break;
        default:
            break;
        }
        if (dir_st->d_name[0] == '.' && dir_st->d_name[1] == '/')
            strnz__cpy(file_spec, &dir_st->d_name[2], MAXLEN - 1);
        else
            strnz__cpy(file_spec, dir_st->d_name, MAXLEN - 1);
        ssnprintf(file_spec, sizeof(file_spec), "%s/%s", base_path, bname);
        if (bname[0] == '.' && suppress_hidden)
            suppress = true;
        if (!suppress && (flags & LF_REGEX)) {
            reti = regexec(compiled_re, file_spec, compiled_re->re_nsub + 1,
                           pmatch, REG_FLAGS);
            if (reti == REG_NOMATCH) {
                suppress = true;
            } else if (reti) {
                char msgbuf[100];
                regerror(reti, compiled_re, msgbuf, sizeof(msgbuf));
                strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
                strnz__cat(tmp_str, msgbuf, MAXLEN - 1);
                Perror(tmp_str);
                return false;
            }
        }
        if (!suppress && (flags & LF_EXC_REGEX)) {
            reti = regexec(compiled_ere, file_spec, compiled_re->re_nsub + 1,
                           pmatch, REG_FLAGS);
            if (reti == 0)
                suppress = true;
            else if (reti == REG_NOMATCH)
                suppress = false;
            else if (reti) {
                char msgbuf[100];
                regerror(reti, compiled_re, msgbuf, sizeof(msgbuf));
                strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
                strnz__cat(tmp_str, msgbuf, MAXLEN - 1);
                Perror(tmp_str);
                return false;
            }
        }
        if (!suppress) {
            if (file_spec[0] == '.' && file_spec[1] == '/')
                printf("%s\n", &file_spec[2]);
            else
                printf("%s\n", file_spec);
        }
        if (dir_st->d_type == DT_DIR && depth + 1 < max_depth) {
            depth++;
            lf_process(file_spec, compiled_re, compiled_ere, depth, max_depth,
                       flags);
            depth--;
        }
    }
    closedir(dir);
    return true;
}
/** @brief If directory doesn't exist, make it
    @ingroup utility_functions
    @param dir directory name
    @return true if directory now exists or false otherwise */
bool mk_dir(char *dir) {
    expand_tilde(dir, MAXLEN - 1);
    if (!verify_dir(dir, S_WCOK | S_QUIET)) {
        if (!mkdir(dir, 0755)) {
            /** Directory does not exist and unable to create */
            ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 2);
            strnz__cpy(em1, "mkdir ", MAXLEN - 1);
            strnz__cat(em1, dir, MAXLEN - 1);
            strnz__cat(em1, " failed", MAXLEN - 1);
            strerror_r(errno, em2, MAXLEN - 1);
            display_error(em0, em1, em2, nullptr);
            return false;
        }
        return true;
    }
    return true;
}
/**  @brief Removes quotes and trims at first space
    @ingroup utility_functions
     @param spec - file specification to canonicalize
     @returns length of resulting string */
size_t canonicalize_file_spec(char *spec) {
    if (spec == nullptr || *spec == '\0')
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
/** @brief Checks if the given path is a directory
    @ingroup utility_functions
    @param path - path to check
    @returns true if the path is a directory, false otherwise */
bool is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}
/** @brief Checks if the given regular expression pattern is valid
    @ingroup utility_functions
    @param pattern - regular expression pattern to check
    @returns true if the pattern is valid, false otherwise */
bool is_valid_regex(const char *pattern) {
    regex_t regex;
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    regfree(&regex);
    return (ret == 0);
}
/** @brief Replace all occurrences of "tgt_s" in "org_s" with "rep_s"
    @ingroup utility_functions
    @param org_s - original string
    @param tgt_s - target substring to replace
    @param rep_s - replacement substring
    @returns A pointer to the newly allocated string with replacements or a copy
   of the replacement string if original string is the same as target string
   This is a special case that allows for replacing the entire original string.
   If any parameter is nullptr, the function returns nullptr. If "tgt_s" is not
   found in "org_s", the function returns a copy of "org_s". If target substring
   is not found the function returns a copy of the original string.
    @note allocates memory for the return value, so the caller is responsible
   for freeing this memory when it is no longer needed to avoid memory leaks.
    @note Does not modify the original string "org_s".
    @note Assumes that "tgt_s" and "rep_s" are null-terminated strings. If they
   are not, the behavior is undefined.
    @note Does not perform any bounds checking on the input strings, so it is
   the caller's responsibility to ensure that they are valid and that the
   resulting string does not exceed available memory.
    @note Uses the standard library functions strlen, strstr, malloc, and
   strcpy, which may have their own limitations and behaviors that the caller
   should be aware of.
    @note Does not handle overlapping occurrences of "tgt_s" in "org_s". If
   "tgt_s" can overlap with itself in "org_s", the behavior may be unexpected.
   The caller should ensure that "tgt_s" does not contain overlapping patterns
   to avoid this issue.
    @note Does not handle cases where "tgt_s" is a substring of "rep_s", which
   could lead to unintended consequences if "tgt_s" appears in "rep_s". The
   caller should ensure that "tgt_s" and "rep_s" are distinct to avoid this
   issue. */
char *rep_substring(const char *org_s, const char *tgt_s, const char *rep_s) {
    if (org_s == nullptr || tgt_s == nullptr || rep_s == nullptr)
        return nullptr;
    if (*org_s == '\0' || *tgt_s == '\0' || *rep_s == '\0')
        return nullptr;
    if (strstr(org_s, tgt_s) == nullptr)
        return strdup(org_s);
    if (strstr(rep_s, tgt_s) != nullptr)
        return nullptr;
    if (tgt_s == rep_s || tgt_s == org_s || rep_s == org_s)
        return strdup(org_s);
    if (strcmp(org_s, tgt_s) == 0)
        return strdup(rep_s);
    char *out_s, *ip, *tmp;
    int tgt_l = strlen(tgt_s);
    int rep_l = strlen(rep_s);
    int head_l;
    int n = 0;
    ip = (char *)org_s;
    while ((tmp = strstr(ip, tgt_s)) != nullptr) {
        n++;
        ip = tmp + tgt_l;
    }
    out_s = malloc(strlen(org_s) + (rep_l - tgt_l) * n + 1);
    if (!out_s) {
        return nullptr;
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
/** @defgroup String_Objects String Objects
    @brief Simple String Object Library
 */
/** @brief String functions provide a simple string library to facilitate string
   manipulation in C, allowing developers to easily create, copy, concatenate,
   and free strings without having to manage memory manually.
    @ingroup String_Objects
   @note The library includes functions to convert C strings to String structs,
   create new String structs with specified lengths, copy and concatenate String
   structs, and free the memory used by String structs. By using this library,
   developers can avoid common pitfalls of C string handling, such as buffer
   overflows and memory leaks, while still benefiting from the performance
   advantages of C.
   @note Designed to be simple and easy to use, making it a great choice for
   developers who want to work with strings in C without having to worry about
   the complexities of manual memory management.
   @note The String struct is defined as follows:
   @code
     typedef struct {
         size_t l; // length of the string (including null terminator)
         char *s;  // pointer to the dynamically allocated string
     } String;
    @endcode
   @note All functions in this library that return a String struct allocate
   memory for the string using malloc or realloc. It is the caller's
   responsibility to free this memory using the free_string function when it is
   no longer needed to avoid memory leaks.
   @note The String functions in this library do not perform bounds checking on
   the input strings or the resulting strings. It is the caller's responsibility
   to ensure that all input strings are valid and that the resulting strings do
   not exceed available memory.
   @note The String functions in this library assume that all input strings are
   null-terminated. If any input string is not null-terminated, the behavior is
   undefined.
   @see snippets/strings_test1.c
 */
/** @brief Convert C string to String struct
    @ingroup String_Objects
    @param s C string
    @return String struct containing dynamically allocated copy of input string
    @note the caller is responsible for freeing the allocated memory.
    */
String to_string(const char *s) {
    if (s == nullptr) {
        String str;
        str.l = 0;
        str.s = nullptr;
        return str;
    }
    String str;
    str.l = strlen(s) + 1;
    str.s = (char *)malloc(str.l);
    strcpy(str.s, s);
    return str;
}
/** @brief Create a String struct with a dynamically allocated string @param l
   length of string to create including null terminator
   @returns String struct
   @note The returned String struct contains a dynamically allocated string of
   he specified length
   @sa free_string
   @note the caller is responsible for calling free_string to free the allocated
   memory. */
String mk_string(size_t l) {
    if (l == 0) {
        String str;
        str.l = 0;
        str.s = nullptr;
        return str;
    }
    String str;
    str.l = l + 1;
    str.s = (char *)malloc(str.l);
    str.s[0] = '\0';
    return str;
}
/** @brief Free the dynamically allocated String
    @ingroup String_Objects
    @param string to free
    @return string with nullptr pointer and length 0
    @note Frees the dynamically allocated string and sets length to 0.
 */
String free_string(String string) {
    if (string.s == nullptr)
        return string;
    free(string.s);
    string.l = 0;
    string.s = nullptr;
    return string;
}
/** @brief Copy src String to dest String, allocating additional memory for dest
   String if necessary
    @ingroup String_Objects
    @param dest - destination String struct
    @param src - source String struct
    @returns length of dest String
    @note the caller is responsible for freeing the allocated memory. */
size_t string_cpy(String *dest, const String *src) {
    if (dest == nullptr || src == nullptr || src->s == nullptr)
        return 0;
    if (dest->l < src->l) {
        dest->s = (char *)realloc(dest->s, src->l);
        dest->l = src->l;
    }
    strcpy(dest->s, src->s);
    return src->l;
}
/** @brief Concatenates src String to dest String, allocating additional memory
   for dest String if necessary
    @ingroup String_Objects
    @param dest - destination String struct
    @param src - source String struct
    @returns new length of dest String after concatenation
    @note the caller is responsible for freeing the allocated memory. */
size_t string_cat(String *dest, const String *src) {
    if (dest == nullptr || src == nullptr || src->s == nullptr)
        return 0;
    size_t new_len = strlen(dest->s) + strlen(src->s) + 1;
    if (dest->l < new_len) {
        dest->s = (char *)realloc(dest->s, new_len);
        dest->l = new_len;
    }
    strcat(dest->s, src->s);
    return new_len;
}
/** @brief Concatenates up to n characters from src String to dest String,
   allocating additional memory for dest String if necessary
    @ingroup String_Objects
    @param dest - destination String struct
    @param src - source String struct
    @param n - maximum number of characters to concatenate
    @returns new length of dest String after concatenation
    @note the caller is responsible for freeing the allocated memory. */
size_t string_ncat(String *dest, const String *src, size_t n) {
    if (dest == nullptr || src == nullptr || src->s == nullptr)
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
/** @brief copies up to n characters from src String to dest String, allocating
   additional memory for dest String if necessary
    @ingroup String_Objects
    @param dest - destination String struct
    @param src - source String struct
    @param n - maximum number of characters to copy
    @note the caller is responsible for freeing the allocated memory. */
size_t string_ncpy(String *dest, const String *src, size_t n) {
    if (dest == nullptr || src == nullptr || src->s == nullptr)
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
/** @defgroup testing_functions Testing Functions
    @brief Functions for Testing Only
 */

/** @brief Function to intentionally cause a segmentation fault for testing
   purposes
    @ingroup testing_functions
    @note This function is designed to intentionally cause a segmentation fault
   by dereferencing a null pointer. It is intended for testing purposes only and
   should not be used in production code. The caller should be aware that
   executing this function will crash the program. */
int segmentation_fault() {
    // int *p = NULL;
    // *p = 100;

    return 0;
}
