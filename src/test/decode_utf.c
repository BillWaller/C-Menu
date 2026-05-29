#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

/** @brief Decodes a UTF-8 byte stream into a wide character code point.
    @returns the number of bytes read (1 to 4), or 0 on decoding error. */
int decode_utf8(const uint8_t *buffer, wchar_t *out_cp) {
    if (buffer[0] == 0) {
        return 0; // Null terminator or invalid start
    }
    // 1-byte sequence: 0xxxxxxx
    if ((buffer[0] & 0x80) == 0x00) {
        *out_cp = buffer[0];
        return 1;
    }
    // 2-byte sequence: 110xxxxx 10xxxxxx
    else if ((buffer[0] & 0xE0) == 0xC0) {
        if ((buffer[1] & 0xC0) != 0x80)
            return 0; // Malformed continuation byte
        *out_cp = ((buffer[0] & 0x1F) << 6) |
                  (buffer[1] & 0x3F);
        return 2;
    }
    // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
    else if ((buffer[0] & 0xF0) == 0xE0) {
        if ((buffer[1] & 0xC0) != 0x80 ||
            (buffer[2] & 0xC0) != 0x80)
            return 0;
        *out_cp = ((buffer[0] & 0x0F) << 12) |
                  ((buffer[1] & 0x3F) << 6) |
                  (buffer[2] & 0x3F);
        return 3;
    }
    // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if ((buffer[0] & 0xF8) == 0xF0) {
        if ((buffer[1] & 0xC0) != 0x80 ||
            (buffer[2] & 0xC0) != 0x80 ||
            (buffer[3] & 0xC0) != 0x80)
            return 0;
        *out_cp = ((buffer[0] & 0x07) << 18) |
                  ((buffer[1] & 0x3F) << 12) |
                  ((buffer[2] & 0x3F) << 6) |
                  (buffer[3] & 0x3F);
        return 4;
    }
    return 0; // Malformed or unsupported UTF-8 prefix
}

int main() {
    // The UTF-8 hex bytes for U+1F600
    uint8_t utf8_bytes[] = {0xF0, 0x9F, 0x98, 0x80};
    wchar_t decoded_cp = 0;

    int bytes_read = decode_utf8(utf8_bytes, &decoded_cp);

    if (bytes_read > 0) {
        printf("Bytes processed: %d\n", bytes_read);
        printf("Decoded Code Point: U+%04X\n", (unsigned int)decoded_cp);
    } else {
        printf("Error: Invalid UTF-8 sequence.\n");
    }

    return 0;
}
