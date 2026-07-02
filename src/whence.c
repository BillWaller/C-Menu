/** @file whence.c
    @brief Find the full path of a file in the directories specified by the
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#define _GNU_SOURCE
#include <argp.h>
#include <cm.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char *path_p;
char path_s[MAXLEN];
// char *file_name[MAXLEN + 1];

int whence(char *, int);
int next_path(char *, char **);
int file_spec_parts(char *, char *, char *);
void ABEND(char *, int, char *);
size_t strnz__cat(char *, const char *, size_t);
size_t strnz__cpy(char *, const char *, size_t);
typedef enum { WH_ALL = 1,
               WH_VERBOSE = 2,
               WH_EXECUTABLE = 4,
               WH_SETUID = 8 } WhenceFlags;
int wh_flags = 0;
const char *argp_program_version = CM_VERSION;
const char *argp_program_bug_address = "billxwaller@gmail.com";
static char doc[] = "whence locate files in path";
static char args_doc[] = "";

static struct argp_option options[] = {
    {"all", 'a', 0, 0, "list all matches", 0},
    {"setuid", 's', 0, 0, "setuid only", 0},
    {"executable", 'x', 0, 0, "executable only", 0},
    {"verbose", 'v', 0, 0, "verbose messages", 0},
    {0}};

struct wh_opts {
    int flags;
    int argc;
    char *argv[MAXARGS];
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct wh_opts *wh_opts = state->input;
    switch (key) {
    case 'a':
        wh_opts->flags |= WH_ALL;
        break;
    case 's':
        wh_opts->flags |= WH_SETUID;
        break;
    case 'v':
        wh_opts->flags |= WH_VERBOSE;
        break;
    case 'x':
        wh_opts->flags |= WH_EXECUTABLE;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num == 0 || state->arg_num == 1) {
            wh_opts->argv[state->arg_num] = arg;
            wh_opts->argc = state->arg_num + 1;
        } else {
            argp_usage(state);
        }
        break;
    case ARGP_KEY_END:
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
static struct argp argp = {options, parse_opt, args_doc, doc,
                           nullptr, nullptr, nullptr};

int main(int argc, char **argv) {
    struct wh_opts wh_opts = {0};
    wh_opts.flags = 0;
    wh_opts.argv[0] = nullptr;
    int i = 0;
    int found = 0;
    argp_parse(&argp, argc, argv, 0, 0, &wh_opts);
    path_p = getenv("PATH");
    if (path_p == nullptr)
        ABEND(argv[0], 0, "PATH environment variable not set");
    if (wh_opts.flags & WH_VERBOSE)
        printf("%s\n", path_p);
    while (i < wh_opts.argc) {
        found = whence(wh_opts.argv[i++], wh_opts.flags);
    }
    if (found == 0)
        exit(1);
    exit(0);
}
/** @brief Find the full path of a file in the directories specified by the PATH
   environment variable
    @param file_spec_p The file specification to search for
    @param flags Flags to control the behavior of the search (e.g., verbose
   mode, list all matches)
    @details This function takes a file specification, extracts the directory
   and file name components, and searches through the directories specified in
   the PATH environment variable to find matches. It prints the full path of
   each match found, and if verbose mode is enabled, it also indicates whether
   each attempted path was found or not. */
int whence(char *file_spec_p, int flags) {
    char file_spec[PATH_MAX];
    char file_dir[PATH_MAX];
    char file_name[PATH_MAX];
    char try_spec[PATH_MAX];
    char try_dir[PATH_MAX];
    int path_l;
    int found = 0;
    struct stat stat_struct;

    strnz__cpy(file_spec, file_spec_p, MAXLEN - 1);
    strnz__cpy(path_s, path_p, MAXLEN - 1);
    file_spec_parts(file_spec, file_dir, file_name);
    path_p = path_s;
    path_l = next_path(try_dir, &path_p);
    while (path_l != 0) {
        strnz__cpy(try_spec, try_dir, MAXLEN - 1);
        if (try_spec[path_l] != '/')
            strnz__cat(try_spec, "/", MAXLEN - 1);
        strnz__cat(try_spec, file_name, MAXLEN - 1);
        if (stat(try_spec, &stat_struct) != 0) {
            if (flags & WH_VERBOSE)
                printf("-      %s\n", try_spec);
            path_l = next_path(try_dir, &path_p);
            continue;
        }
        if (flags & WH_SETUID)
            if (!(stat_struct.st_mode & S_ISUID)) {
                path_l = next_path(try_dir, &path_p);
                continue;
            }
        if (flags & WH_EXECUTABLE)
            if (!(stat_struct.st_mode & S_ISUID)) {
                path_l = next_path(try_dir, &path_p);
                continue;
            }
        if (!(stat_struct.st_mode & S_IXUSR)) {
            path_l = next_path(try_dir, &path_p);
            continue;
        }
        found++;
        if (flags & WH_VERBOSE)
            printf("found  %s\n", try_spec);
        else {
            printf("%s\n", try_spec);
            if (!(flags & WH_ALL))
                return found;
        }
        path_l = next_path(try_dir, &path_p);
    }
    return found;
}
/** @brief Extract the next directory path from the PATH string
    @param dp A buffer to store the extracted directory path
    @param sp A pointer to the current position in the PATH string
    @return The length of the extracted directory path
    @details This function takes a buffer and a pointer to the current position
   in the PATH string, and extracts the next directory path. If the next
   character in the PATH string is a colon, it treats it as an empty path and
   uses the current working directory. Otherwise, it copies characters until it
   reaches a colon or the end of the string, and returns the length of the
   extracted path.
 */
int next_path(char *dp, char **pp) {
    int dl;

    if (**pp == ':') {
        (*pp)++;
        getcwd(dp, PATH_MAX);
        return (strlen(dp));
    } else {
        dl = 0;
        while (**pp != '\0' && **pp != '\n' && **pp != '\r' && **pp != ':') {
            *dp++ = *(*pp)++;
            dl++;
        }
        *dp = '\0';
        if (**pp == ':' && *++(*pp) == '\0')
            (*pp)--;
        return (dl);
    }
}
/** @brief Split a file specification into directory and file name components
    @param file_spec The full file specification to split
    @param file_path A buffer to store the extracted directory path
    @param file_name A buffer to store the extracted file name
    @return 0 on success
    @details This function takes a file specification, checks if it is a
   directory, and if so, it sets the file path accordingly. If the file
   specification is empty, it defaults to the current directory. Otherwise, it
   splits the file specification into the directory and file name components
   based on the last occurrence of a slash ('/'). */
int file_spec_parts(char *file_spec, char *file_path, char *file_name) {
    int i, last_slash;
    char tmp_file_spec[PATH_MAX];
    int file_spec_l;
    struct stat stat_struct;

    if (stat(file_spec, &stat_struct) != -1)
        if ((stat_struct.st_mode & S_IFMT) == S_IFDIR) {
            if (file_spec[strlen(file_path)] != '/')
                strnz__cat(file_spec, "/", MAXLEN - 1);
            strnz__cpy(file_path, file_spec, MAXLEN - 1);
            file_name[0] = '\0';
            return (0);
        }
    if (strlen(file_spec) == 0) {
        strnz__cpy(file_spec, "./", MAXLEN - 1);
        file_name[0] = '\0';
        return (0);
    }
    strnz__cpy(tmp_file_spec, file_spec, MAXLEN - 1);
    last_slash = -1;
    file_spec_l = strlen(tmp_file_spec);
    if (file_spec_l > 0) {
        i = 0;
        while (i < file_spec_l && tmp_file_spec[i] != '\0') {
            if (tmp_file_spec[i] == '/') {
                last_slash = i;
                break;
            }
            i++;
        }
    }
    if (last_slash < 0) {
        strnz__cpy(file_path, "./", MAXLEN - 1);
        if (strcmp(file_spec, ".") == 0)
            file_name[0] = '\0';
        else
            strnz__cpy(file_name, tmp_file_spec, MAXLEN - 1);
        strnz__cpy(file_spec, file_path, MAXLEN - 1);
        strnz__cat(file_spec, file_name, MAXLEN - 1);
    } else {
        tmp_file_spec[last_slash] = '\0';
        strnz__cpy(file_path, tmp_file_spec, MAXLEN - 1);
        strnz__cat(file_path, "/", MAXLEN - 1);
        if (last_slash < file_spec_l)
            last_slash++;
        strnz__cpy(file_name, tmp_file_spec + last_slash, MAXLEN - 1);
    }
    return (0);
}
/** @brief Exit the program with an error message
    @param pgmid The name of the program
    @param rc The return code to exit with
    @param err_msg The error message to display
    @details This function is called to exit the program with an error status.
   It prints the program name, return code, and error message to the standard
   error stream, and then exits with the specified return code. */
void ABEND(char *pgmid, int rc, char *err_msg) {
    fprintf(stderr, "%s; error %d; %s\n", pgmid, rc, err_msg);
    exit(EXIT_FAILURE);
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
