#ifndef UI_NCURSES_INTERNAL_H
#define UI_NCURSES_INTERNAL_H 1

/** @file ui_ncurses_internal.h
   @ingroup ui_ncurses
   @brief Internal header for the NCurses UI backend.

   This header defines the concrete (non-opaque) layouts of UiRuntime and
   UiSurface for the NCurses backend.  It must only be included by NCurses
   backend source files — never by application code.
*/

#define _XOPEN_SOURCE_EXTENDED 1
#define _GNU_SOURCE
#define NCURSES_WIDECHAR 1
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#include <stdbool.h>
#include <stdint.h>

#define XLEN 128
#define MAXLEN 256
#define MAXSFC 30

#define UIKEY_RESIZE KEY_RESIZE
#define UIKEY_UP KEY_UP
#define UIKEY_RIGHT KEY_RIGHT
#define UIKEY_DOWN KEY_DOWN
#define UIKEY_LEFT KEY_LEFT
#define UIKEY_IC KEY_IC
#define UIKEY_DC KEY_DC
#define UIKEY_BACKSPACE KEY_BACKSPACE
#define UIKEY_NPAGE KEY_NPAGE
#define UIKEY_PPAGE KEY_PPAGE
#define UIKEY_HOME KEY_HOME
#define UIKEY_END KEY_END
#define UIKEY_F01 KEY_F(1)
#define UIKEY_F02 KEY_F(2)
#define UIKEY_F03 KEY_F(3)
#define UIKEY_F04 KEY_F(4)
#define UIKEY_F05 KEY_F(5)
#define UIKEY_F06 KEY_F(6)
#define UIKEY_F07 KEY_F(7)
#define UIKEY_F08 KEY_F(8)
#define UIKEY_F09 KEY_F(9)
#define UIKEY_F10 KEY_F(10)
#define UIKEY_F11 KEY_F(11)
#define UIKEY_F12 KEY_F(12)
#define UIKEY_ENTER KEY_ENTER
#define UIKEY_CLEAR KEY_CLEAR
#define UIKEY_BEG KEY_BEG
#define UIKEY_CANCEL KEY_CANCEL
#define UIKEY_CLOSE KEY_CLOSE
#define UIKEY_COMMAND KEY_COMMAND
#define UIKEY_COPY KEY_COPY
#define UIKEY_EXIT KEY_EXIT
#define UIKEY_PRINT KEY_PRINT
#define UIKEY_REFRESH KEY_REFRESH
#define UIKEY_MOUSE KEY_MOUSE
#define UIKEY_RSHIFT KEY_SRIGHT
#define UIKEY_LSHIFT KEY_SLEFT

#ifdef TO_BE_ADDED
#define UIKEY_INVALID KEY_INVALID
#define UIKEY_CAPS_LOCK KEY_CAPS_LOCK
#define UIKEY_SCROLL_LOCK KEY_SCROLL_LOCK
#define UIKEY_NUM_LOCK KEY_NUM_LOCK
#define UIKEY_PRINT_SCREEN KEY_PRINT_SCREEN
#define UIKEY_BUTTON1 KEY_BUTTON1
#define UIKEY_BUTTON2 KEY_BUTTON2
#define UIKEY_BUTTON3 KEY_BUTTON3
#define UIKEY_BUTTON4 KEY_BUTTON4
#define UIKEY_BUTTON5 KEY_BUTTON5
#define UIKEY_MEDIA_PLAY KEY_MEDIA_PLAY
#define UIKEY_MEDIA_PAUSE KEY_MEDIA_PAUSE
#define UIKEY_MEDIA_PPAUSE KEY_MEDIA_PPAUSE
#define UIKEY_MEDIA_REV KEY_MEDIA_REV
#define UIKEY_MEDIA_STOP KEY_MEDIA_STOP
#define UIKEY_MEDIA_FF KEY_MEDIA_FF
#define UIKEY_MEDIA_REWIND KEY_MEDIA_REWIND
#define UIKEY_MEDIA_NEXT KEY_MEDIA_NEXT
#define UIKEY_MEDIA_PREV KEY_MEDIA_PREV
#define UIKEY_MEDIA_RECORD KEY_MEDIA_RECORD
#define UIKEY_MEDIA_LVOL KEY_MEDIA_LVOL
#define UIKEY_MEDIA_RVOL KEY_MEDIA_RVOL
#define UIKEY_MEDIA_MUTE KEY_MEDIA_MUTE
#define UIKEY_SIGNAL KEY_SIGNAL
#define UIKEY_EOF KEY_EOF
#define UIKEY_SCROLL_UP KEY_SCROLL_UP
#define UIKEY_SCROLL_DOWN KEY_SCROLL_DOWN
#endif
#define UIKEY_BTAB KEY_BTAB
#define UIKEY_RETURN KEY_ENTER
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
#define UIKEY_CHAR 0703

#define UIKEY_BUTTON1_CLICKED BUTTON1_CLICKED
#define UIKEY_BUTTON1_PRESSED BUTTON1_PRESSED
#define UIKEY_BUTTON1_RELEASED BUTTON1_RELEASED
#define UIKEY_BUTTON2_CLICKED BUTTON2_CLICKED
#define UIKEY_BUTTON2_PRESSED BUTTON2_PRESSED
#define UIKEY_BUTTON2_RELEASED BUTTON2_RELEASED
#define UIKEY_BUTTON3_CLICKED BUTTON3_CLICKED
#define UIKEY_BUTTON3_PRESSED BUTTON3_PRESSED
#define UIKEY_BUTTON3_RELEASED BUTTON3_RELEASED
#define UIKEY_BUTTON4_CLICKED BUTTON4_CLICKED
#define UIKEY_BUTTON4_PRESSED BUTTON4_PRESSED
#define UIKEY_BUTTON4_RELEASED BUTTON4_RELEASED
#define UIKEY_BUTTON5_CLICKED BUTTON5_CLICKED
#define UIKEY_BUTTON5_PRESSED BUTTON5_PRESSED
#define UIKEY_BUTTON5_RELEASED BUTTON5_RELEASED
#define UIKEY_BUTTON1 0730
#define UIKEY_BUTTON2 0731
#define UIKEY_BUTTON3 0732
#define UIKEY_BUTTON4 0733
#define UIKEY_BUTTON5 0734

/** @struct UiRuntime
   @ingroup ui_ncurses
   @brief Runtime state for the NCurses UI backend.

   Holds all NCurses session resources so that multiple independent sessions
   can coexist and the codebase is not dependent on global variables.
   The legacy globals @c screen and @c tty_fp in @c dwin.c are updated by
   ui_init() / ui_shutdown() for backward compatibility with code that has
   not yet been migrated to the UAL API.
*/
struct UiRuntime {
    SCREEN *screen; /**< NCurses SCREEN created by newterm() */
    FILE *tty_fp;   /**< TTY file handle opened by ui_init() */
    bool mouse_enabled;
    bool alt_screen;
    bool cursor_visible;
    char border_style;
    uint lines;
    uint cols;
    PANEL *panel_main;
};

#define SFC_MAX 30

enum {
    BOX,
    WIN,
    WIN2,
    LNNO,
    CMDLN,
    PAD,
    WIN3,
    SUB_SFC_MAX
};

struct UiSurfaceMeta {
    unsigned int y;
    unsigned int x;
    unsigned int lines;
    unsigned int cols;
    cchar_t bkgd_cell;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};

struct UiSurface {
    union {
        struct { // DEPRECATED - will be removed
            PANEL *box_pan;
            PANEL *win_pan;
            PANEL *win2_pan;
            PANEL *lnno_pan;
            PANEL *cmdln_pan;
            PANEL *pad_pan;
            PANEL *plane1_pan;
            PANEL *plane2_pan;
        }; // END DEPRECATION
        struct {
            PANEL *mpan[SUB_SFC_MAX];
        };
    };
    union {
        struct { // DEPRECATED - will be removed
            WINDOW *box;
            WINDOW *win;
            WINDOW *win2;
            WINDOW *lnno;
            WINDOW *cmdln;
            WINDOW *pad;
            WINDOW *plane1;
            WINDOW *plane2;
        }; // END DEPRECATION
        struct {
            WINDOW *mwin[SUB_SFC_MAX];
        };
    };
    struct UiRuntime *runtime;
    struct UiSurfaceMeta meta[SUB_SFC_MAX];
    struct UiSurface *parent;
    int sfc_idx;
    int sub_cnt;
};

typedef struct {
    int32_t b, g, r, a;
} RGB;

typedef struct {
    int fg, bg;
} UiColorPair;

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

struct UiStyle {
    wchar_t wstr[5];
    attr_t attrs;
    short cp;
};

#define UI_MASK WA_ATTRIBUTES
#define UI_NORMAL WA_NORMAL
#define UI_STANDOUT WA_STANDOUT
#define UI_UNDERLINE WA_UNDERLINE
#define UI_REVERSE WA_REVERSE
#define UI_BLINK WA_BLINK
#define UI_DIM WA_DIM
#define UI_BOLD WA_BOLD
#define UI_ALTCHARSET WA_ALTCHARSET
#define UI_INVIS WA_INVIS
#define UI_PROTECT WA_PROTECT
#define UI_UNDERCURL WA_DIM
#define UI_STRUCK WA_INVIS
#define UI_NONE WA_NORMAL
#define UI_ITALIC WA_ITALIC

/** @struct UiSplitSurface
    @brief Split surface containing multiple child surfaces.
    @verbatim

    split_y > 0 && split_x > 0: 4 quadrants
    0: top-left, 1: top-right, 2: bottom-left, 3: bottom-right

    split_y > 0 && split_x == 0: 2 lines
    0: top, 1: bottom

    split_y == 0 && split_x > 0: 2 cols
    0: left, 1: right

    @endverbatim
 */

/** @struct UiSurface
   @ingroup ui_ncurses
   @brief A drawable surface in the NCurses backend.
   Wraps an NCurses WINDOW and its associated PANEL.
*/
/*
   This configuration is designed to make tiling and splitting surfaces easier
   to manage. One obvious application is to split the view screen into multiple
   panes when selecting photos from a gallery. I only added a four quadrants to
   start with, but it can be expanded to more panes in the future. The split_y and split_x values determine how the surface is divided. If both are greater than
   zero, the surface is split into four quadrants. If only one of them is greater than zero, the surface is split into two lines or two columns. The mwin array holds the child windows for each quadrant. The parent pointer allows for hierarchical relationships between surfaces, enabling complex layouts. The hidden flag can be used to control the visibility of the surface, and the name and title fields provide identifiers for the surface. The lines and cols fields store the dimensions of the surface, while the x and y fields store its position on the screen.
*/

// UiSurface is a structure that represents a drawable surface in the NCurses backend.
// It contains various fields that define the properties and behavior of the
// surface, including its associated PANELs and WINDOWs, runtime state,
// position, dimensions, visibility, and identifiers. The structure also
// supports hierarchical relationships between surfaces, allowing for complex
// layouts and tiling of multiple panes.
//
// The unions create named fields that duplicate the mwin and mpan arrays. This
// was merely a convenience to facilitate easier porting of legacy code. The
// SubSurface enum accomplishes the same thing, but the upper-case letters
// improve comprehennsibility when scan-reading code.
//
// Once the named fields have served their purpose they will most likely be
// removed in the future as they are redundant and can be confusing.
//
typedef cchar_t UiCell;
/* Internal style helpers */
int ui_ncurses_style_apply(struct UiSurface *s, uint w, const struct UiStyle *style);
int ui_ncurses_color_pair_from_style(const struct UiStyle *style);
void ui_style_destroy(struct UiStyle *);
struct UiStyle *ui_style_from_cch(const UiCell *);
UiCell ui_style_to_cch(const struct UiStyle *);
struct UiStyle *ui_style_new(void);
int ui_pair_from_hex(const char *fg, const char *bg);
struct UiStyle *ui_style_copy(const struct UiStyle *src);
int ui_wadd_cell(struct UiSurface *s, uint w, UiCell *cc);
int ui_mvwadd_cell(struct UiSurface *s, uint w, uint y, uint x, UiCell *cc);
int ui_wadd_cellstr(struct UiSurface *s, uint w, UiCell *cmplx_buf);
int ui_wadd_cellnstr(struct UiSurface *s, uint w, UiCell *cmplx_buf, uint n);
int ui_mvwadd_cellstr(struct UiSurface *s, uint w, uint y, uint x, UiCell *cmplx_buf);
int ui_mvwadd_cellnstr(struct UiSurface *s, uint w, uint y, uint x, UiCell *cmplx_buf, uint n);

extern struct UiSurface *stdsfc;

#endif
