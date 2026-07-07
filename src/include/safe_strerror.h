#ifndef SAFE_STRERROR_H
#define SAFE_STRERROR_H

// 1. Force feature test macros before any standard headers are pulled in
#if defined(__linux__) || defined(__gnu_linux__)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#endif

#include <errno.h>
#include <stddef.h>
#include <string.h>

/**
 * @brief Thread-safe cross-platform error string retriever.
 * @param errnum The errno code (e.g., errno).
 * @param buf User-provided destination character buffer.
 * @param buflen Size of the provided buffer.
 * @return char* Pointer to the error string (either 'buf' or an internal static string).
 */
static inline char *safe_strerror(int errnum, char *buf, size_t buflen) {
    if (buf == nullptr || buflen == 0) {
        return "Invalid buffer provided";
    }

#if defined(_WIN32) || defined(_WIN64)
    // Windows implementation using bounds-checked standard function
    if (strerror_s(buf, buflen, errnum) == 0) {
        return buf;
    }
    return "Unknown Windows error";

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || (defined(_POSIX_C_SOURCE) && !defined(_GNU_SOURCE))
    // POSIX-compliant version (returns int) used by macOS and POSIX-strict environments
    if (strerror_r(errnum, buf, buflen) == 0) {
        return buf;
    }
    return "Unknown POSIX error";

#elif defined(__linux__) || defined(__gnu_linux__)
    // GNU-specific strerror_r (returns a char*, which might NOT be the buffer you passed)
    return strerror_r(errnum, buf, buflen);

#else
    // Fallback if no thread-safe alternative is safely exposed
    // Note: plain strerror is not thread-safe, but acts as a last resort
    (void)buf;
    (void)buflen;
    return strerror(errnum);
#endif
}

#endif // SAFE_STRERROR_H
