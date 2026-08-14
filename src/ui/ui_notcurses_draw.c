/** @file ui_notcurses_draw.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — drawing functions.

   Implements the drawing operations declared in ui_backend.h using the
   NotCurses API.
*/
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1

#include "ui_backend.h"
#include "ui_notcurses_internal.h"
#include <notcurses/notcurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define CELL_CHAR_INITIALIZER(c) { \
    .gcluster = (c),               \
    .gcluster_backstop = 0,        \
    .reserved = 0,                 \
    .stylemask = 0,                \
    .channels = 0,                 \
}
#define CELL_INITIALIZER(c, s, chan) { \
    .gcluster = (c),                   \
    .gcluster_backstop = 0,            \
    .reserved = 0,                     \
    .stylemask = (s),                  \
    .channels = (chan),                \
}

// static inline void
// nccell_init(nccell* c){
//   memset(c, 0, sizeof(*c));
// }

/* -------------------------------------------------------------------------
   Text
   ------------------------------------------------------------------------- */

/** @brief Draw UTF-8 text at (y, x) with optional style. */
int ui_draw_text(UiSurface *s, int w, int y, int x, const UiStyle *style,
                 const char *text) {
    if (!s || !text)
        return -1;
    if (style) {
        ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    }
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}

/** @brief Draw at most @p n bytes of UTF-8 text at (y, x). */
int ui_draw_text_n(UiSurface *s, int w, int y, int x, const UiStyle *style,
                   const char *text, size_t n) {
    if (!s || !text)
        return -1;
    if (style) {
        ncplane_set_channels(s->mplane[w], ui_notcurses_channels_from_style(style));
        ncplane_set_styles(s->mplane[w], ui_notcurses_attrs_from_style(style));
    }
    ncplane_putnstr_yx(s->mplane[w], y, x, n, text);
    return 0;
}
// ---------------------------------------------------------------------------
// nccells
// ---------------------------------------------------------------------------

int ui_setnccell(nccell *uic,
                 const uint32_t gcluster,
                 uint32_t stylemask,
                 const uint64_t channels) {
    if (!uic || !gcluster)
        return -1;
    memset(uic, 0, sizeof(struct nccell));
    uic->gcluster = gcluster;
    if (!uic->gcluster)
        return -1;
    uic->gcluster_backstop = 0;
    uic->stylemask = stylemask;
    uic->channels = channels;
    return 0;
}
//
// uic character single
int ui_wadd_wch(UiSurface *s, int w, const nccell *uic) {
    if (!s)
        return -1;

    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, int w, int y, int x, const UiCell *uic) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, uic);
    return 0;
}
// uic strings
int ui_wadd_wchstr(UiSurface *s, int w, UiCell *uic) {
    if (!s)
        return -1;
    int i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_wchstr(UiSurface *s, int w, int y, int x, UiCell *uic) {
    if (!s)
        return -1;
    ui_cursor_move(s, w, y, x);
    int i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_wadd_wchnstr(UiSurface *s, int w, UiCell *uic, int n) {
    if (!s)
        return -1;
    int i = 0;
    while (uic[i].gcluster != '\0' && i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_wchnstr(UiSurface *s, int w, int y, int x, UiCell *uic, int n) {
    if (!s)
        return -1;
    int i = 0;
    ui_cursor_move(s, w, y, x);
    while (i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
/* -------------------------------------------------------------------------
   Screen managemen t
   ------------------------------------------------------------------------- */

void ui_restore_wins() {
    //  for (int s = 0; s <= sfc_ptr; s++) {
    //      for (int w = 0; w < 8; w++)
    //          if (ui_surface[s]->mplane[w] != nullptr)
    //      // touchwin(ui_surface[s]->mplane[w]);
    //  }
    //  ui_render(ui_runtime);
}
