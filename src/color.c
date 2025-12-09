// color.c
//
#include <stdio.h>

#define uint unsigned int;

typedef struct {
    int red;
    int green;
    int blue;
} RGB;

void rgb_idx_to_rgb(int, int *red, int *green, int *blue);

int main(int argc, char **argv) {
    char color_str[7] = "ff0000";
    int rgb_idx;
    RGB *rgb;
    int red, green, blue;

    sscanf(color_str, "%2x%2x%2x\n", &red, &green, &blue);

    printf("red=%d, green = %d, blue = %d\n", red, green, blue);

    rgb_idx = 196;
    rgb_idx_to_rgb(rgb_idx, &red, &green, &blue);
    printf("rgb_idx=%d, red=%d, green=%d, blue=%d\n", rgb_idx, red, green,
           blue);
}

void rgb_idx_to_rgb(int rgb_idx, int *red, int *green, int *blue) {
    int rgb_tbl[] = {0, 95, 135, 175, 215, 255};

    rgb_idx = 196;
    int o;
    o = 196 - 16;
    *red = o / 36;
    *green = (o % 36) / 6;
    *blue = (o % 36) % 6;
    *red = rgb_tbl[*red];
    *green = rgb_tbl[*green];
    *blue = rgb_tbl[*blue];
}
