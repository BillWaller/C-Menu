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
int ui_draw_text(UiSurface *s, uint w, uint y, uint x, const UiStyle *style,
                 const char *text) {
    if (!s || !text)
        return -1;
    ncplane_putstr_yx(s->mplane[w], y, x, text);
    return 0;
}

/** @brief Draw at most @p n bytes of UTF-8 text at (y, x). */
int ui_draw_text_n(UiSurface *s, uint w, uint y, uint x, const UiStyle *style,
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

int ui_wadd_cell(UiSurface *s, uint w, const nccell *uic) {
    if (!s)
        return -1;

    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_mvwadd_cell(UiSurface *s, uint w, uint y, uint x, const nccell *uic) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, uic);
    return 0;
}
// uic strings
int ui_wadd_cellstr(UiSurface *s, uint w, nccell *uic) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_cellstr(UiSurface *s, uint w, uint y, uint x, nccell *uic) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    uint i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_wadd_cellnstr(UiSurface *s, uint w, nccell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0' && i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_cellnstr(UiSurface *s, uint w, uint y, uint x, nccell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    ui_wmove(s, w, y, x);
    while (i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
//----------------------------------------------------------

// Compose NCCell from its components
int ui_compose_nccell(struct UiCell *uic, const uint32_t gcluster, UiStyle *style) {
    if (!uic || !gcluster)
        return -1;
    memset(uic, 0, sizeof(struct nccell));
    uic->gcluster = gcluster;
    if (!uic->gcluster)
        return -1;
    uic->backstop = 0;
    uic->stylemask = style->stylemask;
    uic->channels.fb = style->channels.fb;
    return 0;
}
// Decompose NCCell into its components
int ui_decompose_nccell(struct UiCell *uic, wchar_t *wstr, uint16_t *stylemask, uint64_t *channels) {
    if (!uic)
        return -1;
    uint i = 0;
    while (uic->wstr[i]) {
        wstr[i] = uic->wstr[i];
        i++;
    }
    wstr[i] = '\0';
    *stylemask = uic->stylemask;
    *channels = uic->channels.fb;
    return 0;
}

int ui_mvwadd_style(UiSurface *s, uint w, uint y, uint x, const UiStyle *style) {
    if (!s || !style)
        return -1;
    nccell uic = {0};
    ui_setnccell(&uic, style->gcluster, style->stylemask, style->channels.fb);
    ui_mvwadd_cell(s, w, y, x, &uic);
    return 0;
}
// ---------------------------------------------------------------------------
// nccells
// ---------------------------------------------------------------------------
int ui_wadd_wch(UiSurface *s, uint w, const nccell *uic) {
    if (!s)
        return -1;

    ncplane_putc(s->mplane[w], uic);
    return 0;
}
int ui_mvwadd_wch(UiSurface *s, uint w, uint y, uint x, const struct nccell *uic) {
    if (!s)
        return -1;
    ncplane_putc_yx(s->mplane[w], y, x, uic);
    return 0;
}
// uic strings
int ui_wadd_wchstr(UiSurface *s, uint w, struct nccell *uic) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_mvwadd_wchstr(UiSurface *s, uint w, uint y, uint x, struct nccell *uic) {
    if (!s)
        return -1;
    ui_wmove(s, w, y, x);
    uint i = 0;
    while (uic[i].gcluster != '\0') {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}
int ui_wadd_wchnstr(UiSurface *s, uint w, struct nccell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    while (uic[i].gcluster != '\0' && i < n) {
        ncplane_putc(s->mplane[w], uic);
        i++;
    }
    return 0;
}

int ui_mvwadd_wchnstr(UiSurface *s, uint w, uint y, uint x, struct nccell *uic, uint n) {
    if (!s)
        return -1;
    uint i = 0;
    ui_wmove(s, w, y, x);
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
