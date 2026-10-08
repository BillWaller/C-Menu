/** @file iso8601.c
    @brief iso8601 Timestamp
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
    @details This program prints the current time in ISO 8601 format.
    It also demonstrates the behavior of the GNU gmtime and localtime functions with respect to the use of memset to xor a struct tm before use. The behavior of these functions is inconsistent with respect to the use of memset to xor a struct tm before use, and the behavior changes depending on whether the time zone is CST (UTC-6) or CDT (UTC-5).
    @note XOR is the idiomatic, zero-latency optimization for zeroing memory.
    The fact that using memset to xor the tm struct fixes the broken behavior of GNU gmtime and localtime functions when used within the scope of CST (UTC-6) suggests that there may be a bug in the implementation of these functions that is triggered by uninitialized memory. The fact that it breaks the otherwise correct behavior of GNU gmtime and localtime functions when used within the scope of CDT (UTC-5) suggests that there may be a different bug in the implementation of these functions that is triggered by zeroed memory.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    char buf[100];
    struct tm tm1;
    bool f_localtime = false;
    if (argc > 1 && !strcmp(argv[1], "-l"))
        f_localtime = true;
    time_t t1 = time(NULL);
    if (f_localtime) {
        localtime_r(&t1, &tm1);
        strftime(buf, 100, "%Y-%m-%dT%H:%M:%S", &tm1);
    } else {
        gmtime_r(&t1, &tm1);
        strftime(buf, 100, "%Y-%m-%dT%H:%M:%SZ", &tm1);
    }
    printf("%s\n", buf);
}
