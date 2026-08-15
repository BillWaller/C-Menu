// ui_test_channels.c - Test the UiChannels union and its channel values
// This might have some endianness issues, but it should be fine for now. The union is used to access the same data in different ways, and the channel values are stored in a specific order. The test checks if the channel values are correct after setting the fb value.

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define _GNU_SOURCE

typedef union {
    struct {
        union {
            struct {
                uint8_t b_b, b_g, b_r, b_a;
            };
            uint32_t bargb;
        };
        union {
            struct {
                uint8_t f_b, f_g, f_r, f_a;
            };
            uint32_t fargb;
        };
    };
    uint64_t fb;
} UiChannels;

void out8(uint8_t x1, uint8_t x2) {
    printf("got: 0x%" PRIx8 ", expected: 0x%" PRIx8 ", %s\n",
           x1, x2, (x1 == x2) ? "PASS" : "FAIL");
}
void out32(uint32_t w1, uint32_t w2) {
    printf("got: 0x%" PRIx32 ", expected 0x%" PRIx32 ", %s\n",
           w1, w2, (w1 == w2) ? "PASS" : "FAIL");
}
void out64(uint64_t z1, uint64_t z2) {
    printf("got: 0x%" PRIx64 ", expected: 0x%" PRIx64 ", %s\n",
           z1, z2, (z1 == z2) ? "PASS" : "FAIL");
}

#define out(A, B) _Generic((A, B), \
    uint8_t: out8,                 \
    uint32_t: out32,               \
    uint64_t: out64)(A, B)

int main() {
    UiChannels channels;
    channels.fb = 0x40efcdab40896745;
    out(channels.f_a, (uint8_t)0x40);
    out(channels.f_r, (uint8_t)0xef);
    out(channels.f_g, (uint8_t)0xcd);
    out(channels.f_b, (uint8_t)0xab);
    out(channels.b_a, (uint8_t)0x40);
    out(channels.b_r, (uint8_t)0x89);
    out(channels.b_g, (uint8_t)0x67);
    out(channels.b_b, (uint8_t)0x45);
    out(channels.fargb, (uint32_t)0x40efcdab);
    out(channels.bargb, (uint32_t)0x40896745);
    out(channels.fb, (uint64_t)0x40efcdab40896745);
    if (channels.f_r == 0xef && channels.f_g == 0xcd && channels.f_b == 0xab &&
        channels.f_a == 0x40 && channels.b_r == 0x89 && channels.b_g == 0x67 &&
        channels.b_b == 0x45 && channels.b_a == 0x40) {
        printf("Success: All channel values are correct.\n");
        return 0;
    } else
        printf("Error: Channel values are incorrect.\n");
    return -1;
}
