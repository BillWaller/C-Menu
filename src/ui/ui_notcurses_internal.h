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

/* Internal style helpers */
uint64_t ui_notcurses_channels_from_style(const UiStyle *style);
uint32_t ui_notcurses_attrs_from_style(const UiStyle *style);

#endif
