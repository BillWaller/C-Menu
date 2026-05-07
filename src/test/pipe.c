#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAXARGS 10

int main() {
    int fds[2];
    pipe(fds); // Create the pipe
    char *eargv[MAXARGS];

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(fds[1]);              // Close unused write end
        dup2(fds[0], STDIN_FILENO); // Redirect read pipe to STDIN
        close(fds[0]);              // Close duplicated read pipe
        int argc = 0;
        eargv[argc++] = strdup("sort");
        eargv[argc++] = strdup("-r");
        eargv[argc] = nullptr;
        execvp(eargv[0], eargv);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(fds[0]);               // Close unused read pipe
        dup2(fds[1], STDOUT_FILENO); // Redirect STDOUT to write pipe
        close(fds[1]);               // Close duplicated write pipe
        int argc = 0;
        eargv[argc++] = strdup("lf");
        eargv[argc] = nullptr;
        execvp(eargv[0], eargv);
    }

    // Parent must close its copies of the pipe for the second child to exit
    close(fds[0]); // Close read pipe
    close(fds[1]); // Close write pipe

    int wstatus1 = 0;
    waitpid(pid1, &wstatus1, 0); // Wait for children

    int wstatus2 = 0;
    waitpid(pid2, &wstatus2, 0); // Wait for children
    return 0;
}
