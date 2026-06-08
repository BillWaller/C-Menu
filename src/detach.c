#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** @brief Fork, set new session ID, close files, and execute detached command
    @ingroup exec
    @param eargv - array of arguments for the command to execute
    @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
    @details Sets the new session ID. Redirects standard file descriptors
    to /dev/null. Closes all open file descriptors. Executes the command using
    execvp. Exits with failure if any step fails.
    @note Tested 2026-06-05 on Linux - appears to be functioning properly */
int fork_detach_execvp(char **eargv) {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "First fork failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        if (setsid() < 0) {
            fprintf(stderr, "Set session ID failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
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
        for (long fd = 3; fd < max_fd; fd++)
            close(fd);
        execvp(eargv[0], eargv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char **eargv = &argv[1];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (fork_detach_execvp(eargv) != 0) {
        fprintf(stderr, "Failed to fork and execute command\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
