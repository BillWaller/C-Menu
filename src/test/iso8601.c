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

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {
    char buf[100];
    char time_s[32];
    struct tm tm1;
    time_t t1 = time(NULL);

    // ==========================================================
    strcpy(time_s, "2026-06-01T00:00:00");
    printf("input:     %s\n", time_s);
    memset(&tm1, 0, sizeof(struct tm)); // xor tm1
    printf("with memset(&tm1)\n");
    strptime(time_s, "%Y-%m-%dT%H:%M:%S", &tm1);
    t1 = mktime(&tm1);

    gmtime_r(&t1, &tm1);
    strftime(buf, 100, "gmtime     %Y-%m-%dT%H:%M:%SZ", &tm1);
    printf("%s\n", buf);

    localtime_r(&t1, &tm1);
    strftime(buf, 100, "localtime  %Y-%m-%dT%H:%M:%S", &tm1);
    printf("%s\n\n", buf);

    // used memset to xor tm1

    // input:     2026-06-01T00:00:00
    // gmtime     2026-06-01T06:00:00Z
    // localtime  2026-06-01T01:00:00

    // gmtime_r incorrectly used UTC-6 instead of UTC-5
    // localtime_r incorrectly added 1 hour
    // ----------------------------------------------------------
    strcpy(time_s, "2026-06-01T00:00:00");
    printf("input:     %s\n", time_s);
    printf("without memset(&tm1)\n");
    strptime(time_s, "%Y-%m-%dT%H:%M:%S", &tm1);
    t1 = mktime(&tm1);

    gmtime_r(&t1, &tm1);
    strftime(buf, 100, "gmtime     %Y-%m-%dT%H:%M:%SZ", &tm1);
    printf("%s\n", buf);

    localtime_r(&t1, &tm1);
    strftime(buf, 100, "localtime  %Y-%m-%dT%H:%M:%S", &tm1);
    printf("%s\n\n", buf);

    // did not use memset to xor tm1
    //
    // input:     2026-06-01T00:00:00
    // gmtime     2026-06-01T05:00:00Z
    // localtime  2026-06-01T00:00:00
    //
    // gmtime correctly used UTC-5
    // localtime correctly did not add 1 hour
    // ==========================================================
    strcpy(time_s, "2026-03-07T00:00:00");
    printf("input:     %s\n", time_s);
    printf("without memset(&tm1)\n");
    strptime(time_s, "%Y-%m-%dT%H:%M:%S", &tm1);
    t1 = mktime(&tm1);

    gmtime_r(&t1, &tm1);
    strftime(buf, 100, "gmtime     %Y-%m-%dT%H:%M:%SZ", &tm1);
    printf("%s\n", buf);

    localtime_r(&t1, &tm1);
    strftime(buf, 100, "localtime  %Y-%m-%dT%H:%M:%S", &tm1);
    printf("%s\n\n", buf);

    // did not use memset to xor tm1
    //
    // input:     2026-03-07T00:00:00
    // gmtime     2026-03-07T05:00:00Z
    // localtime  2026-03-06T23:00:00
    //
    // gmtime_r incorrectly used UTC-6 instead of UTC-5
    // localtime_r incorrectly subtracted 1 hour
    // ----------------------------------------------------------
    strcpy(time_s, "2026-03-07T00:00:00");
    printf("input:     %s\n", time_s);
    memset(&tm1, 0, sizeof(struct tm));
    printf("with memset(&tm1)\n");
    strptime(time_s, "%Y-%m-%dT%H:%M:%S", &tm1);
    t1 = mktime(&tm1);

    gmtime_r(&t1, &tm1);
    strftime(buf, 100, "gmtime     %Y-%m-%dT%H:%M:%SZ", &tm1);
    printf("%s\n", buf);

    localtime_r(&t1, &tm1);
    strftime(buf, 100, "localtime  %Y-%m-%dT%H:%M:%S", &tm1);
    printf("%s\n\n", buf);

    // used memset to xor tm1
    //
    // input:     2026-03-07T00:00:00
    // gmtime     2026-03-07T06:00:00Z
    // localtime  2026-03-07T00:00:00
    //
    // gmtime_r correctly used UTC-6 instead of UTC-5
    // localtime_r correctly did not add 1 hour
    // ==========================================================
}
