/** @file strings_test1.c
    @brief String Objects
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09 */

/** @test Test string concatenation functions
    This test verifies that the string concatenation functions work correctly.
    It creates two strings, concatenates them, and checks the results.
    @note It is not necessary to pre-allocate spaace for str1 or str2  */

#include <cm.h>
#include <stddef.h>

int main() {
    String str1 = to_string("Hello");
    String str2 = to_string(" World!");
    string_cat(&str1, &str2);
    printf("%s\n", str1.s); /**< Output: Hello World */
    printf("bytes allocated for str1.s: %zu\n", str1.l);
    string_ncat(&str1, &str2, 3);
    printf("%s\n", str1.s); /**< Output: Hello World Wo */
    printf("bytes allocated for str1.s: %zu\n", str1.l);
    free_string(str1);
    free_string(str2);
    return 0;
}
