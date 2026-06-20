#include <notcurses/notcurses.h>

// Helper to cleanly pack standard RGB colors into a full background/foreground channel mask
static inline uint64_t make_channels(uint32_t fg_rgb, uint32_t bg_rgb) {
    uint64_t channels = 0;
    // Set foreground RGB
    channels = ncchannels_set_fchannel(&channels, fg_rgb);
    // Set background RGB
    channels = ncchannels_set_bchannel(&channels, bg_rgb);
    return channels;
}

// Replicating your ncurses types into clean 24-bit equivalents
// Colors format hex: 0xRRGGBB
#define CC_BOX_CHANNELS make_channels(0x888888, 0x111111) // Grey borders, dark grey background
#define CC_NT_CHANNELS make_channels(0xFFFFFF, 0x000000)  // White text, pure black background
#define CC_LN_CHANNELS make_channels(0x00FF00, 0x222222)  // Green numbers, soft grey track background
