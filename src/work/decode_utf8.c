#include <stdio.h>
#include <stdint.h>

// Decodes a single UTF-8 sequence into a codepoint and returns bytes consumed
int utf8_to_codepoint(const uint8_t *str, uint32_t *out_codepoint) {
    if (!str || !out_codepoint) return 0;

    uint8_t b1 = str[0];

    // 1-byte ASCII (0xxxxxxx)
    if (b1 < 0x80) {
        *out_codepoint = b1;
        return 1;
    }
    // 2-byte sequence (110xxxxx 10xxxxxx)
    else if ((b1 & 0xE0) == 0xC0) {
        *out_codepoint = ((b1 & 0x1F) << 6) | (str[1] & 0x3F);
        return 2;
    }
    // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
    else if ((b1 & 0xF0) == 0xE0) {
        *out_codepoint = ((b1 & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return 3;
    }
    // 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
    else if ((b1 & 0xF8) == 0xF0) {
        *out_codepoint = ((b1 & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        return 4;
    }

    return -1; // Invalid UTF-8 starting byte
}

int main() {
    const uint8_t utf8_char[] = {0xE2, 0x82, 0xAC}; // '€' in UTF-8
    uint32_t codepoint = 0;

    int bytes = utf8_to_codepoint(utf8_char, &codepoint);
    if (bytes > 0) {
        printf("Unicode Codepoint: U+%04X (Parsed %d bytes)\n", codepoint, bytes);
    } else {
        printf("Failed to decode UTF-8\n");
    }
    return 0;
}

