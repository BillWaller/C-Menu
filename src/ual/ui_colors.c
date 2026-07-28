/**
 * @file ui_colors.c
 * @brief UI color definitions and utilities.
 */

typedef struct {
    int r, g, b;
} RGB;

typedef struct {
    union {
        struct RGB8 {
            uint8_t a, b, g, r; // alpha LSB (Little Endian order)
        };
        struct RGB32 {
            uint32_t rgba; // 0xRRGGBBAA   red, green, blue, alpha
        };
    };
    uint32_t idx;
} UiColor;

typedef struct {
    UiColor fg;
    UiColor bg;
    uint32_t idx;
} UiColorPair;

UiColor *ui_color;
UiColorPair *ui_color_pair;
RGB *ui_color_to_rgb(UiColor *);
