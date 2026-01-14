#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int fd;
    char *map;
    struct stat fileInfo;
    off_t offset;

    // 1. Open the file
    const char *filename =
        "mmap_read.c"; // The program reads its own source file
    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 2. Get file size to determine the length of the mapping
    if (fstat(fd, &fileInfo) == -1) {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }
    size_t length = fileInfo.st_size;

    // 3. Map the file into memory
    map = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // 4. Access the file content directly from the memory address
    printf("The first line of the file is:\n");
    for (offset = 0; offset < length; ++offset) {
        if (map[offset] == '\n') {
            printf("\n");
            break; // Stop after the first line
        }
        printf("%c", map[offset]);
    }

    // 5. Free the mmapped memory
    if (munmap(map, length) == -1) {
        perror("munmap");
    }

    // 6. Close the file descriptor
    close(fd);

    return 0;
}
