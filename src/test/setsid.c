#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int status;
    int consoledev_fd;

    if (fork()) {
        printf("Waiting for child to terminate\n");
        wait(&status);
        printf("Exiting.\n");

        return 0;
    } else {
        printf("I'm child. Closing std{in|out|err} and trying to steal the "
               "terminal!\n");
        close(0);
        close(1);
        close(2);
        setsid();

        consoledev_fd = open("/dev/pts/17", O_RDWR | O_NOCTTY);
        ioctl(consoledev_fd, TIOCSCTTY, 1);

        sleep(45);
        return 0;
    }
}
