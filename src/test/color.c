/** @file color.c
 * @brief Test program for xterm 256-color to RGB conversion and vice versa.
 * @details This program defines two functions: `xterm256_to_rgb` to convert an
 * xterm 256-color code to its corresponding RGB color, and `rgb_to_xterm256` to
 * convert an RGB color to the nearest xterm 256-color code. The main function
 * tests these conversions by iterating through all 256 xterm color codes,
 * converting them to RGB, and then converting back to xterm codes, printing the
 * results.
 * @author OpenAI's ChatGPT
 * @date 2024-06
 */

#include <stdio.h>

/** @struct RGB
 * @brief Represents an RGB color with 8-bit components.
 */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;

/** @brief Converts an xterm 256-color code to its corresponding RGB color.
 * @param code The xterm 256-color code (0-255).
 * @return An RGB struct representing the corresponding color.
 */
RGB xterm256_to_rgb(int code) {
    RGB color;
    if (code < 16) {
        // Standard colors
        static const RGB standard_colors[16] = {
            {0, 0, 0},       {128, 0, 0},   {0, 128, 0},   {128, 128, 0},
            {0, 0, 128},     {128, 0, 128}, {0, 128, 128}, {192, 192, 192},
            {128, 128, 128}, {255, 0, 0},   {0, 255, 0},   {255, 255, 0},
            {0, 0, 255},     {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};
        color = standard_colors[code];
    } else if (code >= 16 && code <= 231) {
        // 6x6x6 color cube
        int index = code - 16;
        int r = (index / 36) % 6;
        int g = (index / 6) % 6;
        int b = index % 6;
        if (r > 0)
            color.r = r * 40 + 55;
        else
            color.r = 0;
        if (g > 0)
            color.g = g * 40 + 55;
        else
            color.g = 0;
        if (b > 0)
            color.b = b * 40 + 55;
        else
            color.b = 0;
    } else if (code >= 232 && code <= 255) {
        // Grayscale colors
        int gray = (code - 232) * 10 + 8;
        color.r = gray;
        color.g = gray;
        color.b = gray;
    } else {
        // Invalid code
        color.r = color.g = color.b = 0;
    }
    return color;
}

/** @brief Converts an RGB color to the nearest xterm 256-color code.
 * @param r The red component (0-255).
 * @param g The green component (0-255).
 * @param b The blue component (0-255).
 * @return The nearest xterm 256-color code (0-255).
 */
int rgb_to_xterm256(unsigned char r, unsigned char g, unsigned char b) {
    if (r == g && g == b) {
        if (r < 8)
            return 16;
        if (r > 248)
            return 231;
        return ((r - 8) / 10) + 232;
    } else {
        // Color cube
        int r_index = (r < 45) ? 0 : (r - 60) / 40 + 1;
        int g_index = (g < 45) ? 0 : (g - 60) / 40 + 1;
        int b_index = (b < 45) ? 0 : (b - 60) / 40 + 1;
        return 16 + (36 * r_index) + (6 * g_index) + b_index;
    }
}
int main(int argc, char *argv[]) {
    RGB rgb;
    RGB rgb2;
    int i, idx;
    for (i = 0; i < 256; i++) {
        rgb = xterm256_to_rgb(i);
        idx = rgb_to_xterm256(rgb.r, rgb.g, rgb.b);
        rgb2 = xterm256_to_rgb(idx);
        printf("idx %-3d (%3d, %3d, %3d) #%02x%02x%02x %-3d (%3d, %3d, %3d) "
               "#%02x%02x%02x\n",
               i, rgb.r, rgb.g, rgb.b, rgb.r, rgb.g, rgb.b, idx, rgb2.r, rgb2.g,
               rgb2.b, rgb2.r, rgb2.g, rgb2.b);
    }
    return 0;
}
