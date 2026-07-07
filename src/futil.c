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
#include <stdint.h>

#include <argp.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <ifaddrs.h>
#include <pwd.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#define LF_LNK 1
#define LF_DIR 2
#define LF_REG 4

void write_cmenu_log(char *);
void open_cmenu_log();
void left_justify(char *s);
void right_justify(char *, int);
bool is_valid_date(int yyyy, int mm, int dd);
bool is_valid_time(int hh, int mm, int ss);
void numeric(char *d, char *s);
int cmenu_log_fd;

char earg_str[MAXLEN];
int eargc;
char *eargv[MAXARGS];
typedef struct {
    char re[PATH_MAX];
    char ere[PATH_MAX];
    regex_t compiled_re;
    regex_t compiled_ere;
    long flags;
    time_t after;
    time_t before;
    uintmax_t user_id;
    intmax_t file_size_min;
    int max_depth;
    bool f_ignore_case;
    bool f_sort;
    bool f_reverse;
    bool f_hide;
    char *file_types_p;
    char *args[2];
    int argc;
    char exec[MAXLEN];
    int include_types;
    int suppress_types;
    bool include;
    bool blk;
    bool chr;
    bool dir;
    bool fifo;
    bool lnk;
    bool reg;
    bool sock;
    bool unknown;
} SearchFilters;

// bool init_find(const char *, SearchFilters *);
// int scan_files(const char *, SearchFilters *, int depth);
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
char *iso8601_time(char *, int, time_t *, bool);
bool parse_local_timestamp(const char *, time_t *);
char *format_local_timestamp(time_t, char *, size_t);
char *get_local_timestamp();
char *get_user_str(char *, size_t);
char *get_ip_addresses(char *, int);
char *fill_field(char *accept_s, char *display_s, char fill_char, int flen);
char stdio_names_str[MAXLEN];

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
int wait_timeout;
/** @brief Checks if the file specified by "fut" is newer than the file specified by "control".
    @ingroup utility_functions
    @param control - path to the control file
    @param fut - path to the file to compare against the control file
    @returns true if "fut" is newer than "control", false otherwise
    @details This function uses the stat system call to retrieve the modification times of both files. It compares the modification time of "fut" with that of "control" and returns true if "fut" has a more recent modification time. If either file cannot be accessed or if any error occurs during the stat calls, this function returns false. The caller must ensure that both file paths are valid and that the files exist before calling this function.
 */
bool is_newer(char *control, char *fut) {
    // is fut newer than control?
    struct stat control_st, fut_st;
    if (!stat(control, &control_st))
        if (!lstat(fut, &fut_st))
            if (fut_st.st_mtime > control_st.st_mtime)
                return true;
    return false;
}
/** @brief Retrieves the documentation string for a given key name from an argp
   options array.
    @ingroup utility_functions
    @param comment - buffer to receive the documentation string
    @param options - array of argp_option structures to search
    @param key_name - the long option name or short option character
*/
bool get_argp_doc_by_name(char *comment, const struct argp_option *options,
                          const char *key_name) {
    for (size_t i = 0; options[i].name != NULL || options[i].key != 0; i++) {
        // Skip purely cosmetic header/group entries in argp
        if (options[i].name == NULL && options[i].doc != NULL &&
            options[i].key == 0) {
            continue;
        }

        // 1. Check against the long option name (e.g., "verbose")
        if (options[i].name && strcmp(options[i].name, key_name) == 0) {
            strnz__cpy(comment, options[i].doc, MAXLEN - 1);
            return true;
        }

        // 2. Check against the short option key character (e.g., 'v')
        if (options[i].key > 0 && options[i].key < 127) {
            char short_str[2] = {(char)options[i].key, '\0'};
            if (strcmp(short_str, key_name) == 0) {
                strnz__cpy(comment, options[i].doc, MAXLEN - 1);
                return true;
            }
        }
    }
    return false; // Key not found in the argp structure
}
/** @brief Validates that a string consists of exactly len hexadecimal digits.
    @ingroup utility_functions
    @param str - input string to validate
    @param len - expected number of hexadecimal digits
    @returns true if str is a valid hex string of the specified length, false otherwise
    @details This function checks that the input string contains only hexadecimal characters (0-9, A-F, a-f) and that the total number of hex digits matches the specified length. If the input string is valid, it returns true; otherwise, it returns false. The caller must ensure that the input string is not null and has at least one character before calling this function.
 */
bool is_hex_str(char *str, int len) {
    char *s = str;
    char *e;
    if (s == NULL || *s == '\0')
        return false;
    e = (s + len + 1);
    while (s < e && *s != '\0') {
        if (!isxdigit(*s)) {
            return false;
        }
        s++;
    }
    if ((int)(s - str) != len)
        return false;
    return true;
}
/** @brief Validates that a string is a hex color code in the format "#RRGGBB".
    @ingroup utility_functions
    @param dst - buffer to receive validated hex color string
    @param str - input string to validate
    @returns true if str is a valid hex color code, false otherwise
    @details This function checks that the input string starts with a '#' character, followed by exactly six hexadecimal digits (0-9, A-F, a-f). If the input string is valid, it copies the hex color code into the provided destination buffer. The caller must ensure that dst has enough space to hold the resulting string (at least 8 characters including the null terminator). If the input string is invalid (e.g., does not start with '#', contains non-hex characters, or does not have exactly six hex digits), this function returns false and does not modify the destination buffer.
 */
bool unstr_hex_clr(char *dst, char *str) {
    char *s = str;
    char *e;
    char *d;
    if (s == NULL || *s == '\0')
        return false;
    if (*s != '#')
        return false;
    d = dst;
    *d++ = *s++;
    e = (s + 6);
    while (s < e && *s != '\0') {
        if (!isxdigit(*s)) {
            return false;
        }
        *d++ = *s++;
    }
    *d = '\0';
    if ((int)(s - str) != 7)
        return false;
    return true;
}

/** @brief Formats a struct tm as an ISO 8601 string.
    @ingroup utility_functions
    @param buf - buffer to receive formatted string
    @param n - size of buffer
    @param t - struct tm to format
    @param local - if true, include local time zone offset; if false, use 'Z' for UTC
    @returns pointer to buf
    @note The caller is responsible for ensuring that buf has enough space to hold the resulting string. The ISO 8601 format produced is "YYYY-MM-DDTHH:MM:SSZ" for UTC or "YYYY-MM-DDTHH:MM:SS±hhmm" for local time. This function uses strftime internally, so the actual format may vary based on the implementation of strftime and the locale settings.
 */
char *iso8601_time(char *buf, int n, time_t *t, bool local) {
    struct tm *tp = local ? localtime(t) : gmtime(t);
    if (local) {
        strftime(buf, n, "%Y-%m-%dT%H:%M:%S%z", tp);
    } else {
        strftime(buf, n, "%Y-%m-%dT%H:%M:%SZ", tp);
    }
    return buf;
}
/** @brief Parses an ISO 8601 timestamp string in local time and converts it to time_t.
    @ingroup utility_functions
    @param s - ISO 8601 timestamp string to parse (e.g., "2024-06-01T12:34:56")
    @param out - pointer to time_t variable to receive the result
    @returns true if parsing and conversion were successful, false otherwise
    @details This function expects the input string to be in the format "YYYY-MM-DDTHH:MM:SS" representing local time. It uses strptime to parse the string into a struct tm, then uses mktime to convert it to time_t. The caller must ensure that the input string is properly formatted and represents a valid date and time. If the input string is invalid or if any error occurs during parsing or conversion, this function returns false and does not modify the output variable.
 */
bool parse_local_timestamp(const char *s, time_t *out) {
    struct tm tmv;
    memset(&tmv, 0, sizeof tmv);
    tmv.tm_isdst = -1;

    if (strptime(s, "%Y-%m-%dT%H:%M:%S", &tmv) == NULL)
        return false;

    time_t t = mktime(&tmv);
    if (t == (time_t)-1)
        return false;

    *out = t;
    return true;
}
/** @brief Formats a time_t as an ISO 8601 string in local time.
    @ingroup utility_functions
    @param t - time to format
    @param buf - buffer to receive formatted string
    @param n - size of buffer
    @returns pointer to buf
    @note The caller is responsible for ensuring that buf has enough space to hold the resulting string. The ISO 8601 format produced is "YYYY-MM-DDTHH:MM:SS" followed by the local time zone offset (e.g., "+hhmm" or "-hhmm"). This function uses strftime internally, so the actual format may vary based on the implementation of strftime and the locale settings.
 */
char *format_local_timestamp(time_t t, char *buf, size_t n) {
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%S", &tmv);
    return buf;
}
/** @brief Returns the current local time as an ISO 8601 formatted string.
    @ingroup utility_functions
    @returns pointer to static buffer containing the current local timestamp in ISO 8601 format
    @note The returned string is stored in a static buffer, so it will be overwritten by subsequent calls to this function. The format of the returned string is "YYYY-MM-DDTHH:MM:SS" followed by the local time zone offset (e.g., "+hhmm" or "-hhmm"). This function uses the current system time and formats it using strftime internally, so the actual format may vary based on the implementation of strftime and the locale settings.
 */
char *get_local_timestamp() {
    static char buf[32];
    time_t t = time(NULL);
    format_local_timestamp(t, buf, sizeof buf);
    return buf;
}
/** @brief Retrieves the current user's name and UID, and formats it into a string.
    @ingroup utility_functions
    @param user_str - buffer to receive formatted string
    @param maxlen - size of buffer
    @returns pointer to user_str containing the formatted user information, or nullptr if an error occurs
    @details This function uses getuid to retrieve the current user's UID, then uses getpwuid to get the corresponding passwd structure, which contains the user's name. It formats the user's name and UID into the provided buffer in the format "User: username (uid)\n". The caller must ensure that user_str has enough space to hold the resulting string. If getpwuid fails (e.g., if the UID does not exist), this function returns nullptr and does not modify the buffer.
 */
char *get_user_str(char *user_str, size_t maxlen) {
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL)
        return nullptr;
    ssnprintf(user_str, maxlen - 1, "%s (%u)", pw->pw_name, (unsigned int)uid);
    return user_str;
}
/**  @brief Trims trailing spaces from string s in place.
     @param s - string to trim
     @returns length of trimmed string */
size_t rtrim(char *s) {
    if (s == nullptr || *s == '\0')
        return (size_t)0;
    char *p = s + strlen(s) - 1;
    while (p >= s && *p == ' ')
        p--;
    *(p + 1) = '\0';
    return (size_t)(p - s + 1);
}
/** @brief Retrieves the IP addresses of the local machine and formats them into a string.
    @ingroup utility_functions
    @param ip_str - buffer to receive formatted string of IP addresses
    @param maxlen - size of buffer
    @returns pointer to ip_str containing the formatted IP addresses, or nullptr if an error occurs
    @details This function uses getifaddrs to retrieve a linked list of network interfaces on the local machine. It iterates through the list and checks for interfaces with IPv4 addresses (AF_INET). For each valid interface, it converts the binary IP address to a human-readable string using inet_ntop and appends it to the provided buffer in the format "[interface-name-IP-address]". Multiple interfaces are separated by commas. The caller must ensure that ip_str has enough space to hold the resulting string. If getifaddrs fails, this function returns nullptr and does not modify the buffer.
 */
char *get_ip_addresses(char *ip_str, int maxlen) {
    char tmp_str[MAXLEN];
    struct ifaddrs *ifaddr, *ifa;
    char host[INET_ADDRSTRLEN];
    bool comma_before = false;
    // getifaddrs returns a linked list of network interface structures
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    ip_str[0] = '\0';
    // Walk through the linked list
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        // Check for IPv4 addresses
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)ifa->ifa_addr;

            // Convert binary IP to human-readable string
            inet_ntop(AF_INET, &pAddr->sin_addr, host, INET_ADDRSTRLEN);

            if (comma_before)
                snprintf(tmp_str, MAXLEN - 1, ",[%s-%s]", ifa->ifa_name, host);
            else
                snprintf(tmp_str, MAXLEN - 1, "[%s-%s]", ifa->ifa_name, host);
            strnz__cat(ip_str, tmp_str, maxlen - 1);
            comma_before = true;
        }
    }
    freeifaddrs(ifaddr); // Clean up the memory allocated by getifaddrs
    return ip_str;
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
    @param argv - array of pointers to arguments
    @param arg_str - string containing arguments
    @param max_args - maximum number of arguments to parse
    @returns argc, a count of allocated vectors in argv
    @details Handles quoted strings and escaped quotes, preserving text inside
   quotes as individual arguments. It has been in service for many years without
   problems.
    @note The caller is responsible for deallocating the strings in argv. */
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
int destroy_argv(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            free(argv[i]);
            argv[i] = nullptr;
        }
    }
    argc = 0;
    return argc;
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
    @param d - destination string
    @param s - source string
    @param max_len - maximum length to copy
    @returns length of resulting string
    @details Append string s to d, ensuring that the total length of d does not
   exceed max_len, and that the resulting string is null-terminated. It also
   treats newline and carriage return characters as string terminators,
   preventing them from being included in the result. This is particularly
   useful when concatenating user input or file data, where embedded newlines
   could cause issues.
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
     @param s string to terminate
     @param max_len - maximum length to scan
     @returns length of resulting string
     @details The use case is to ensure that strings read from files or user
   input do not contain embedded newlines or carriage returns. */
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
    char *p, *ms, *e;
    size_t m;
    if (s == nullptr || *s == '\0' || l == 0)
        return nullptr;
    for (p = s, m = 1; *p != '\0'; p++, m++)
        ;
    ms = p = (char *)malloc(m);
    if (ms != nullptr) {
        e = ms + l;
        while (*s != '\0' && *s != '\n' && *s != '\r' && p < e)
            *p++ = *s++;
        *p = '\0';
    }
    return ms;
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
/** @brief removes leading and trailing double quotes if present
    @ingroup utility_functions
    @param s - string to strip quotes from
    @returns true if successful, false if s is nullptr or empty
    @details If the string has a leading double quote and a trailing double quote,
   this function removes them in place. If the string does not have both leading
   and trailing double quotes, it is left unchanged. The function returns true
   if the operation was successful (i.e., if the string was modified or if it
   was valid), and false if the input string was null or empty. */
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
    @details Same as STRIP_QUOTES but returns true if quotes were removed */
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
    @returns converted integer value, or -1 if an error occurs
    @details Accepts positive integers only.
    Sets a_toi_error to (-1) on error */
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
/** @brief Converts a string to an unsigned long long integer, with support for
   suffixes K, M, and G for kilobytes, megabytes, and gigabytes respectively.
    @ingroup utility_functions
    @param str - string to convert
    @returns converted unsigned long long value, or 0 if str is nullptr, empty,
   or invalid
    @details This function is useful for parsing human-readable file sizes or
   memory sizes that may include suffixes to indicate the scale of the value.
    If the string is invalid (e.g., contains non-numeric characters other than
   the optional suffix), this function returns 0. The caller must ensure that
   the input string is a valid representation of an unsigned long long integer
   with an optional suffix before calling this function. */
unsigned long a_to_ul(const char *str) {
    char *endptr;
    unsigned long value = (unsigned long)strtoull(str, &endptr, 10);
    if (endptr == str)
        return 0;
    switch (tolower(*endptr)) {
    case 'g':
        return value * 1024ULL * 1024ULL * 1024ULL;
    case 'm':
        return value * 1024ULL * 1024ULL;
    case 'k':
        return value * 1024ULL;
    default:
        return value;
    }
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
    @details Only handles SGR sequences ending in 'm' or 'K'
    Skips non-ASCII characters
    The caller must ensure that d has enough space to hold the
    stripped string
    This function does not allocate memory; it assumes d is
    pre-allocated
    This function processes the entire string until the null
    terminator
    This function does not modify the source string s */
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
     @param file_name - name component to return
     @param fs - full file specification
     @note The caller is responsible for ensuring that "file_name" has enough space to
     receive the result. */
bool file_spec_name(char *file_name, char *fs) {
    if (fs == nullptr || *fs == '\0' || file_name == nullptr) {
        if (file_name != nullptr)
            *file_name = '\0';
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
    d = file_name;
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
/** @brief Replaces "~/" in string with the user's home directory.
    @ingroup utility_functions
    @param str - string to modify
    @param path_maxlen - maximum length of resulting string
    @returns true if successful, false if str is nullptr or empty */
bool expand_tilde(char *str, int path_maxlen) {
    if (str == nullptr || *str == '\0')
        return false;
    const char tgt[3] = "~/";
    char path[MAXLEN];
    char *e = getenv("HOME");
    strnz__cpy(path, e, MAXLEN - 1);
    strnz__cat(path, "/", MAXLEN - 1);
    char *tmp;
    tmp = rep_substring(str, tgt, path);
    strnz__cpy(str, tmp, path_maxlen - 1);
    free(tmp);
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
/** @brief Retrieves the file path associated with a given file descriptor.
    @ingroup utility_functions
    @param fd - file descriptor
    @param out_path - buffer to receive the file path
    @param out_size - size of the output buffer
    @returns 0 on success, -1 on failure
    @details This function uses the /proc filesystem to read the symbolic link
   corresponding to the file descriptor. It constructs the path to the symbolic
   link in /proc/self/fd/ and uses readlink to retrieve the actual file path.
   The caller must ensure that out_path has enough space to hold the resulting
   path. If readlink fails, this function returns -1 and does not modify
   out_path.
 */
char *fdname(int fd, char *out_path) {
    char proc_path[MAXLEN];

    snprintf(proc_path, sizeof(proc_path), "/proc/self/fd/%d", fd);
    ssize_t len = readlink(proc_path, out_path, MAXLEN - 1);
    if (len == -1)
        return nullptr;
    out_path[len] = '\0';
    return out_path;
}
char *stdio_names(char *stdio_str, char *id) {
    if (!stdio_str)
        return nullptr;
    char buf[MAXLEN] = {'\0'};
    char err_str[MAXLEN] = {'\0'};
    errno = 0;
    ssnprintf(buf, MAXLEN - 1, "%s - ", id);
    strnz__cpy(stdio_str, buf, MAXLEN - 1);
    strnz__cat(stdio_str, ttyname(0), MAXLEN - 1);
    strnz__cat(stdio_str, ", ", MAXLEN - 1);
    if (errno)
        ssnprintf(err_str, MAXLEN - 1, "Error fd %d: %s\n", 0, strerror(errno));

    strnz__cat(stdio_str, ttyname(1), MAXLEN - 1);
    strnz__cat(stdio_str, ", ", MAXLEN - 1);
    if (errno)
        ssnprintf(err_str, MAXLEN - 1, "Error fd %d: %s\n", 1, strerror(errno));

    strnz__cat(stdio_str, ttyname(2), MAXLEN - 1);
    strnz__cat(stdio_str, ", ", MAXLEN - 1);
    if (errno)
        ssnprintf(err_str, MAXLEN - 1, "Error fd %d: %s\n", 2, strerror(errno));

    return stdio_str;
}
char *stdio_fdnames(char *stdio_str, char *id) {
    if (!stdio_str)
        return nullptr;
    char buf[MAXLEN] = {'\0'};
    ssnprintf(buf, MAXLEN - 1, "%s - ", id);
    strnz__cpy(stdio_str, buf, MAXLEN - 1);
    strnz__cat(stdio_str, fdname(0, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(1, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(2, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(3, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(4, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(5, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(6, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(7, buf), MAXLEN - 1);
    strnz__cat(stdio_str, ",", MAXLEN - 1);
    strnz__cat(stdio_str, fdname(8, buf), MAXLEN - 1);
    return stdio_str;
    return stdio_str;
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
     @returns true if successful
     @details S_WCOK and S_QUIET are stripped before calling faccessat */
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
     @returns true if successful
     @details S_WCOK and S_QUIET are stripped before calling faccessat */
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
    char ifn[MAXLEN];
    char *p, *fnp, *dir;

    canonicalize_file_spec(file_name);
    strnz__cpy(ifn, file_name, MAXLEN - 1);
    fnp = ifn;
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
    @returns     0 exists
                 1 is a directory
                -1 does not exist */
bool is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) == 0)
        if (S_ISDIR(statbuf.st_mode))
            return true;
    return false;
}
/** @brief Checks if the given path is a symbolic link to a directory
    @ingroup utility_functions
    @param path - path to check
    @returns     0 exists
                 1 symbolic link to a directory
                -1 does not exist or not a symbolic link to a directory */
bool is_symlink_to_dir(const char *path) {
    struct stat link_stat;
    struct stat target_stat;

    if (lstat(path, &link_stat) == 0)
        if (S_ISLNK(link_stat.st_mode))
            if (stat(path, &target_stat) == 0)
                if (S_ISDIR(target_stat.st_mode))
                    return true; // symbolic link to a directory
    return false;
}
/** @brief Checks if the given regular expression pattern is valid
    @ingroup utility_functions
    @param pattern - regular expression pattern to check
    @returns true if the pattern is valid, false otherwise */
bool is_valid_regex(const char *pattern) {
    regex_t regex;
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    regfree(&regex);
    if (ret == 0)
        return true;
    return false;
}
/** @brief Replace all occurrences of "tgt_s" in "org_s" with "rep_s"
    @ingroup utility_functions
    @param org_s - original string
    @param tgt_s - target substring to replace
    @param rep_s - replacement substring
    @returns A pointer to the newly allocated string with replacements or a
   copy of the replacement string if original string is the same as target
   string This is a special case that allows for replacing the entire
   original string. If any parameter is nullptr, the function returns
   nullptr. If "tgt_s" is not found in "org_s", the function returns a copy
   of "org_s". If target substring is not found the function returns a copy
   of the original string.
   @note allocates memory for the return value, so the caller is
   responsible for freeing this memory when it is no longer needed to avoid
   memory leaks.
    Does not modify the original string "org_s".
   @note Assumes that "tgt_s" and "rep_s" are null-terminated strings. If
   they are not, the behavior is undefined.
   @note Does not perform any bounds checking on the input strings, so it
   is the caller's responsibility to ensure that they are valid and that the
   resulting string does not exceed available memory.
   @note Uses the standard library functions strlen, strstr, malloc, and
   strcpy, which may have their own limitations and behaviors that the
   caller should be aware of.
   @note Does not handle overlapping occurrences of "tgt_s" in "org_s". If
   "tgt_s" can overlap with itself in "org_s", the behavior may be
   unexpected. The caller should ensure that "tgt_s" does not contain
   overlapping patterns to avoid this issue.
   @note Does not handle cases where "tgt_s" is a substring of "rep_s",
   which could lead to unintended consequences if "tgt_s" appears in
   "rep_s". The caller should ensure that "tgt_s" and "rep_s" are distinct
   to avoid this issue. */
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
/** @brief String functions provide a simple string library to facilitate
   string manipulation in C, allowing developers to easily create, copy,
   concatenate, and free strings without having to manage memory manually.
    @ingroup String_Objects
   @details The library includes functions to convert C strings to String
   structs, create new String structs with specified lengths, copy and
   concatenate String structs, and free the memory used by String structs.
   By using this library, developers can avoid common pitfalls of C string
   handling, such as buffer overflows and memory leaks, while still
   benefiting from the performance advantages of C.
   Designed to be simple and easy to use, making it a great choice for
   developers who want to work with strings in C without having to worry
   about the complexities of manual memory management.
   The String struct is defined as follows:
   @code
     typedef struct {
         size_t l; // length of the string (including null terminator)
         char *s;  // pointer to the dynamically allocated string
     } String;
    @endcode
   All functions in this library that return a String struct allocate
   memory for the string using malloc or realloc. It is the caller's
   responsibility to free this memory using the free_string function when it
   is no longer needed to avoid memory leaks.
   @note The String functions in this library do not perform bounds checking
   on the input strings or the resulting strings. It is the caller's
   responsibility to ensure that all input strings are valid and that the
   resulting strings do not exceed available memory.
   @note The String functions in this library assume that all input strings
   are null-terminated. If any input string is not null-terminated, the
   behavior is undefined.
 */
/** @brief Convert C string to String struct
    @ingroup String_Objects
    @param s C string
    @return String struct containing dynamically allocated copy of input
   string
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
/** @brief Create a String struct with a dynamically allocated string @param
   l length of string to create including null terminator
   @returns String struct
   @details The returned String struct contains a dynamically allocated
   string of he specified length
   @note the caller is responsible for calling free_string to free the
   allocated memory. */
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
    @details Frees the dynamically allocated string and sets length to 0.
 */
String free_string(String string) {
    if (string.s == nullptr)
        return string;
    free(string.s);
    string.l = 0;
    string.s = nullptr;
    return string;
}
/** @brief Copy src String to dest String, allocating additional memory for
   dest String if necessary
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
/** @brief Concatenates src String to dest String, allocating additional
   memory for dest String if necessary
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
/** @brief copies up to n characters from src String to dest String,
   allocating additional memory for dest String if necessary
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
    @details This function is designed to intentionally cause a segmentation
   fault by dereferencing a null pointer. It is intended for testing
   purposes only and should not be used in production code. The caller
   should be aware that executing this function will crash the program. */
int segmentation_fault() {
    // int *p = NULL;
    // *p = 100;

    return 0;
}
/** @brief Open new C-Menu log file
    @ingroup utility_functions */
void open_cmenu_log() {
    char ttyname[MAXLEN];
    char cmenu_user[MAXLEN];
    char *p;
    cmenu_log_fd = open("/tmp/cmenu.log", O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    p = getenv("USER");
    strnz__cpy(cmenu_user, p, MAXLEN - 1);
    if (ttyname_r(STDERR_FILENO, ttyname, sizeof(ttyname)) == 0)
        strnz__cpy(em0, ttyname, MAXLEN - 1);
    ssnprintf(em0, MAXLEN - 1, "C-Menu started by user '%s' on terminal '%s'\n",
              cmenu_user, ttyname);
    write_cmenu_log(em0);
}
/** @brief Write message to C-Menu log file with timestamp
    @ingroup utility_functions
    @param msg - string to write to log file
 */
void write_cmenu_log(char *msg) {
    char time_buf[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%S%z", t);
    strnz__cpy(em1, time_buf, MAXLEN - 1);
    strnz__cat(em1, " ", MAXLEN - 1);
    strnz__cat(em1, msg, MAXLEN - 1);
    write(cmenu_log_fd, em1, strlen(em1));
    write(cmenu_log_fd, "\n", 1);
    return;
}
/** @brief Write message to C-Menu log file without timestamp
    @ingroup utility_functions
    @param msg - string to write to log file
 */
void write_cmenu_log_nt(char *msg) {
    write(cmenu_log_fd, msg, strlen(msg));
    write(cmenu_log_fd, "\n", 1);
    return;
}

void left_justify(char *s) { trim(s); }
void right_justify(char *s, int fl) {
    char *p = s;
    char *d = s + fl;
    trim(s);
    *d = '\0';
    while (*s != '\0') {
        s++;
    }
    while (s != p) {
        *(--d) = *(--s);
    }
    while (d != p) {
        *(--d) = ' ';
    }
}
bool is_valid_date(int yyyy, int mm, int dd) {
    if (yyyy < 1 || mm < 1 || mm > 12 || dd < 1)
        return false;
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((yyyy % 4 == 0 && yyyy % 100 != 0) || (yyyy % 400 == 0))
        days_in_month[2] = 29;
    if (dd > days_in_month[mm])
        return false;
    return true;
}
bool is_valid_time(int hh, int mm, int ss) {
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59)
        return false;
    return true;
}
void numeric(char *d, char *s) {
    while (*s != '\0') {
        if (*s == '-' || *s == '.' || (*s >= '0' && *s <= '9'))
            *d++ = *s++;
        else
            s++;
    }
    *d = '\0';
}

char *fill_field(char *accept_s, char *display_s, char fill_char, int flen) {
    char *s = accept_s;
    char *d = display_s;
    char *e = d + flen;
    while (*s != '\0' && d < e)
        *d++ = *s++;
    while (d < e)
        *d++ = fill_char;
    *d = '\0';
    return display_s;
}
