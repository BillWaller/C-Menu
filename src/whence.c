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
char *env_path_ptr;
char env_path_str[PATH_MAX];
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
    env_path_ptr = getenv("PATH");
    if (env_path_ptr == NULL)
        abend(argv[0], FAIL, "PATH environment variable not set");
    if (f_verbose)
        printf("%s\n", env_path_ptr);
    while (optind < argc)
        whence(argv[optind++]);
    normalend(0);
}

void usage() {
    fprintf(stderr, "usage: whence [options] file_name\n");
    fprintf(stderr, "       -a show all matches\n");
    fprintf(stderr, "       -h help\n");
    fprintf(stderr, "       -v verbose (implies -a)\n");
}

void whence(char *file_specPtr) {
    char file_spec[PATH_MAX];
    char file_dir[PATH_MAX];
    char file_name[PATH_MAX];
    char try_spec[PATH_MAX];
    char try_dir[PATH_MAX];
    int env_path_len;
    struct stat stat_struct;

    strcpy(file_spec, file_specPtr);
    strcpy(env_path_str, env_path_ptr);
    file_spec_parts(file_spec, file_dir, file_name);
    env_path_ptr = env_path_str;
    env_path_len = next_path(try_dir, &env_path_ptr);
    while (env_path_len != 0) {
        strcpy(try_spec, try_dir);
        if (try_spec[env_path_len] != '/')
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
        env_path_len = next_path(try_dir, &env_path_ptr);
    }
}

int next_path(char *dst_ptr, char **src_ptr) {
    int dst_len;

    if (**src_ptr == ':') {
        (*src_ptr)++;
        getcwd(dst_ptr, PATH_MAX);
        return (strlen(dst_ptr));
    } else {
        dst_len = 0;
        while (**src_ptr != '\0' && **src_ptr != '\n' && **src_ptr != '\r' &&
               **src_ptr != ':') {
            *dst_ptr++ = *(*src_ptr)++;
            dst_len++;
        }
        *dst_ptr = '\0';
        if (**src_ptr == ':' && *++(*src_ptr) == '\0')
            (*src_ptr)--;
        return (dst_len);
    }
}

int file_spec_parts(char *file_spec, char *file_path, char *file_name) {
    int i, last_slash;
    char tmp_file_spec[PATH_MAX];
    int file_spec_len;
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
    file_spec_len = strlen(file_spec);
    last_slash = -1;
    i = 0;
    while (i < file_spec_len) {
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
        if (last_slash < file_spec_len)
            last_slash++;
        strcpy(file_name, tmp_file_spec + last_slash);
    }
    return (0);
}

void normalend(int rc) { exit(rc); }

void abend(char *pgmid, int rc, char *err_msg) {
    fprintf(stderr, "%s; error %d; %s\n", pgmid, rc, err_msg);
    exit(rc);
}
