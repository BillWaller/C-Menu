#ifndef UI_NOTCURSES_INTERNAL_H
#define UI_NOTCURSES_INTERNAL_H 1

/** @file ui_notcurses_internal.h
   @ingroup ui_notcurses
   @brief Internal header for the NotCurses UI backend.

   Defines the concrete (non-opaque) layouts of UiRuntime and UiSurface for
   the NotCurses backend.  Must only be included by NotCurses backend source
   files — never by application code.
*/

#include "ui_backend.h"
#include <notcurses/notcurses.h>
#include <stdbool.h>
#include <stdint.h>

#define XLEN 256
#define MAXLEN 256
#define MAXPLANE 30
#define MAXSFC 30
#define U_VE L'\x2502' /**< vertical line */

typedef struct notcurses NotCurses;
typedef struct ncplane NcPlane;
typedef struct notcurses_options NotCursesOptions;
typedef struct ncplane_options NcPlaneOptions;

/** @struct UiRuntime
   @ingroup ui_notcurses
   @brief Runtime state for the NotCurses backend.

   Wraps the @c struct notcurses* context created by notcurses_init() and
   carries per-session configuration flags.
*/
struct UiRuntime {
    struct notcurses *nc; /**< root NotCurses context */
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    int lines;
    int cols;
};

/** @struct UiSurface
   @ingroup ui_notcurses
   @brief A drawable surface in the NotCurses backend.

   Each UiSurface wraps a @c struct ncplane*.  Visibility is simulated by
   moving the plane off-screen when hidden and restoring it when shown.
*/

struct UiSurface {
    union {
        struct {
            struct NcPlane *box;
            struct NcPlane *win;
            struct NcPlane *win2;
            struct NcPlane *lnno;
            struct NcPlane *cmdln;
            struct NcPlane *pad;
            struct NcPlane *plane1;
            struct NcPlane *plane2;
        };
        struct {
            struct NcPlane *mplane[8];
        };
    };
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int y;
    int x;
    int lines;
    int cols;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};

// struct NcCell {
//     uint32_t gcluster;         // 0  3   4  little endian EGC
//     uint8_t gcluster_backstop; // 4  1   5  (8 bits of zero)
//     uint8_t width;             // 5  1   6  (8 bits of EGC column width)
//     uint16_t stylemask;        // 6  2   8  (16 bits of NCSTYLE_* attributes)
//     uint64_t channels;         // 8  8  16  (fg/bg, alpha, palette index,
//     uadrant)
// };

union UiChannels {
    struct {
        struct {
            uint8_t f_b, f_g, f_r, f_a;
        };
        struct {
            uint8_t b_b, b_g, b_r, b_a;
        };
    };
    struct {
        uint32_t chan[2];
    };
    struct {
        uint64_t chans;
    };
};

union UiGCluster {
    struct {
        uint8_t c[4]; // 4-bytes for UTF-8
        uint8_t t;    // 1-byte terminator
    };
    uint8_t gc[5];
};

struct UiStyle {
    union UiGCluster gclust;   // 0 -  4   little endian EGC
    uint8_t width;             // 5 -  5   (8 bits of EGC column width)
    uint16_t stylemask;        // 6 -  7   2-bytes
    union UiChannels channels; // 8 - 15   8 bytes
};

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
/* Internal style helpers */

int ui_bkgrnd(struct UiSurface *s, int w, const struct UiStyle *style, const char *c);
uint64_t ui_notcurses_channels_from_style(const struct UiStyle *style);
uint32_t ui_notcurses_attrs_from_style(const struct UiStyle *style);
struct NcPlane *ncplane_clicked(struct NcPlane *pile_member, ncinput *ni);

#endif
