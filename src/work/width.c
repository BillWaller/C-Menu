#define _GNU_SOURCE

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int main() {
    // Set locale so wcwidth/wcswidth know how to evaluate UTF-8 characters
    setlocale(LC_CTYPE, "");

    // Example symbol: Japanese Kanji, which requires 2 character cells
    // wchar_t test_char = 0x6C34; // '水'
    wchar_t test_char = 0xFF3F; // '＿'
    int width = wcwidth(test_char);
    printf("The character '%lc' requires %d columns.\n", test_char, width);

    // To measure an entire wide string
    wchar_t test_str[] = L"Hello, ＿!";
    int str_width = wcswidth(test_str, wcslen(test_str));

    printf("The string \"%ls\" requires %d columns.\n", test_str, str_width);

    return 0;
}
