/** @file ui_notcurses_input.c
   @ingroup ui_notcurses
   @brief NotCurses UI backend — input handling.

   Translates NotCurses @c ncinput events into the portable UiEvent
   representation defined in ui_backend.h.
*/

#include "ui_backend.h"
#include "ui_notcurses_internal.h"
#include <notcurses/notcurses.h>
#include <string.h>
#include <termios.h>

/* -------------------------------------------------------------------------
   Key translation
   ------------------------------------------------------------------------- */

static UiKey translate_nckey(uint32_t id, const ncinput *ni) {
    if (id == NCKEY_INVALID || id == (uint32_t)NCKEY_EOF)
        return UI_KEY_NONE;

    /* Mouse buttons — handled separately in ui_get_event(). */
    if (nckey_mouse_p(id))
        return UI_KEY_MOUSE;

    switch (id) {
    case NCKEY_RESIZE:
        return UI_KEY_RESIZE;
    case NCKEY_ENTER:
        return UI_KEY_ENTER;
    case NCKEY_ESC:
        return UI_KEY_ESCAPE;
    case NCKEY_BACKSPACE:
        return UI_KEY_BACKSPACE;
    case NCKEY_TAB:
        return UI_KEY_TAB;
    case NCKEY_UP:
        return UI_KEY_UP;
    case NCKEY_DOWN:
        return UI_KEY_DOWN;
    case NCKEY_LEFT:
        return UI_KEY_LEFT;
    case NCKEY_RIGHT:
        return UI_KEY_RIGHT;
    case NCKEY_HOME:
        return UI_KEY_HOME;
    case NCKEY_END:
        return UI_KEY_END;
    case NCKEY_PGUP:
        return UI_KEY_PGUP;
    case NCKEY_PGDOWN:
        return UI_KEY_PGDN;
    case NCKEY_INS:
        return UI_KEY_INSERT;
    case NCKEY_DEL:
        return UI_KEY_DELETE;
    case NCKEY_F01:
        return UI_KEY_F1;
    case NCKEY_F02:
        return UI_KEY_F2;
    case NCKEY_F03:
        return UI_KEY_F3;
    case NCKEY_F04:
        return UI_KEY_F4;
    case NCKEY_F05:
        return UI_KEY_F5;
    case NCKEY_F06:
        return UI_KEY_F6;
    case NCKEY_F07:
        return UI_KEY_F7;
    case NCKEY_F08:
        return UI_KEY_F8;
    case NCKEY_F09:
        return UI_KEY_F9;
    case NCKEY_F10:
        return UI_KEY_F10;
    case NCKEY_F11:
        return UI_KEY_F11;
    case NCKEY_F12:
        return UI_KEY_F12;
    default:
        if (ni && ni->evtype == NCTYPE_UNKNOWN)
            return UI_KEY_NONE;
        /* Printable Unicode codepoint. */
        if (id >= 0x20 && id < NCKEY_INVALID)
            return UI_KEY_CHAR;
        return UI_KEY_NONE;
    }
}

/* -------------------------------------------------------------------------
   Event retrieval
   ------------------------------------------------------------------------- */

/** @brief Wait for an input event from the NotCurses context.
   @param ui         UI runtime context.
   @param target     Unused for NotCurses (events are global to the context).
   @param ev         Output UiEvent structure.
   @param timeout_ms Milliseconds to wait; -1 = block indefinitely.
   @return 0 on success, -1 if @p ui or @p ev is NULL.
*/
int ui_get_event(UiSurface *s, uint w, UiEvent *ev, int timeout_ms) {
    if (!ui || !ev)
        return -1;
    memset(ev, 0, sizeof(*ev));

    ncinput ni;
    ui_cursor_enable(s, w, true);
    // tcflush(0, TCIFLUSH);
    if (timeout_ms < 0) {
        do {
            notcurses_get_blocking(ui->nc, &ni);
        } while (ni.evtype == NCTYPE_RELEASE || ni.id == NCKEY_INVALID);
    } else {
        struct timespec ts = {
            .tv_sec = timeout_ms / 1000,
            .tv_nsec = (long)(timeout_ms % 1000) * 1000000L,
        };
        do {
            notcurses_get(ui->nc, &ts, &ni);
        } while (ni.evtype == NCTYPE_RELEASE || ni.id == NCKEY_INVALID);
    }
    notcurses_cursor_disable(ui->nc);
    ev->key = translate_nckey(ni.id, &ni);
    ev->alt = ncinput_alt_p(&ni);
    ev->ctrl = ncinput_ctrl_p(&ni);
    ev->shift = ncinput_shift_p(&ni);
    if (ev->key == UI_KEY_CHAR) {
        ev->ch = ni.id; /* Unicode codepoint */
    } else if (ev->key == UI_KEY_MOUSE) {
        ev->y = ni.y;
        ev->x = ni.x;
        if (ni.id == NCKEY_BUTTON4)
            ev->mouse_action = UI_MOUSE_SCROLL_UP;
        else if (ni.id == NCKEY_BUTTON5)
            ev->mouse_action = UI_MOUSE_SCROLL_DOWN;
        else if (ni.evtype == NCTYPE_PRESS)
            ev->mouse_action = UI_MOUSE_PRESS;
        else if (ni.evtype == NCTYPE_RELEASE)
            ev->mouse_action = UI_MOUSE_RELEASE;
    }
    return ni.id;
}

int ui_get_event_multi(UiSurface *s, uint w, UiEvent *ev, int timeout_ms) {
    if (!ui || !ev)
        return -1;
    memset(ev, 0, sizeof(*ev));

    ncinput ni;
    // int y, x;
    // notcurses_cursor_yx(ui->nc, &y, &x);
    // notcurses_cursor_enable(ui->nc, y, x);
    notcurses_render(ui->nc);
    if (timeout_ms < 0)
        do {
            notcurses_get_blocking(ui->nc, &ni);
        } while (ni.evtype == NCTYPE_RELEASE || ni.id == NCKEY_INVALID);
    else {
        struct timespec ts = {
            .tv_sec = timeout_ms / 1000,
            .tv_nsec = (long)(timeout_ms % 1000) * 1000000L,
        };
        do {
            notcurses_get(ui->nc, &ts, &ni);
        } while (ni.evtype == NCTYPE_RELEASE || ni.id == NCKEY_INVALID);
    }
    notcurses_cursor_disable(ui->nc);
    ev->key = translate_nckey(ni.id, &ni);
    ev->alt = ncinput_alt_p(&ni);
    ev->ctrl = ncinput_ctrl_p(&ni);
    ev->shift = ncinput_shift_p(&ni);
    if (ev->key == UI_KEY_CHAR) {
        ev->ch = ni.id; /* Unicode codepoint */
    } else if (ev->key == UI_KEY_MOUSE) {
        if (ni.id == NCKEY_BUTTON1) {
            struct ncplane *nn = ncplane_clicked(s, w, &ni);
            ev->in_win = get_plane_idx(s, nn);
            ev->key = 0;
        }
    }
    ev->y = ni.y;
    ev->x = ni.x;
    if (ni.id == NCKEY_BUTTON4)
        ev->mouse_action = UI_MOUSE_SCROLL_UP;
    else if (ni.id == NCKEY_BUTTON5)
        ev->mouse_action = UI_MOUSE_SCROLL_DOWN;
    else if (ni.evtype == NCTYPE_PRESS)
        ev->mouse_action = UI_MOUSE_PRESS;
    else if (ni.evtype == NCTYPE_RELEASE)
        ev->mouse_action = UI_MOUSE_RELEASE;
    return ni.id;
}

int ui_get_event_no_mouse(UiSurface *target, uint w, UiEvent *ev) {
    (void)target;
    (void)w;
    if (!ui || !ev)
        return -1;
    memset(ev, 0, sizeof(*ev));
    ncinput ni;
    notcurses_mice_disable(ui->nc);
    do {
        notcurses_get_blocking(ui->nc, &ni);
    } while (ni.evtype == NCTYPE_RELEASE || ni.id == NCKEY_INVALID);
    ev->key = translate_nckey(ni.id, &ni);
    ev->alt = ncinput_alt_p(&ni);
    ev->ctrl = ncinput_ctrl_p(&ni);
    ev->shift = ncinput_shift_p(&ni);
    if (ev->key == UI_KEY_CHAR)
        ev->ch = ni.id;
    return ni.id;
}

uint get_plane_idx(UiSurface *s, struct ncplane *plane) {
    if (!s)
        return -1;
    for (uint w = 0; w < SUB_SFC_MAX; ++w) {
        if (s->mplane[w] == plane)
            return w;
    }
    return -1;
}

NcPlane *ncplane_clicked(UiSurface *s, uint w, ncinput *ni) {
    NcPlane *pile_member = s->mplane[w];
    struct ncplane *cur = ncpile_top(pile_member);
    while (cur != NULL) {
        int y = ni->y, x = ni->x;
        if (ncplane_translate_abs(cur, &y, &x)) {
            ni->y = y;
            ni->x = x;
            return cur;
        }
        cur = ncplane_below(cur);
    }
    return NULL;
}
