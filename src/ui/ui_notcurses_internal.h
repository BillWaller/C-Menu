#ifndef UI_NOTCURSES_INTERNAL_H
#define UI_NOTCURSES_INTERNAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

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

#define UIKEY_INVALID NCKEY_INVALID
#define UIKEY_RESIZE NCKEY_RESIZE
#define UIKEY_UP NCKEY_UP
#define UIKEY_RIGHT NCKEY_RIGHT
#define UIKEY_DOWN NCKEY_DOWN
#define UIKEY_LEFT NCKEY_LEFT
#define UIKEY_IC NCKEY_INS
#define UIKEY_DC NCKEY_DEL
#define UIKEY_BACKSPACE NCKEY_BACKSPACE
#define UIKEY_NPAGE NCKEY_PGDOWN
#define UIKEY_PPAGE NCKEY_PGUP
#define UIKEY_HOME NCKEY_HOME
#define UIKEY_END NCKEY_END
#define UIKEY_F01 NCKEY_F01
#define UIKEY_F02 NCKEY_F02
#define UIKEY_F03 NCKEY_F03
#define UIKEY_F04 NCKEY_F04
#define UIKEY_F05 NCKEY_F05
#define UIKEY_F06 NCKEY_F06
#define UIKEY_F07 NCKEY_F07
#define UIKEY_F08 NCKEY_F08
#define UIKEY_F09 NCKEY_F09
#define UIKEY_F10 NCKEY_F10
#define UIKEY_F11 NCKEY_F11
#define UIKEY_F12 NCKEY_F12
#define UIKEY_F13 NCKEY_F13
#define UIKEY_ENTER NCKEY_ENTER
#define UIKEY_CLEAR NCKEY_CLS
#define UIKEY_BEG NCKEY_BEGIN
#define UIKEY_CANCEL NCKEY_CANCEL
#define UIKEY_CLOSE NCKEY_CLOSE
#define UIKEY_COMMAND NCKEY_COMMAND
#define UIKEY_COPY NCKEY_COPY
#define UIKEY_EXIT NCKEY_EXIT
#define UIKEY_PRINT NCKEY_PRINT
#define UIKEY_REFRESH NCKEY_REFRESH
#define UIKEY_CAPS_LOCK NCKEY_CAPS_LOCK
#define UIKEY_SCROLL_LOCK NCKEY_SCROLL_LOCK
#define UIKEY_NUM_LOCK NCKEY_NUM_LOCK
#define UIKEY_PRINT_SCREEN NCKEY_PRINT_SCREEN
#define UIKEY_BUTTON1 NCKEY_BUTTON1
#define UIKEY_BUTTON2 NCKEY_BUTTON2
#define UIKEY_BUTTON3 NCKEY_BUTTON3
#define UIKEY_BUTTON4 NCKEY_BUTTON4
#define UIKEY_BUTTON5 NCKEY_BUTTON5
#define UIKEY_RSHIFT NCKEY_RSHIFT
#define UIKEY_LSHIFT NCKEY_LSHIFT
#define UIKEY_MEDIA_PLAY NCKEY_MEDIA_PLAY
#define UIKEY_MEDIA_PAUSE NCKEY_MEDIA_PAUSE
#define UIKEY_MEDIA_PPAUSE NCKEY_MEDIA_PPAUSE
#define UIKEY_MEDIA_REV NCKEY_MEDIA_REV
#define UIKEY_MEDIA_STOP NCKEY_MEDIA_STOP
#define UIKEY_MEDIA_FF NCKEY_MEDIA_FF
#define UIKEY_MEDIA_REWIND NCKEY_MEDIA_REWIND
#define UIKEY_MEDIA_NEXT NCKEY_MEDIA_NEXT
#define UIKEY_MEDIA_PREV NCKEY_MEDIA_PREV
#define UIKEY_MEDIA_RECORD NCKEY_MEDIA_RECORD
#define UIKEY_MEDIA_LVOL NCKEY_MEDIA_LVOL
#define UIKEY_MEDIA_RVOL NCKEY_MEDIA_RVOL
#define UIKEY_MEDIA_MUTE NCKEY_MEDIA_MUTE
#define UIKEY_SIGNAL NCKEY_SIGNAL
#define UIKEY_EOF NCKEY_EOF
#define UIKEY_SCROLL_UP NCKEY_SCROLL_UP
#define UIKEY_SCROLL_DOWN NCKEY_SCROLL_DOWN
#define UIKEY_RETURN NCKEY_ENTER
#define UIKEY_TAB 0x09
#define UIKEY_ESCAPE 0x1B
#define UIKEY_SPACE 0x20
#define UIKEY_MOD_SHIFT 1
#define UIKEY_MOD_ALT 2
#define UIKEY_MOD_CTRL 4
#define UIKEY_MOD_SUPER 8
#define UIKEY_MOD_HYPER 16
#define UIKEY_MOD_META 32
#define UIKEY_MOD_CAPSLOCK 64
#define UIKEY_MOD_NUMLOCK 128
#define UIKEY_NONE 0700
#define UIKEY_BTAB 0701
#define UIKEY_MOUSE 0702
#define UIKEY_CHAR 0703
#define UIKEY_BUTTON1_CLICKED 0704
#define UIKEY_BUTTON1_PRESSED 0706
#define UIKEY_BUTTON1_RELEASED 0707
#define UIKEY_BUTTON2_CLICKED 0710
#define UIKEY_BUTTON2_PRESSED 0711
#define UIKEY_BUTTON2_RELEASED 0712
#define UIKEY_BUTTON3_CLICKED 0713
#define UIKEY_BUTTON3_PRESSED 0714
#define UIKEY_BUTTON3_RELEASED 0715
#define UIKEY_BUTTON4_CLICKED 0716
#define UIKEY_BUTTON4_PRESSED 0717
#define UIKEY_BUTTON4_RELEASED 0720
#define UIKEY_BUTTON5_CLICKED 0721
#define UIKEY_BUTTON5_PRESSED 0722
#define UIKEY_BUTTON5_RELEASED 0723

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
    struct nccell bkgd_cell;
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

#ifdef __cplusplus
}
#endif
#endif
