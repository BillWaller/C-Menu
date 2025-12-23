// lf.c
// recursively list files

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

#define ALL 0x01
#define RECURSE 0x02

char tmp_str[MAXLEN];

bool list_files(char *, char *, int);
bool lf_find_files(char *, char *, int);
bool lf_find_dirs(char *, char *, int);
bool lf_write_file(int, char *, int);

int main(int argc, char **argv) {
    char dir[MAXLEN] = "";
    char re[MAXLEN] = "";
    bool f_help = false;
    bool f_version = false;
    int flags = 0;
    int opt;

    while ((opt = getopt(argc, argv, "ahrv")) != -1) {
        switch (opt) {
        case 'a':
            flags |= ALL;
            break;
        case 'h':
            f_help = true;
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
        printf("  -h        Show this help message\n");
        printf("  -r        Recurse into subdirectories\n");
        printf("  -v        Show version information\n");
        exit(EXIT_SUCCESS);
    }
    if (f_version) {
        printf("lf version 1.0\n");
        exit(EXIT_SUCCESS);
    }
    if (optind < argc) {
        strcpy(dir, argv[optind]);
        optind++;
    }
    if (optind < argc) {
        strcpy(re, argv[optind]);
        optind++;
    }
    if (re[0] == '\0')
        strcpy(re, ".*");
    if (dir[0] == '\0')
        strcpy(dir, ".");
    if (flags & RECURSE) {
        lf_find_files(dir, re, flags);
        lf_find_dirs(dir, re, flags);
    } else {
        lf_find_files(dir, re, flags);
    }
    return true;
}

bool lf_find_dirs(char *dir, char *re, int flags) {
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
                strncpy(tmp_str, "can't stat ", MAXLEN - 1);
                strncat(tmp_str, file_spec, MAXLEN - strlen(tmp_str));
                perror(tmp_str);
                return false;
            }
            if ((sb.st_mode & S_IFMT) == S_IFDIR) {
                strcpy(dir_s, file_spec);
                lf_find_files(dir_s, re, flags);
                lf_find_dirs(dir_s, re, flags);
            }
        }
        dir_st = readdir(dirp);
    }
    closedir(dirp);
    return true;
}

bool lf_find_files(char *dir, char *re, int flags) {
    struct stat sb;
    struct dirent *dir_st;
    DIR *dirp;
    int REG_FLAGS = 0;
    int reti;
    regmatch_t pmatch[1];
    regex_t compiled_re;
    char file_spec[MAXLEN];
    char *file_spec_p;

    reti = regcomp(&compiled_re, re, REG_FLAGS);
    if (reti) {
        perror("Invalid pattern");
        return false;
    }
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
                strcpy(tmp_str, "can't stat ");
                strcat(tmp_str, file_spec);
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
                strcpy(tmp_str, "Regex match failed: ");
                strcat(tmp_str, msgbuf);
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
