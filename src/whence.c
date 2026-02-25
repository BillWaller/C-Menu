/** @file whence.c
    @brief Find the full path of a file in the directories specified by the
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <cm.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define FAIL -1

bool f_all, f_display_help, f_verbose;
char *path_p;
char path_s[MAXLEN];
char *file_name[MAXLEN + 1];

void whence_usage();
void whence(char *);
int next_path(char *, char **);
int file_spec_parts(char *, char *, char *);
void ABEND(char *, int, char *);
void normalend();
/** @brief Main function for whence utility
    @param argc Argument count
    @param argv Argument vector
    @return Exit status
    options include:
    -a: Show all matches (default is to show only the first match)
    -h: Display help message
    -v: Verbose mode (implies -a)
    @note This function processes command-line arguments, retrieves the PATH
   environment variable, and calls the whence function for each specified file.
 */
int main(int argc, char **argv) {

    int opt;
    if (argc < 2)
        whence_usage();
    while ((opt = getopt(argc, argv, "ahv")) != -1) {
        switch (opt) {
        case 'a':
            f_all = TRUE;
            break;
        case 'h':
            f_display_help = TRUE;
            break;
        case 'v':
            f_verbose = TRUE;
            break;
        default:
            ABEND(argv[0], FAIL, "unrecognized option");
        }
    }
    if (f_display_help) {
        whence_usage();
        normalend();
    }
    path_p = getenv("PATH");
    if (path_p == NULL)
        ABEND(argv[0], FAIL, "PATH environment variable not set");
    if (f_verbose)
        printf("%s\n", path_p);
    while (optind < argc) {
        whence(argv[optind++]);
    }
    normalend();
}
/** @brief Display usage information for the whence utility
 *  @note This function prints the usage instructions and available options for
 * the whence utility to the standard error stream.
 */
void whence_usage() {
    fprintf(stderr, "whence_usage: whence [options] file_name\n");
    fprintf(stderr, "       -a show all matches\n");
    fprintf(stderr, "       -h help\n");
    fprintf(stderr, "       -v verbose (implies -a)\n");
}
/** @brief Find the full path of a file in the directories specified by the PATH
   environment variable
    @param file_spec_p The file specification to search for
    @note This function takes a file specification, extracts the directory and
   file name components, and searches through the directories specified in the
   PATH environment variable to find matches. It prints the full path of each
   match found, and if verbose mode is enabled, it also indicates whether each
   attempted path was found or not. */
void whence(char *file_spec_p) {
    char file_spec[PATH_MAX];
    char file_dir[PATH_MAX];
    char file_name[PATH_MAX];
    char try_spec[PATH_MAX];
    char try_dir[PATH_MAX];
    int path_l;
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
/** @brief Extract the next directory path from the PATH string
    @param dp A buffer to store the extracted directory path
    @param sp A pointer to the current position in the PATH string
    @return The length of the extracted directory path
    @note This function takes a buffer and a pointer to the current position in
   the PATH string, and extracts the next directory path. If the next character
   in the PATH string is a colon, it treats it as an empty path and uses the
   current working directory. Otherwise, it copies characters until it reaches a
   colon or the end of the string, and returns the length of the extracted path.
 */
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
/** @brief Split a file specification into directory and file name components
    @param file_spec The full file specification to split
    @param file_path A buffer to store the extracted directory path
    @param file_name A buffer to store the extracted file name
    @return 0 on success
    @note This function takes a file specification, checks if it is a directory,
   and if so, it sets the file path accordingly. If the file specification is
   empty, it defaults to the current directory. Otherwise, it splits the file
   specification into the directory and file name components based on the last
   occurrence of a slash ('/'). */
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
/** @brief Exit the program successfully
    @note This function is called to exit the program with a success status. It
   simply calls the exit function with EXIT_SUCCESS. */
void normalend() { exit(EXIT_SUCCESS); }
/** @brief Exit the program with an error message
    @param pgmid The name of the program
    @param rc The return code to exit with
    @param err_msg The error message to display
    @note This function is called to exit the program with an error status. It
   prints the program name, return code, and error message to the standard error
   stream, and then exits with the specified return code. */
void ABEND(char *pgmid, int rc, char *err_msg) {
    fprintf(stderr, "%s; error %d; %s\n", pgmid, rc, err_msg);
    exit(EXIT_FAILURE);
}
