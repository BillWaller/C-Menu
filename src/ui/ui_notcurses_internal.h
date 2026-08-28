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

#define KEY_DOWN NCKEY_DOWN
#define KEY_UP NCKEY_UP
#define KEY_LEFT NCKEY_LEFT
#define KEY_RIGHT NCKEY_RIGHT
#define KEY_HOME NCKEY_HOME
#define KEY_BACKSPACE NCKEY_BACKSPACE
#define KEY_F0 NCKEY_F01 1
#define KEY_F01 NCKEY_F01
#define KEY_F02 NCKEY_F02
#define KEY_F03 NCKEY_F03
#define KEY_F04 NCKEY_F04
#define KEY_F05 NCKEY_F05
#define KEY_F06 NCKEY_F06
#define KEY_F07 NCKEY_F07
#define KEY_F08 NCKEY_F08
#define KEY_F09 NCKEY_F09
#define KEY_F10 NCKEY_F10
#define KEY_F11 NCKEY_F11
#define KEY_F12 NCKEY_F12
#define KEY_F13 NCKEY_F13
#define KEY_DC NCKEY_DEL
#define KEY_IC NCKEY_INS
#define KEY_NPAGE NCKEY_PGDOWN
#define KEY_PPAGE NCKEY_PGUP
#define KEY_ENTER NCKEY_ENTER
#define KEY_RESIZE NCKEY_RESIZE
#define KEY_END NCKEY_END
#define KEY_BREAK NCKEY_F20
#define KEY_MOUSE NCKEY_BUTTON1
#define KEY_RESIZE NCKEY_RESIZE
#define KEY_SRIGHT NCKEY_F11
#define KEY_SLEFT NCKEY_F12
#define KEY_PRINT NCKEY_PRINT
#define KEY_CATAB NCKEY_F13
#define KEY_LL NCKEY_F14
#define KEY_ALTF09 NCKEY_F21

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
    char border_style;
    unsigned int lines;
    unsigned int cols;
    FILE *tty_fp;
};

/** @struct UiSurface
   @ingroup ui_notcurses
   @brief A drawable surface in the NotCurses backend.

   Each UiSurface wraps a @c struct ncplane*.  Visibility is simulated by
   moving the plane off-screen when hidden and restoring it when shown.
*/

/** @enum sub_surface
   @ingroup ui_notcurses
   @brief Identifiers for planes in a UiSurface
   These identifiers are used to index into the array of ncplanes that compose a UiSurface. They are not essential to the functioning of the UiSurface planes, but merely a convenience. You can just use index numbers to identify the planes.
   The first six identifiers are named based on C-Menu's particular layouts of planes within UiSurfaces, and the eighth, (idx 7), named SUB_SFC_MAX, is used as an indicator of the array size when allocating and freeing UiSurface planes and other UiSurface management tasks. It serves as a sentinel value to indicate that a UiSurface has no more planes. SUB_SFC_MAX is available for use as a plane identifier.
   If you are creating your own surfaces, you will probably want to add a custom enum with your own identifiers and use those to index into the ncplane array of your UiSurfaces. You may add more planes. It is convenient to use SUB_SFC_MAX as the last plane, but you may also define SUB_SFC_MAX outside the enum to accomplish the same effect.
*/

#define SFC_MAX 30

typedef enum SubSurface {
    BOX,
    WIN,
    WIN2,
    LNNO,
    CMDLN,
    PAD,
    WIN3,
    SUB_SFC_MAX
} sub_surface;

struct UiSurfaceMeta {
    unsigned int y;
    unsigned int x;
    unsigned int lines;
    unsigned int cols;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};

struct UiSurface {
    union {
        struct { // DEPRECATED - will be removed
            struct ncplane *box;
            struct ncplane *win;
            struct ncplane *win2;
            struct ncplane *lnno;
            struct ncplane *cmdln;
            struct ncplane *pad;
            struct ncplane *plane1;
            struct ncplane *plane2;
        }; // END DEPRECATION
        struct {
            struct ncplane *mplane[SUB_SFC_MAX];
        };
    };
    struct UiSurfaceMeta meta[SUB_SFC_MAX];
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int sfc_idx;
    int sub_cnt;
};

// struct NcCell {
//     uint32_t gcluster;         // 0  3   4  little endian EGC
//     uint8_t gcluster_backstop; // 4  1   5  (8 bits of zero)
//     uint8_t width;             // 5  1   6  (8 bits of EGC column width)
//     uint16_t stylemask;        // 6  2   8  (16 bits of NCSTYLE_* attributes)
//     uint64_t channels;         // 8  8  16  (fg/bg, alpha, palette index,
//     uadrant)
// };

typedef struct {
    union {
        struct {
            uint8_t b, g, r, a;
        };
        uint32_t color;
    };
} RGB;

union UiChannels {
    struct {
        union {
            struct {
                uint8_t b_b, b_g, b_r, b_a;
            };
            uint32_t bargb;
        };
        union {
            struct {
                uint8_t f_b, f_g, f_r, f_a;
            };
            uint32_t fargb;
        };
    };
    uint64_t fb;
};

struct UiCell {
    union {
        uint32_t gcluster;
        uint32_t u32;
        wchar_t u16[2];
        uint8_t u8[4];
        char c[4];
    };
    char gcluster_backstop;
    uint8_t width;
    uint16_t stylemask;
    uint32_t chhannels;
    union {
        struct {
            union {
                struct {
                    uint8_t b_b, b_g, b_r, b_a;
                };
                uint32_t bargb;
            };
            union {
                struct {
                    uint8_t f_b, f_g, f_r, f_a;
                };
                uint32_t fargb;
            };
        };
        uint64_t channels;
    };
};

struct UiColor {
    union {
        struct {
            uint8_t b, g, r;
        };
        uint32_t rgb;
    };
};

struct UiPair {
    uint fg;
    uint bg;
};

#define UI_MASK NCSTYLE_MASK
#define UI_NORMAL 0x0
#define UI_STANDOUT NCSTYLE_BOLD
#define UI_UNDERLINE NCSTYLE_UNDERLINE
#define UI_REVERSE NCSTYLE_UNDERLINE
#define UI_BLINK 0x0
#define UI_DIM 0x0
#define UI_BOLD NCSTYLE_BOLD
#define UI_ALTCHARSET 0x0
#define UI_INVIS 0x0
#define UI_STRUCK NCSTYLE_STRUCK
#define UI_PROTECT NCSTYLE_UNDERLINE
#define UI_UNDERCURL NCSTYLE_UNDERCURL
#define UI_NONE 0x0
#define UI_ITALIC NCSTYLE_ITALIC

#define WA_ATTRIBUTES NCSTYLE_MASK
#define WA_NORMAL 0x0
#define WA_STANDOUT NCSTYLE_BOLD
#define WA_UNDERLINE NCSTYLE_UNDERLINE
#define WA_REVERSE NCSTYLE_UNDERLINE
#define WA_BLINK NCSTYLE_UNDERLINE
#define WA_DIM NCSTYLE_UNDERLINE
#define WA_BOLD NCSTYLE_BOLD
#define WA_ALTCHARSET 0x0
#define WA_INVIS 0x0
#define WA_STRUCK NCSTYLE_STRUCK
#define WA_PROTECT NCSTYLE_UNDERLINE
#define WA_UNDERCURL NCSTYLE_UNDERCURL
#define WA_ITALIC NCSTYLE_ITALIC
#define WA_NONE 0x0

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

#endif
