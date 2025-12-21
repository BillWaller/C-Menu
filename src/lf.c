// lf.c
// Bill Waller
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
char tmp_str[MAXLEN];

bool list_files(char *, char *, bool);
bool lf_find_files(char *, char *);
bool lf_find_dirs(char *, char *);
bool lf_write_file(int, char *);

int main(int argc, char **argv) {
    char dir[MAXLEN] = "";
    char re[MAXLEN] = "";
    int i;
    bool f_recurse = false;

    if (argc > 5) {
        snprintf(tmp_str, MAXLEN - 1, "usage: dir [regexp]\n\n");
        perror(tmp_str);
        snprintf(tmp_str, MAXLEN - 1, "Too many arguments. Maxmum is 5.");
        perror(tmp_str);
        exit(1);
    }
    i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-r") == 0) {
            f_recurse = true;
            i++;
            continue;
        } else {
            strcpy(dir, argv[i++]);
            break;
        }
    }
    f_recurse = true;
    if (i < argc)
        strcpy(re, argv[i++]);
    if (re[0] == '\0')
        strcpy(re, ".*");
    if (dir[0] == '\0')
        strcpy(dir, "./");
    list_files(dir, re, f_recurse);
}

bool list_files(char *dir, char *regexp, bool f_recurse) {
    if (f_recurse) {
        lf_find_files(dir, regexp);
        lf_find_dirs(dir, regexp);
    } else {
        lf_find_files(dir, regexp);
    }
    return true;
}

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
                strncpy(tmp_str, "can't stat ", MAXLEN - 1);
                strncat(tmp_str, file_spec, MAXLEN - strlen(tmp_str));
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

bool lf_find_files(char *dir, char *re) {
    struct stat sb;
    struct dirent *dir_st;
    DIR *dirp;
    int REG_FLAGS = 0;
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
