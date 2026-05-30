#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int dmon(char **argv);

int main(int argc, char *argv[]) {
    int rc;
    char **eargv = calloc((argc - 1), sizeof(char *));
    for (int i = 0; i < (argc - 1); i++) {
        eargv[i] = argv[i + 1];
    }
    rc = dmon(eargv);
    free(eargv);
    exit(rc);
}

int dmon(char **eargv) {

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Failed to fork: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if (pid > 0) {
        return EXIT_SUCCESS;
    }
    if (setsid() < 0) {
        fprintf(stderr, "Failed to set session ID: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Second fork failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    char *e = getenv("HOME");
    if (e != NULL) {
        if (chdir(e) < 0)
            exit(EXIT_FAILURE);
    }
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null != -1) {
        dup2(dev_null, STDIN_FILENO);
        dup2(dev_null, STDOUT_FILENO);
        dup2(dev_null, STDERR_FILENO);
        if (dev_null > 2) {
            close(dev_null);
        }
    }
    long max_fd = sysconf(_SC_OPEN_MAX);
    for (long fd = 3; fd < max_fd; fd++) {
        close(fd);
    }
    execvp(eargv[0], eargv);
    exit(EXIT_FAILURE);
}
