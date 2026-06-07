#define _GNU_SOURCE
/** @name dt.c
 *  @author BillWaller
 *  @date 2026-05-20
 *  @brief Test the values of DT_* and S_IF* constants.
 *  @details Because lf uses these constants to determine file types, it's
 important to verify that they have the expected values. This program prints the
 values of the DT_* constants from <dirent.h> and the S_IF* constants from
 <sys/stat.h> in both decimal and binary formats. The output can be used to
 confirm that the constants are defined as expected on the system where lf is
 being developed and tested.

    Currently the values are the same, excep the stat constants are shifted
 right by 12 bits. This is because the S_IF* constants are defined as bit masks
 in the st_mode field of the stat structure, where the file type is stored in
 the upper bits. By shifting them right by 12 bits, we can compare them directly
 to the DT_* constants, which are defined as small integer values representing
 file types in directory entries.

    Just in case you someday suspect something has changed with the data
 structures, you can run this for verification.

 */

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

typedef enum {
    /** byte 1 - bits 8-15 */
    LF_PERM_X = 0b00000001, /**< 1 Select Files with Execute Permission */
    LF_PERM_W = 0b00000010, /**< 2 Select Files with Write Permission */
    LF_PERM_R = 0b00000100, /**< 4 Select Files with Read Permission */
    LF_SETGID = 0b00010000, /**< 16 Select Setgid Files */
    LF_SETUID = 0b00100000, /**< 32 Select Setuid Files */
} LFFlags;

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
    printf("-------------------------------\n");
    printf("%-10s %2d %08b\n", "S_IXUSR", S_IXUSR >> 6, S_IXUSR >> 6);
    printf("%-10s %2d %08b\n", "S_IWUSR", S_IWUSR >> 6, S_IWUSR >> 6);
    printf("%-10s %2d %08b\n", "S_IRUSR", S_IRUSR >> 6, S_IRUSR >> 6);
    printf("%-10s %2d %08b\n", "S_ISGID", S_ISGID >> 6, S_ISGID >> 6);
    printf("%-10s %2d %08b\n", "S_ISUID", S_ISUID >> 6, S_ISUID >> 6);
    printf("-------------------------------\n");
    printf("%-10s %2d %08b\n", "LF_PERM_X", LF_PERM_X, LF_PERM_X);
    printf("%-10s %2d %08b\n", "LF_PERM_W", LF_PERM_W, LF_PERM_W);
    printf("%-10s %2d %08b\n", "LF_PERM_R", LF_PERM_R, LF_PERM_R);
    printf("%-10s %2d %08b\n", "LF_SETGID", LF_SETGID, LF_SETGID);
    printf("%-10s %2d %08b\n", "LF_SETUID", LF_SETUID, LF_SETUID);
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
