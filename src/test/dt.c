#define _GNU_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

int main() {
    printf("%-10s %2d %08b\n", "DT_FIFO", DT_FIFO, DT_FIFO);
    printf("%-10s %2d %08b\n", "DT_CHR", DT_CHR, DT_CHR);
    printf("%-10s %2d %08b\n", "DT_DIR", DT_DIR, DT_DIR);
    printf("%-10s %2d %08b\n", "DT_BLK", DT_BLK, DT_BLK);
    printf("%-10s %2d %08b\n", "DT_REG", DT_REG, DT_REG);
    printf("%-10s %2d %08b\n", "DT_LNK", DT_LNK, DT_LNK);
    printf("%-10s %2d %08b\n", "DT_SOCK", DT_SOCK, DT_SOCK);
    printf("%-10s %2d %08b\n", "DT_UNKNOWN", DT_UNKNOWN, DT_UNKNOWN);
    printf("-------------------------------\n");
    printf("%-10s %2d %08b\n", "S_IFIFO", S_IFIFO >> 12, S_IFIFO >> 12);
    printf("%-10s %2d %08b\n", "S_IFCHR", S_IFCHR >> 12, S_IFCHR >> 12);
    printf("%-10s %2d %08b\n", "S_IFDIR", S_IFDIR >> 12, S_IFDIR >> 12);
    printf("%-10s %2d %08b\n", "S_IFBLK", S_IFBLK >> 12, S_IFBLK >> 12);
    printf("%-10s %2d %08b\n", "S_IFREG", S_IFREG >> 12, S_IFREG >> 12);
    printf("%-10s %2d %08b\n", "S_IFLNK", S_IFLNK >> 12, S_IFLNK >> 12);
    printf("%-10s %2d %08b\n", "S_IFSOCK", S_IFSOCK >> 12, S_IFSOCK >> 12);
    printf("%-10s %2d %08b\n", "S_UNKNOWN", 0, 0);
    return 0;
}

// DT_FIFO 1
// DT_CHR 2
// DT_DIR 4
// DT_BLK 6
// DT_REG 8
// DT_LNK 10
// DT_SOCK 12
// DT_UNKNOWN 0
