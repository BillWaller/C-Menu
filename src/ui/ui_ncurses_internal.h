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

#include "ui_backend.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#include <stdbool.h>

#define XLEN 256

typedef cchar_t UiCell;

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
    int lines;
    int cols;
    PANEL *panel_main;
};

struct UiColorPair {
    int fg;
    int bg;
};

struct UiStyle {
    wchar_t wc;
    attr_t attrs;
    short cp;
    int fg;
    struct {
        int r, g, b;
    } frgb;
    int bg;
    struct {
        int r, g, b;
    } brgb;
};

#define stdsfc stdscr

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

typedef enum {
    BOX,
    WIN,
    WIN2,
    LNNO,
    CMDLN,
    PAD,
    WINX,
    SUB_SFC_MAX
} SubSurface;

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

struct UiSurface {
    union {
        struct {
            PANEL *box_pan;
            PANEL *win_pan;
            PANEL *win2_pan;
            PANEL *lnno_pan;
            PANEL *cmdln_pan;
            PANEL *pad_pan;
        };
        struct {
            PANEL *mpan[8];
        };
    };
    union {
        struct {
            WINDOW *box;
            WINDOW *win;
            WINDOW *win2;
            WINDOW *lnno;
            WINDOW *cmdln;
            WINDOW *pad;
        };
        struct {
            WINDOW *mwin[8];
        };
    };
    struct UiRuntime *runtime;
    struct UiSurface *parent;
    int y;
    int x;
    int lines;
    int cols;
    int sfc_idx;
    int sub_cnt;
    bool hidden;
    char name[XLEN];
    char title[XLEN];
};

/* Internal style helpers */
int ui_ncurses_style_apply(UiSurface *s, int w, const UiStyle *style);
int ui_ncurses_color_pair_from_style(const UiStyle *style);
UiStyle *ui_style_new(void);
void ui_style_destroy(UiStyle *);
UiStyle *ui_style_from_cch(const UiCell *);
UiCell ui_style_to_cch(const UiStyle *, const char *);

int ui_wadd_wchstr(UiSurface *s, int w, UiCell *cmplx_buf);
int ui_wadd_wch(UiSurface *s, int w, UiCell *cc);
int ui_mvwadd_wch(UiSurface *s, int w, int y, int x, UiCell *cc);
int ui_mvwadd_wchnstr(UiSurface *s, int w, int y, int x, UiCell *cmplx_buf, int n);
int ui_mvwadd_wchstr(UiSurface *s, int w, int y, int x, UiCell *cmplx_buf);
int ui_setcchar(UiCell *wch, const wchar_t *wc, attr_t attrs, short pair, const void *opts);
int ui_bkgrnd(UiSurface *s, int w, const UiCell *c);
int ui_bkgrndset(UiSurface *s, int w, const UiCell *c);

#endif
