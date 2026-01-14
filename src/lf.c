//  lf.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
///  Recursively list files matching a regular expression

#include <dirent.h>
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
#include <unistd.h>

#define MAXLEN 256

/// ╭───────────────────────────────────────────────────────────────╮
/// │ FLAGS                                                         │
/// ╰───────────────────────────────────────────────────────────────╯
/// Search flags
/// ALL     List all files including hidden files
/// RECURSE Recurse into subdirectories
/// ICASE   Ignore case in search
#define ALL 0x01
#define RECURSE 0x02
#define ICASE 0x04

char tmp_str[MAXLEN];

bool list_files(char *, char *, int);
bool lf_find_files(char *, char *, int);
bool lf_find_dirs(char *, char *, int);
bool lf_write_file(int, char *, int);
int strnz__cat(char *, char *, int);
int strnz__cpy(char *, char *, int);
int max_depth = 16;
int depth = 0;

/// ╭───────────────────────────────────────────────────────────────╮
/// │ LF EXECUTABLE                                                 │
/// ╰───────────────────────────────────────────────────────────────╯
/// @usage   Usage: lf [options] [directory] [regexp]
/// @usage   Options:\n");
/// @param     -a        List all files (including hidden files)
/// @param     -d        maximum depth of subdirectories to examine
/// @param     -h        show this help message
/// @param     -i        ignore case in search
/// @param     -r        recurse into subdirectories
/// @param     -v        show version information

int main(int argc, char **argv) {
    char dir[MAXLEN] = "";
    char re[MAXLEN] = "";
    bool f_help = false;
    bool f_version = false;
    int flags = 0;
    int opt;

    while ((opt = getopt(argc, argv, "ad:hrv")) != -1) {
        switch (opt) {
        case 'a':
            flags |= ALL;
            break;
        case 'd':
            max_depth = atoi(optarg);
            break;
        case 'h':
            f_help = true;
            break;
        case 'i':
            flags |= ICASE;
            break;
        case 'r':
            flags |= RECURSE;
            break;
        case 'v':
            f_version = true;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    if (f_help) {
        printf("Usage: lf [options] [directory] [regexp]\n");
        printf("Options:\n");
        printf("  -a        List all files (including hidden files)\n");
        printf("  -d        maximum depth of subdirectories to examine\n");
        printf("  -h        show this help message\n");
        printf("  -i        ignore case in search\n");
        printf("  -r        recurse into subdirectories\n");
        printf("  -v        show version information\n");
        exit(EXIT_SUCCESS);
    }
    if (f_version) {
        printf("lf version 1.0\n");
        exit(EXIT_SUCCESS);
    }
    if (optind < argc) {
        strnz__cpy(dir, argv[optind], MAXLEN - 1);
        optind++;
    }
    if (optind < argc) {
        strnz__cpy(re, argv[optind], MAXLEN - 1);
        optind++;
    }
    if (re[0] == '\0')
        strnz__cpy(re, ".*", MAXLEN - 1);
    if (dir[0] == '\0')
        strnz__cpy(dir, ".", MAXLEN - 1);
    if (flags & RECURSE) {
        lf_find_files(dir, re, flags);
        lf_find_dirs(dir, re, flags);
    } else {
        lf_find_files(dir, re, flags);
    }
    return true;
}

/// ╭───────────────────────────────────────────────────────────────╮
/// │ LF_FIND_DIRS                                                  │
/// ╰───────────────────────────────────────────────────────────────╯
/// Recursively find directories and call lf_find_files on each
/// directory found
/// @param dir   starting directory
/// @param re    regular expression to match files
/// @param flags search flags
/// return      true if successful, false otherwise
bool lf_find_dirs(char *dir, char *re, int flags) {
    struct stat sb;
    struct dirent *dir_st;
    DIR *dirp;
    char dir_s[MAXLEN];
    char file_spec[MAXLEN];

    if (depth == max_depth)
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
                lf_find_dirs(dir_s, re, flags);
            }
        }
        dir_st = readdir(dirp);
    }
    closedir(dirp);
    depth--;
    return true;
}

/// ╭───────────────────────────────────────────────────────────────╮
/// │ LF_FIND_FILES                                                 │
/// ╰───────────────────────────────────────────────────────────────╯
/// Find files in a directory matching a regular expression
/// @param dir   directory to search
/// @param re    regular expression to match files
/// @param flags search flags
/// return      true if successful, false otherwise
bool lf_find_files(char *dir, char *re, int flags) {
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

/// ╭───────────────────────────────────────────────────────────────╮
/// │ STRNZ_CPY                                                     │
/// │ stops at max_len, newline, or carriage return                 │
/// │ max_len limits the destination buffer size                    │
/// │ returns length of resulting string                            │
/// ╰───────────────────────────────────────────────────────────────╯
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
/// ╭───────────────────────────────────────────────────────────────╮
/// │ STRNZ_CAT                                                     │
/// │ stops at max_len, newline, or carriage return                 │
/// │ max_len limits the destination buffer size                    │
/// │ returns length of resulting string                            │
/// ╰───────────────────────────────────────────────────────────────╯
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
