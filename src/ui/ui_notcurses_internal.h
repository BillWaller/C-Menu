#ifndef UI_NOTCURSES_INTERNAL_H
#define UI_NOTCURSES_INTERNAL_H 1

/** @file ui_notcurses_internal.h
   @ingroup ui_notcurses
   @brief Internal header for the NotCurses UI backend.

   Defines the concrete (non-opaque) layouts of UiRuntime and UiSurface for
   the NotCurses backend.  Must only be included by NotCurses backend source
   files — never by application code.
*/

#define _XOPEN_SOURCE_EXTENDED 1
#include <notcurses/notcurses.h>
#include <stdbool.h>
#include <stdint.h>

#define XLEN 256
#define MAXLEN 256
#define MAXPLANE 30
#define MAXSFC 30
#define U_VE L'\x2502' /**< vertical line */
typedef struct notcurses NotCurses;
typedef struct notcurses_options NotCursesOptions;

#define NC_COLORS 512
#define NC_PAIRS 512

int LINES;
int COLS;

/** @struct UiRuntime
   @ingroup ui_notcurses
   @brief Runtime state for the NotCurses backend.

   Wraps the @c struct notcurses* context created by notcurses_init() and
   carries per-session configuration flags.
*/
struct UiRuntime {
    struct notcurses *nc;
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
            struct ncplane *box;
            struct ncplane *win;
            struct ncplane *win2;
            struct ncplane *lnno;
            struct ncplane *cmdln;
            struct ncplane *pad;
            struct ncplane *plane1;
            struct ncplane *plane2;
        };
        struct {
            struct ncplane *mplane[8];
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

struct UiColor {
    uint8_t r, g, b;
};

struct UiColorPair {
    int fg;
    int bg;
};

typedef struct {
    union {
        struct {
            uint8_t b, g, r;
        };
        uint32_t rgb;
    };
} RGB;

union UiChannels {
    union {
        struct {
            union {
                struct {
                    uint8_t f_b, f_g, f_r, f_a;
                };
                uint32_t fargb;
            };
            union {
                struct {
                    uint8_t b_b, b_g, b_r, b_a;
                };
                uint32_t bargb;
            };
        };
        uint64_t chs;
    };
};

struct UiStyle {
    union {
        struct {
            wchar_t gcluster[4];       // 4-bytes for UTF-8
            uint8_t gcluster_backstop; // 1-byte terminator
        };
        uint8_t wstr[5];
    };
    uint8_t width;             // 5 -  5   (8 bits of EGC column width)
    uint16_t stylemask;        // 6 -  7   2-bytes
    union UiChannels channels; // 8 - 15   8 bytes
};

struct UiCell {
    union {
        struct {
            uint8_t gclus[4]; // 4-bytes for UTF-8
            uint8_t bs;       // 1-byte terminator
        };
        struct {
            wchar_t wst0[4]; // 4-bytes for UTF-8
            wchar_t wst1[1]; // 1-byte backstop
        };
    };
    uint8_t width;             // 5 -  5   (8 bits of EGC column width)
    uint16_t stylemask;        // 6 -  7   2-bytes
    union UiChannels channels; // 8 - 15   8 bytes
};

#define UISTYLE_MASK NCSTYLE_MASK
#define UISTYLE_NORMAL NCSTYLE_NONE
#define UISTYLE_STANDOUT NCSTYLE_UNDERLINE
#define UISTYLE_UNDERLINE NCSTYLE_UNDERLINE
#define UISTYLE_REVERSE NCSTYLE_UNDERLINE
#define UISTYLE_BLINK NCSTYLE_UNDERLINE
#define UISTYLE_DIM NCSTYLE_UNDERLINE
#define UISTYLE_BOLD NCSTYLE_BOLD
#define UISTYLE_ALTCHARSET NCSTYLE_UNDERLINE
#define UISTYLE_INVIS NCSTYLE_UNDERLINE
#define UISTYLE_STRUCK NCSTYLE_STRUCK
#define UISTYLE_PROTECT NCSTYLE_UNDERLINE
#define UISTYLE_UNDERCURL NCSTYLE_UNDERCURL
#define UISTYLE_NONE NCSTYLE_NONE
#define UISTYLE_ITALIC NCSTYLE_UNDERLINE

#define WA_ATTRIBUTES NCSTYLE_MASK
#define WA_NORMAL NCSTYLE_NONE
#define WA_STANDOUT NCSTYLE_UNDERLINE
#define WA_UNDERLINE NCSTYLE_UNDERLINE
#define WA_REVERSE NCSTYLE_UNDERLINE
#define WA_BLINK NCSTYLE_UNDERLINE
#define WA_DIM NCSTYLE_UNDERLINE
#define WA_BOLD NCSTYLE_BOLD
#define WA_ALTCHARSET NCSTYLE_UNDERLINE
#define WA_INVIS NCSTYLE_UNDERLINE
#define WA_STRUCK NCSTYLE_STRUCK
#define WA_PROTECT NCSTYLE_UNDERLINE
#define WA_UNDERCURL NCSTYLE_UNDERCURL
#define WA_ITALIC NCSTYLE_UNDERLINE
#define WA_NONE NCSTYLE_NONE

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
struct NcPlane *ncplane_clicked(struct ncplane *pile_member, ncinput *ni);

#endif
