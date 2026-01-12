/* rfc3339 timestamp */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/// rfc3339 is a very simple C program to demonstrate the usage of
/// C-Menu's String functions. Similar functionality can be provided
/// by a one-line shell command.
///
/// compile with:
/// gcc -o rfc3339 rfc3339.c
/// run with:
/// ./rfc3339
///
/// @example:
///  date -u +"%Y-%m-%dT%H:%M:%SZ"
///  // or
///  date +"%Y-%m-%dT%H:%M:%S%:z"
///
/// However, this C program illustrates how to work with
/// C-Menu's String struct and functions.
///
/// These functions are not yet widely used in C-Menu, but they will be
/// integrated into more C-Menu programs in the future.
///
/// This program simply gets the current timestamp in rfc3339 format and
/// prints it to stdout.
///
/// @file: rfc3339.c
///
/// String struct and functions
/// fn to_string(char *s) -> String
/// Convert C string to String struct
/// @param: s C string
/// @note: The returned String struct contains a dynamically allocated copy
/// of the input string.
/// @note: the caller is responsible for freeing the allocated memory.
/// @see: free_string
/// @return: String struct
/// @example:
///
///   String str = to_string("Hello, World!");
///   //   // Use str.s and str.l
///   str = free_string(str);
///
typedef struct {
    char *s;  // pointer to string
    size_t l; // length of string including null terminator
} String;
/// fn mk_string(size_t l) -> String
/// Create an empty String struct with specified length
/// @param: l Length of the string (excluding null terminator)
/// @note: The returned String struct contains a dynamically allocated
/// string of length l + 1 (including null terminator).
/// @note: the caller is responsible for freeing the allocated memory.
/// @see: free_string
/// @return: String struct
String mk_string(size_t l) {
    String str;
    str.l = l + 1;
    str.s = (char *)malloc(str.l);
    str.s[0] = '\0';
    return str;
}
/// fn to_string(const char *s) -> String
/// Convert C string to String struct
/// @param: s C string
/// @note: The returned String struct contains a dynamically allocated copy
/// of the input string.
/// @note: the caller is responsible for freeing the allocated memory.
/// @see: free_string
/// @return: String struct
String to_string(const char *s) {
    String str;
    str.l = strlen(s) + 1;
    str.s = (char *)malloc(str.l);
    strcpy(str.s, s);
    return str;
}
/// fn free_string(String str) -> String
/// Free the memory allocated for a String struct
/// @param: str String struct to free
/// @note: This function frees the memory allocated for the string
/// in the String struct and sets its length to 0 and pointer to NULL.
/// @return: String struct with NULL pointer and length 0
String free_string(String str) {
    free(str.s);
    str.l = 0;
    str.s = NULL;
    return str;
}
/// fn get_rfc3339_str(String timestamp_s)
/// Get the current timestamp in rfc3339 format
/// @param: timestamp_s String struct to store the rfc3339 timestamp
/// @note: The caller is responsible for ensuring that timestamp_s
/// has enough space to hold the rfc3339 timestamp (at least 30 bytes).
/// @example: see main()
void get_rfc3339_str(String timestamp_s) {
    time_t now;
    struct tm *tms;
    int use_localtime = 0;
    String z_str1, z_str2;
    z_str1 = mk_string(30);
    z_str2 = mk_string(30);
    int i = 0, j;

    time(&now);
    tms = localtime((time_t *)&now);
    strftime(z_str1.s, z_str1.l, "%z", tms);
    for (j = 0; j < 6; j++) {
        if (i == 3)
            z_str2.s[i++] = ':';
        z_str2.s[i++] = z_str1.s[j];
    }
    z_str2.s[6] = '\0';
    sprintf(timestamp_s.s, "%04d-%02d-%02dT%02d:%02d:%02d%s",
            tms->tm_year + 1900, tms->tm_mon + 1, tms->tm_mday, tms->tm_hour,
            tms->tm_min, tms->tm_sec, z_str2.s);
    free_string(z_str1);
    free_string(z_str2);
}
int main() {
    String timestamp_s;
    timestamp_s = mk_string(30);
    get_rfc3339_str(timestamp_s);
    printf("%s\n", timestamp_s.s);
    free_string(timestamp_s);
}
