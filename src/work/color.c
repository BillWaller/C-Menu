/** @file color.c
   @brief Test program for xterm 256-color to RGB conversion and vice versa.
   @details This program defines two functions: `xterm256_to_rgb` to convert an
   xterm 256-color code to its corresponding RGB color, and `rgb_to_xterm256` to
   convert an RGB color to the nearest xterm 256-color code. The main function
   tests these conversions by iterating through all 256 xterm color codes,
   converting them to RGB, and then converting back to xterm codes, printing the
   results.
   @author Bill Waller
   @date 2025-12
 */

#include <stdint.h>
#include <stdio.h>

/** @struct RGB
   @brief Represents an RGB color with 8-bit components.
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

/** @brief Converts an xterm 256-color code to its corresponding RGB color.
   @param code The xterm 256-color code (0-255).
   @return An RGB struct representing the corresponding color.
    16 color standard palette
   216 color xterm cube
    24 color grayscale
 */
RGB xterm256_to_rgb1(int code) {
    RGB color;
    if (code < 16) {
        /** 16 color standard palette */
        static const RGB standard_colors[16] = {
            {0, 0, 0}, {128, 0, 0}, {0, 128, 0}, {128, 128, 0}, {0, 0, 128}, {128, 0, 128}, {0, 128, 128}, {192, 192, 192}, {128, 128, 128}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};
        color = standard_colors[code];
    } else if (code >= 16 && code <= 231) {
        /** 216 (6x6x6) color xterm cube */
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
        /** 24 color grayscale */
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

int rgb_to_xterm1(unsigned char r, unsigned char g, unsigned char b) {
    if (r == g && g == b) {
        if (r < 8)
            return 16;
        if (r > 248)
            return 231;
        return ((r - 8) / 10) + 232;
    } else {
        /** xterm Color cube */
        int r_index = (r * 5) / 255;
        int g_index = (g * 5) / 255;
        int b_index = (b * 5) / 255;
        return 16 + (36 * r_index) + (6 * g_index) + b_index;
    }
}
/** @brief Converts an RGB color to the nearest xterm 256-color code.
   @param r The red component (0-255).
   @param g The green component (0-255).
   @param b The blue component (0-255).
   @return The nearest xterm 256-color code (0-255). */
int rgb_to_xterm256(unsigned char r, unsigned char g, unsigned char b) {
    if (r == g && g == b) {
        if (r < 8)
            return 16;
        if (r > 248)
            return 231;
        return ((r - 8) / 10) + 232;
    } else {
        /** xterm Color cube */
        int r_index = (r < 45) ? 0 : (r - 60) / 40 + 1;
        int g_index = (g < 45) ? 0 : (g - 60) / 40 + 1;
        int b_index = (b < 45) ? 0 : (b - 60) / 40 + 1;
        return 16 + (36 * r_index) + (6 * g_index) + b_index;
    }
}

// Helper to find the closest 6x6x6 cube index (0 to 5)
int rgb_to_cube_val(int channel) {
    if (channel < 48)
        return 0;
    if (channel < 115)
        return 1;
    return (channel - 35) / 40;
}

// Convert RGB (0-255) to xterm-256 code (0-255)
int rgb_to_xterm(int r, int g, int b) {
    // Check if it's close to grayscale
    if (r == g && g == b) {
        if (r < 8)
            return 16; // Black
        if (r > 248)
            return 231; // White
        // 24 levels of grayscale (232-255)
        return 232 + (r - 8) / 10;
    }

    // Map to 6x6x6 color cube indices (0-5)
    int ir = rgb_to_cube_val(r);
    int ig = rgb_to_cube_val(g);
    int ib = rgb_to_cube_val(b);

    // Base offset for 6x6x6 cube is 16
    return 16 + (36 * ir) + (6 * ig) + ib;
}

RGB xterm_to_rgb(uint8_t idx) {
    // 1. Standard 16 ANSI colors (approximate typical xterm defaults)
    static const uint8_t ansi[16][3] = {
        {0, 0, 0}, {205, 0, 0}, {0, 205, 0}, {205, 205, 0}, {0, 0, 205}, {205, 0, 205}, {0, 205, 205}, {229, 229, 229}, {127, 127, 127}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};
    RGB rgb;

    if (idx < 16) {
        rgb.r = ansi[idx][0];
        rgb.g = ansi[idx][1];
        rgb.b = ansi[idx][2];
    }
    // 2. 6x6x6 RGB cube (indices 16 to 231)
    else if (idx < 232) {
        uint8_t c = idx - 16;
        uint8_t ir = c / 36;
        uint8_t ig = (c / 6) % 6;
        uint8_t ib = c % 6;

        // Convert 0-5 scale to 0-255 scale
        rgb.r = ir ? (ir * 40 + 55) : 0;
        rgb.g = ig ? (ig * 40 + 55) : 0;
        rgb.b = ib ? (ib * 40 + 55) : 0;
    }
    // 3. Grayscale ramp (indices 232 to 255)
    else {
        uint8_t gray = 8 + (idx - 232) * 10;
        rgb.r = gray;
        rgb.g = gray;
        rgb.b = gray;
    }
    return rgb;
}

int main() {
    RGB rgb;
    RGB rgb2;
    uint8_t i, idx, fail = 0;
    i = 0;
    do {
        rgb = xterm_to_rgb(i);
        idx = rgb_to_xterm(rgb.r, rgb.g, rgb.b);
        rgb2 = xterm_to_rgb(idx);
        if (rgb.r != rgb2.r || rgb.g != rgb2.g || rgb.b != rgb2.b) {
            printf("Mismatch for idx %d: (%d, %d, %d) -> idx %d -> (%d, %d, %d)\n",
                   i, rgb.r, rgb.g, rgb.b, idx, rgb2.r, rgb2.g, rgb2.b);
            fail = 1;
        } else {
            printf("idx %-3d (%3d, %3d, %3d) #%02x%02x%02x %-3d (%3d, %3d, %3d) "
                   "#%02x%02x%02x\n",
                   i, rgb.r, rgb.g, rgb.b, rgb.r, rgb.g, rgb.b, idx, rgb2.r, rgb2.g,
                   rgb2.b, rgb2.r, rgb2.g, rgb2.b);
        }
        if (i == 255)
            break;
        i++;
    } while (1);
    if (fail) {
        printf("Test failed.\n");
        return 1;
    }
    return 0;
}
