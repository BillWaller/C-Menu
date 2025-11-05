/* whence.c
 * whence: adverb
 *           from what place or source
 *           in this case, a directory name
 * Bill Waller
 * billxwaller@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#define FAIL -1
#define PATH_MAX 256

int f_all, f_help, f_verbose;
char *path_p;
char path_s[PATH_MAX];
char *file_name[PATH_MAX + 1];

void usage();
void whence(char *);
int next_path(char *, char **);
int file_spec_parts(char *, char *, char *);
void abend(char *, int, char *);
void normalend(int);

int main(int argc, char **argv) {

    int opt;
    if (argc < 2)
        usage();
    while ((opt = getopt(argc, argv, "ahv")) != -1) {
        switch (opt) {
        case 'a':
            f_all = TRUE;
            break;
        case 'h':
            f_help = TRUE;
            break;
        case 'v':
            f_verbose = TRUE;
            break;
        default:
            abend(argv[0], FAIL, "unrecognized option");
        }
    }
    if (f_help) {
        usage();
        normalend(0);
    }
    path_p = getenv("PATH");
    if (path_p == NULL)
        abend(argv[0], FAIL, "PATH environment variable not set");
    if (f_verbose)
        printf("%s\n", path_p);
    while (optind < argc) {
        whence(argv[optind++]);
    }
    normalend(0);
}

void usage() {
    fprintf(stderr, "usage: whence [options] file_name\n");
    fprintf(stderr, "       -a show all matches\n");
    fprintf(stderr, "       -h help\n");
    fprintf(stderr, "       -v verbose (implies -a)\n");
}

void whence(char *file_spec_p) {
    char file_spec[PATH_MAX];
    char file_dir[PATH_MAX];
    char file_name[PATH_MAX];
    char try_spec[PATH_MAX];
    char try_dir[PATH_MAX];
    int path_l;
    struct stat stat_struct;

    strcpy(file_spec, file_spec_p);
    strcpy(path_s, path_p);
    file_spec_parts(file_spec, file_dir, file_name);
    path_p = path_s;
    path_l = next_path(try_dir, &path_p);
    while (path_l != 0) {
        strcpy(try_spec, try_dir);
        if (try_spec[path_l] != '/')
            strcat(try_spec, "/");
        strcat(try_spec, file_name);
        if (f_verbose) {
            if (stat(try_spec, &stat_struct) == -1)
                printf("-      %s\n", try_spec);
            else
                printf("found  %s\n", try_spec);
        } else if (stat(try_spec, &stat_struct) == 0) {
            printf("%s\n", try_spec);
            if (!f_all)
                return;
        }
        path_l = next_path(try_dir, &path_p);
    }
}

int next_path(char *dp, char **sp) {
    int dl;

    if (**sp == ':') {
        (*sp)++;
        getcwd(dp, PATH_MAX);
        return (strlen(dp));
    } else {
        dl = 0;
        while (**sp != '\0' && **sp != '\n' && **sp != '\r' && **sp != ':') {
            *dp++ = *(*sp)++;
            dl++;
        }
        *dp = '\0';
        if (**sp == ':' && *++(*sp) == '\0')
            (*sp)--;
        return (dl);
    }
}

int file_spec_parts(char *file_spec, char *file_path, char *file_name) {
    int i, last_slash;
    char tmp_file_spec[PATH_MAX];
    int file_spec_l;
    struct stat stat_struct;

    if (stat(file_spec, &stat_struct) != -1)
        if ((stat_struct.st_mode & S_IFMT) == S_IFDIR) {
            if (file_spec[strlen(file_path)] != '/')
                strcat(file_spec, "/");
            strcpy(file_path, file_spec);
            file_name[0] = '\0';
            return (0);
        }
    if (strlen(file_spec) == 0) {
        strcpy(file_spec, "./");
        strcpy(file_path, "./");
        file_name[0] = '\0';
        return (0);
    }
    strcpy(tmp_file_spec, file_spec);
    file_spec_l = strlen(file_spec);
    last_slash = -1;
    i = 0;
    while (i < file_spec_l) {
        if (tmp_file_spec[i] == '/')
            last_slash = i;
        i++;
    }
    if (last_slash < 0) {
        strcpy(file_path, "./");
        if (strcmp(file_spec, ".") == 0)
            file_name[0] = '\0';
        else
            strcpy(file_name, tmp_file_spec);
        strcpy(file_spec, file_path);
        strcat(file_spec, file_name);
    } else {
        tmp_file_spec[last_slash] = '\0';
        strcpy(file_path, tmp_file_spec);
        strcat(file_path, "/");
        if (last_slash < file_spec_l)
            last_slash++;
        strcpy(file_name, tmp_file_spec + last_slash);
    }
    return (0);
}

void normalend(int rc) { exit(EXIT_SUCCESS); }

void abend(char *pgmid, int rc, char *err_msg) {
    fprintf(stderr, "%s; error %d; %s\n", pgmid, rc, err_msg);
    exit(EXIT_FAILURE);
}
