#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

/** @brief Converts a wide character code point to UTF-8 bytes.
    @returns the number of bytes written (1 to 4), or 0 on error.
    utf8cp_to_str;
*/
int wccp_to_str(wchar_t cp, uint8_t *buffer) {
    if (cp <= 0x7F) {
        // 1-byte sequence: 0xxxxxxx
        buffer[0] = (uint8_t)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        buffer[0] = (uint8_t)(0xC0 | ((cp >> 6) & 0x1F));
        buffer[1] = (uint8_t)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp <= 0xFFFF) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        buffer[0] = (uint8_t)(0xE0 | ((cp >> 12) & 0x0F));
        buffer[1] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        buffer[2] = (uint8_t)(0x80 | (cp & 0x3F));
        return 3;
    } else if (cp <= 0x10FFFF) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        buffer[0] = (uint8_t)(0xF0 | ((cp >> 18) & 0x07));
        buffer[1] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
        buffer[2] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        buffer[3] = (uint8_t)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0; // Invalid Unicode code point
}

int main() {
    wchar_t smiley = L'\U0001f600';
    uint8_t utf_s[4] = {0};

    int bytes_written = wccp_to_str(smiley, utf_s);
    printf("Smiley: %s\n", utf_s);
    printf("Code point: U+%04X\n", (unsigned int)smiley);
    printf("UTF-8 Hex:  0x");
    for (int i = 0; i < bytes_written; i++) {
        printf("%02X", utf_s[i]);
    }
    printf("\n");

    wchar_t underscore = L'\U0000ff3f';
    uint8_t utf_s2[4] = {0};
    // convert wide character code point to UTF-8 bytes
    bytes_written = wccp_to_str(underscore, utf_s2);
    printf("underscore: %s\n", utf_s2);
    printf("Code point: U+%04X\n", (unsigned int)underscore);
    printf("UTF-8 Hex:  0x");
    for (int i = 0; i < bytes_written; i++) {
        printf("%02X", utf_s2[i]);
    }
    printf("\n");

    memcpy(utf_s3, "＿", 3); // copy the UTF-8 bytes for '＿' (U+FF3F) into utf_s3
    printf("underscore: %s\n", utf_s3);
    printf("Code point: U+%04X\n", (unsigned int)underscore);
    printf("UTF-8 Hex:  0x");
    for (int i = 0; i < bytes_written; i++) {
        printf("%02X", utf_s2[i]);
    }
    printf("\n");

    return 0;
}
