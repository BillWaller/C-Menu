#ifndef UI_NOTCURSES_INTERNAL_H
#define UI_NOTCURSES_INTERNAL_H 1

/** @file ui_notcurses_internal.h
   @ingroup ui_notcurses
   @brief Internal header for the NotCurses UI backend.

   Defines the concrete (non-opaque) layouts of UiRuntime and UiSurface for
   the NotCurses backend.  Must only be included by NotCurses backend source
   files — never by application code.
*/

#include "../include/ui_backend.h"
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

typedef struct {
    union {
        struct {
            uint8_t b; // blue             LSB (Little Endian order)
            uint8_t g; // green
            uint8_t r; // red
            uint8_t a; // alpha            MSB
        };
        uint32_t rgba; // 0xAARRGGBB  alpha, red, green, blue
    };
} UiRGBA;

typedef struct {
    uint32_t fg; //       4-bytes
    uint32_t bg; //       4-bytes
} UiColorPair;   // total 8-bytes

typedef struct { // 16-bytes
    wchar_t wc;
    int attrs; //  4-bytes
    UiRGBA fg; //  4-bytes
    UiRGBA bg; //  4-bytes
} UiStyle;

typedef struct {
    char gcluster[5]; // 4-bytes for UTF-8 + null terminator;
    int stylemask;
    union {
        struct {
            UiRGBA fg; // foreground color 4-bytes
            UiRGBA bg; // background color 4-bytes
        };
        uint64_t channels;
    };
} UiCell;

struct UiSurface {
    union {
        struct {
            NcPlane *box;
            NcPlane *win;
            NcPlane *win2;
            NcPlane *lnno;
            NcPlane *cmdln;
            NcPlane *pad;
            NcPlane *plane1;
            NcPlane *plane2;
        };
        struct {
            NcPlane *mplane[8];
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

/* Internal style helpers */
int ui_bkgrnd(UiSurface *s, int w, const UiStyle *style, const char *c);
uint64_t ui_notcurses_channels_from_style(const UiStyle *style);
uint32_t ui_notcurses_attrs_from_style(const UiStyle *style);
NcPlane *ncplane_clicked(NcPlane *pile_member, ncinput *ni);

#endif
