// decode UTF-8 encoded string to Unicode code points

#include <stdint.h>
#include <stdio.h>

uint32_t utf8_decode(const unsigned char **s) {
    uint32_t cp = 0;
    const unsigned char *p = *s;

    if (p[0] < 0x80) { // 1-byte ascii
        cp = p[0];
        p += 1;
    } else if ((p[0] & 0xe0) == 0xc0) { // 2-byte
        cp = ((p[0] & 0x1f) << 6) | (p[1] & 0x3f);
        p += 2;
    } else if ((p[0] & 0xf0) == 0xe0) { // 3-byte
        cp = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
        p += 3;
    } else if ((p[0] & 0xf8) == 0xf0) { // 4-byte
        cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3f) << 12) |
             ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
        p += 4;
    } else {
        cp = 0xfffd; // invalid sequence
        p += 1;
    }

    *s = p;
    return cp;
}

int main() {
    const unsigned char *str = (const unsigned char *)"🙍👴👵♂♀";

    while (*str) {
        uint32_t cp = utf8_decode(&str);
        if (cp > 0xFFFF) {
            printf("U+%06X\n", cp);
            continue;
        } else {
            printf("U+%06X < 0xFFFF\n", cp);
            continue;
        }
    }
    return 0;
}
