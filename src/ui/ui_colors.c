typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

typedef struct {
    union {
        struct {
            uint8_t a; // alpha    LSB (Little Endian order)
            uint8_t b; // blue
            uint8_t g; // green
            uint8_t r; // red      MSB (Little Endian order)
        };
        struct {
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
