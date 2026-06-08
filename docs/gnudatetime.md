# DATE/TIME CONUNDRUM

Bill Waller Copyright (c) 2026  
2026-05-25  
MIT License  
[bill waller email](email@billxwaller@gmail.com)

## Overview

This note documents an issue encountered while parsing local date/time text with `strptime()`, converting the result to `time_t` with `mktime()`, and then formatting the resulting value after conversion back to local time with `localtime_r()`.

I am including this documentation because the behavior can be confusing at first, and it is easy to misattribute the problem to `localtime_r()` when the real issue is usually an uninitialized `struct tm` field that affects how `mktime()` interprets the input.

At first glance, the behavior can look like a GNU `gmtime_r()` or `localtime_r()` bug because the formatted output appears to shift by one hour depending on whether the `struct tm` was first cleared with `memset()`. In practice, the real problem is usually the value of `tm_isdst` before calling `mktime()`.

`strptime()` fills only the fields specified by the format string. It does **not** guarantee that the rest of the `struct tm` is initialized to useful values. One of the most important remaining fields is `tm_isdst`:

- `tm_isdst = 0` means standard time is in effect.
- `tm_isdst = 1` means daylight saving time is in effect.
- `tm_isdst = -1` means "determine DST automatically."

That means:

- If the structure is zeroed with `memset()`, then `tm_isdst` becomes `0`, which forces `mktime()` to interpret the input as standard time even when the date falls in CDT.
- If the structure is **not** initialized first, `tm_isdst` may retain leftover data from prior use, which can accidentally make one test case appear correct and another appear incorrect.

So the one-hour discrepancy is not caused by `localtime_r()` itself. The incorrect value is usually created earlier by `mktime()` when it receives an incompletely initialized `struct tm`.

## Correct workaround

The safe pattern is:

1. Clear the full `struct tm`.
2. Set `tm_isdst = -1`.
3. Call `strptime()`.
4. Call `mktime()`.

Example:

```c
struct tm tm1;
memset(&tm1, 0, sizeof tm1);
tm1.tm_isdst = -1;

if (strptime(time_s, "%Y-%m-%dT%H:%M:%S", &tm1) == NULL) {
    /* parse error */
}

time_t t1 = mktime(&tm1);
```

Then format for display using a separate output structure:

```c
struct tm out;
localtime_r(&t1, &out);
strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%S", &out);
```

## Why the previous results looked contradictory

The earlier examples showed this pattern:

- Zeroing the structure appeared to break CDT dates.
- Not zeroing the structure appeared to break CST dates.

That happens because the tests were effectively toggling the initial value of `tm_isdst`:

- `memset(..., 0, ...)` forced `tm_isdst = 0`, i.e. standard time.
- Reusing the same structure without reinitializing it allowed `tm_isdst` to carry stale state from earlier operations.

As a result, the code was not consistently asking `mktime()` to determine DST from the input date. Once `tm_isdst` is explicitly set to `-1`, both standard-time and daylight-time dates should round-trip correctly in the local timezone.

## Important note about `gmtime_r()`

`gmtime_r()` converts a `time_t` to UTC. It does not use the local timezone rules to decide whether the original parsed time was CST or CDT. If the UTC result appears to be off by one hour, that usually means the `time_t` value produced by `mktime()` was already wrong.

## Recommended helper

A simple helper for parsing local timestamps safely:

```c
#include <stdbool.h>
#include <string.h>
#include <time.h>

bool parse_local_timestamp(const char *s, time_t *out)
{
    struct tm tmv;
    memset(&tmv, 0, sizeof tmv);
    tmv.tm_isdst = -1;

    if (strptime(s, "%Y-%m-%dT%H:%M:%S", &tmv) == NULL)
        return false;

    time_t t = mktime(&tmv);
    if (t == (time_t)-1)
        return false;

    *out = t;
    return true;
}
```

And a matching formatter:

```c
void format_local_timestamp(time_t t, char *buf, size_t n)
{
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%S", &tmv);
}
```

## Revised interpretation of the original test cases

The original test cases are still useful because they show that leaving `tm_isdst` uncontrolled produces different behavior across DST boundaries. However, they do **not** demonstrate a `localtime_r()` or `gmtime_r()` bug. They demonstrate that `mktime()` needs a fully initialized `struct tm`, with `tm_isdst` set appropriately.

## Original demonstration code

```c
/** @file iso8601.c
    @brief iso8601 Timestamp
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
    @details This program prints the current time in ISO 8601 format.
    It also demonstrates the behavior of the GNU gmtime and localtime functions with respect to the use of memset to xor a struct tm before use, and how the behavior changes depending on whether the tm struct is zeroed before being passed through strptime() and mktime().
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

    // CDT && MEMSET - INCORRECT
    strcpy(time_s, "2026-06-01T00:00:00");
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

    // CDT && NOT MEMSET - CORRECT
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

    // NOT CDT && NOT MEMSET - INCORRECT
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

    // NOT CDT && MEMSET - CORRECT
    strcpy(time_s, "2026-03-07T00:00:00");
    printf("input:     %s\n", time_s);
    memset(&tm1, 0, sizeof(struct tm));
    strptime(time_s, "%Y-%m-%dT%H:%M:%S", &tm1);
    t1 = mktime(&tm1);

    gmtime_r(&t1, &tm1);
    strftime(buf, 100, "gmtime     %Y-%m-%dT%H:%M:%SZ", &tm1);
    printf("%s\n", buf);

    localtime_r(&t1, &tm1);
    strftime(buf, 100, "localtime  %Y-%m-%dT%H:%M:%S", &tm1);
    printf("%s\n\n", buf);
}
```
