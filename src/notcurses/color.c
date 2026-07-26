#include <inttypes.h>

typedef struct {
    uint8_t r, g, b;
} RGB;

RGB hex_clr_str_to_rgb(char *s);

RGB hex_clr_str_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02hhx%02hhx%02hhx", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
