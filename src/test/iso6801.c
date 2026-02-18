/** @file iso6801.c
    @brief iso6801 Timestamp
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/** This program prints the current time in ISO 6801 format. If the first
 * argument is "-l", it prints the local time with the timezone offset.
 * Otherwise, it prints the UTC time with a "Z" suffix. The output format is UTC
 * "YYYY-MM-DDTHH:MM:SSZ" LOCAL "YYYY-MM-DDTHH:MM:SS±hhmm" */

int main(int argc, char **argv) {
    char buf[100];
    time_t now = time(NULL);
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'l' &&
        argv[1][2] == '\0') {
        struct tm *t = localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", t);
    } else {
        struct tm *t = gmtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);
    }
    printf("%s\n", buf);
    return 0;
}
