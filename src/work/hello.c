#include <inttypes.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

typedef struct {
    union {
        uint32_t u32;
        wchar_t u16[2];
        uint8_t u8[4];
        char c[4];
    };
    uint8_t backstop;
    uint8_t width; // 5 -  5   (8 bits of EGC column width)
} GCluster;

int main() {
    setlocale(LC_ALL, "");
    GCluster g0, g1, g2, g3;

    g0.u32 = L'┤';
    g1.u32 = L'├';
    g2.u32 = 0x2524;
    g3.u32 = 0x251c;

    printf("%08x %08x %08x %08x\n", g0.u32, g1.u32, g2.u32, g3.u32);
    //   00002524 0000251c 00002524 0000251c
    printf("%s %s %s %s\n", g0.c, g1.c, g2.c, g3.c);
    //              $% %  $% %

    // Here, we are assigning the wchar_t values directly to the u16 array
    wchar_t c0 = L'┤';
    wchar_t c1 = L'├';
    g2.u16[0] = c0;
    g3.u16[0] = c1;
    printf("%08x %08x %08x %08x\n", g0.u32, g1.u32, g2.u32, g3.u32);
    // 00002524 0000251c 00002524 0000251c
    //
    // We can probably use these values with Notcurses. They are not UTF-8
    // encoded, but they are valid Unicode codepoints. The documentation seems
    // to indicate that this encoding will work with ncplane_putwegc().
    //
    // We did that and then used ncplane_at_yx_cell() to fetch the values from
    // the plane. We got 0xa494e2 and 0x9c94e2. Those are the correct multibyte
    // UTF-8 encodings for our characters. So we need to use those values
    // instead of the wchar_t values with the non-wide-character functions of
    // Notcurses.
    //
    // We used strcpy to copy the wchar_t values into the char arrays. Low and
    // behold, we got the correct multibyte UTF-8 encodings in the char arrays.
    strcpy(g2.c, "┤"); // -> 0xa494e2
    strcpy(g3.c, "├"); // -> 0x9c94e2
    printf("%08x %08x %08x %08x\n", g0.u32, g1.u32, g2.u32, g3.u32);
    printf("%s %s %s %s\n", g0.c, g1.c, g2.c, g3.c);
    //      $%      %       ┤        ├
    g0.u32 = L'┤';
    strcpy(g2.c, "┤"); // -> 0xa494e2
    printf("%x %x %s\n", g0.u32, g2.u32, g2.c);
    //              2524 a494e2 ┤
    g1.u32 = L'├';
    strcpy(g3.c, "├"); // -> 0x9c94e2
    printf("%x %x %s\n", g1.u32, g3.u32, g3.c);
    //              251c 9c94e2 ├
    //   00002524 0000251c 00a494e2 009c94e2
    //
    // Wee doggies! Would you look at that? Just like magic, we have the correct
    // UTF-8 encoded values in the char arrays. Can we use these with Notcurses?
    // Well, yes, we can use the char arrays with Notcurses, but we cannot use
    // the wchar_t values. The wchar_t values are not UTF-8 encoded. They are just
    // Unicode codepoints. But, we don't really care, so long as we have a way
    // to get the results we want, either from Unicode codepoints or the character
    // glyphs. I wonder if iconv can convert the wchar_t values to UTF-8 encoded
    // char arrays. We can try that next.
    //
    printf("\n\nFrom Ode to a Mouse\n");
    printf("\nBut Mousie, thou art no thy-lane,\n");
    printf("In proving foresight may be vain:\n");
    printf("The best laid schemes o’ Mice an’ Men\n");
    printf("        Gang aft agley,\n");
    printf("An’ lea’e us nought but grief an’ pain,\n");
    printf("        For promis’d joy!\n\n");
    printf("         by Robert Burns\n\n");

    return 0;
}
