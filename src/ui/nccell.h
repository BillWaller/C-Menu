#include <stdint.h>

typedef struct nccell {
    uint32_t gcluster;         // 0   4B → 4B little endian EGC
    uint8_t gcluster_backstop; // 4   1B → 5B (8 bits of zero)
    uint8_t width;             // 5   1B → 6B (8 bits of EGC column width)
    uint16_t stylemask;        // 6   2B → 8B (16 bits of NCSTYLE_* attributes)
    uint64_t channels;         // 8   8B → 16B (fg/bg, alpha, palette index, quadrant)
} nccell;

#define NCALPHA_HIGHCONTRAST 0x30000000ull
#define NCALPHA_TRANSPARENT 0x20000000ull
#define NCALPHA_BLEND 0x10000000ull
#define NCALPHA_OPAQUE 0x00000000ull

#define NCCHANNELS_FOREGROUND_ALPHA_MASK 0x3000000000000000ull
#define NCCHANNELS_FOREGROUND_DEFAULT 0x4000000000000000ull
#define NCCHANNELS_FOREGROUND_QUADRANT_UL 0x8000000000000000ull

#define NCCHANNELS_FOREGROUND_QUADRANT_LR 0x0100000000000000ull
#define NCCHANNELS_FOREGROUND_QUADRANT_LL 0x0200000000000000ull
#define NCCHANNELS_FOREGROUND_QUADRANT_UR 0x0400000000000000ull
#define NCCHANNELS_FOREGROUND_PALETTE 0x0800000000000000ull

#define NCCHANNELS_FOREGROUND_MASK 0x00ffffff00000000ull

#define NCCHANNELS_BACKGROUND_ALPHA_MASK 0x0000000030000000ull
#define NCCHANNELS_BACKGROUND_DEFAULT 0x0000000040000000ull
#define NCCHANNELS_RESERVED1 0x0000000080000000ull

#define NCCHANNELS_RESERVED2 0x0000000007000000ull
#define NCCHANNELS_BACKGROUND_PALETTE 0x0000000008000000ull

#define NCCHANNELS_BACKGROUND_MASK 0x0000000000ffffffull

// (channels & 0x3000000000000000ull): foreground alpha (2 bits)
// (channels & 0x4000000000000000ull): foreground is *not* "default color"
// (channels & 0x8000000000000000ull): blitted to upper-left quadrant
// (channels & 0x0100000000000000ull): blitted to lower-right quadrant
// (channels & 0x0200000000000000ull): blitted to lower-left quadrant
// (channels & 0x0400000000000000ull): blitted to upper-right quadrant
// (channels & 0x0800000000000000ull): foreground uses palette index
// (channels & 0x00ffffff00000000ull): foreground in 3x8 RGB (rrggbb)
// (channels & 0x0000000080000000ull): reserved, must be 0
// (channels & 0x0000000040000000ull): background is *not* "default color"
// (channels & 0x0000000030000000ull): background alpha (2 bits)
// (channels & 0x0000000008000000ull): background uses palette index
// (channels & 0x0000000007000000ull): reserved, must be 0
// (channels & 0x0000000000ffffffull): background in 3x8 RGB (rrggbb)
