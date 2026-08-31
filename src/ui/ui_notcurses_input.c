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

int ui_get_event_multix(UiSurface *s, uint w, UiEvent *ev, int timeout_ms);
/* -------------------------------------------------------------------------
   Key translation
   ------------------------------------------------------------------------- */

static UiKey translate_nckey(uint32_t id, const ncinput *ni) {
    if (id == NCKEY_INVALID || id == (uint32_t)NCKEY_EOF)
        return UIKEY_NONE;

    /* Mouse buttons — handled separately in ui_get_event(). */
    if (nckey_mouse_p(id))
        return UIKEY_MOUSE;

    switch (id) {
    case NCKEY_RESIZE:
        return UIKEY_RESIZE;
    case NCKEY_ENTER:
        return UIKEY_ENTER;
    case NCKEY_ESC:
        return UIKEY_ESCAPE;
    case NCKEY_BACKSPACE:
        return UIKEY_BACKSPACE;
    case NCKEY_TAB:
        return UIKEY_TAB;
    case NCKEY_UP:
        return UIKEY_UP;
    case NCKEY_DOWN:
        return UIKEY_DOWN;
    case NCKEY_LEFT:
        return UIKEY_LEFT;
    case NCKEY_RIGHT:
        return UIKEY_RIGHT;
    case NCKEY_HOME:
        return UIKEY_HOME;
    case NCKEY_END:
        return UIKEY_END;
    case NCKEY_PGUP:
        return UIKEY_PPAGE;
    case NCKEY_PGDOWN:
        return UIKEY_NPAGE;
    case NCKEY_INS:
        return UIKEY_IC;
    case NCKEY_DEL:
        return UIKEY_DC;
    case NCKEY_F01:
        return UIKEY_F01;
    case NCKEY_F02:
        return UIKEY_F02;
    case NCKEY_F03:
        return UIKEY_F03;
    case NCKEY_F04:
        return UIKEY_F04;
    case NCKEY_F05:
        return UIKEY_F05;
    case NCKEY_F06:
        return UIKEY_F06;
    case NCKEY_F07:
        return UIKEY_F07;
    case NCKEY_F08:
        return UIKEY_F08;
    case NCKEY_F09:
        return UIKEY_F09;
    case NCKEY_F10:
        return UIKEY_F10;
    case NCKEY_F11:
        return UIKEY_F11;
    case NCKEY_F12:
        return UIKEY_F12;
    default:
        if (ni && ni->evtype == NCTYPE_UNKNOWN)
            return UIKEY_NONE;
        /* Printable Unicode codepoint. */
        if (id >= 0x20 && id < NCKEY_INVALID)
            return UIKEY_CHAR;
        return UIKEY_NONE;
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
    // ui_cursor_enable(s, w, true);
    // tcflush(0, TCIFLUSH);
    if (timeout_ms < 0) {
        do {
            // notcurses_get(ui->nc, NULL, &ni);
            notcurses_get_blocking(ui->nc, &ni);
        } while (ni.evtype == NCTYPE_RELEASE ||
                 ni.id == NCKEY_INVALID ||
                 ni.id == NCKEY_MOTION ||
                 ni.id == NCKEY_SIGNAL);
    } else {
        struct timespec ts = {
            .tv_sec = timeout_ms / 1000,
            .tv_nsec = (long)(timeout_ms % 1000) * 1000000L,
        };
        do {
            notcurses_get(ui->nc, &ts, &ni);
        } while (ni.evtype == NCTYPE_RELEASE || ni.id == NCKEY_INVALID);
    }
    // notcurses_cursor_disable(ui->nc);
    ev->key = translate_nckey(ni.id, &ni);
    ev->alt = ncinput_alt_p(&ni);
    ev->ctrl = ncinput_ctrl_p(&ni);
    ev->shift = ncinput_shift_p(&ni);
    if (ev->key == UIKEY_CHAR) {
        ev->ch = ni.id; /* Unicode codepoint */
    } else if (ev->key == UIKEY_MOUSE) {
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

int ui_getch() {
    if (!ui)
        return -1;
    ncinput ni;
    do {
        notcurses_get(ui->nc, NULL, &ni);
    } while (ni.evtype == NCTYPE_RELEASE ||
             ni.id == NCKEY_INVALID ||
             ni.id == NCKEY_MOTION ||
             ni.id == NCKEY_SIGNAL);
    return ni.id;
}

int ui_get_event_multix(UiSurface *s, uint w, UiEvent *ev, int timeout_ms) {
    (void)timeout_ms;
    (void)ev;
    (void)w;
    (void)s;
    if (!ui)
        return -1;
    ncinput ni;
    do {
        // notcurses_get_blocking(ui->nc, &ni);
        notcurses_get(ui->nc, NULL, &ni);
    } while (ni.evtype == NCTYPE_RELEASE ||
             ni.id == NCKEY_INVALID ||
             ni.id == NCKEY_MOTION ||
             ni.id == NCKEY_SIGNAL);
    return ni.id;
}
int ui_get_event_multi(UiSurface *s, uint w, UiEvent *ev, int timeout_ms) {
    if (!ui || !ev)
        return -1;
    memset(ev, 0, sizeof(*ev));
    ncinput ni;
    if (timeout_ms < 0)
        do {
            notcurses_get(ui->nc, NULL, &ni);
        } while (ni.evtype == NCTYPE_RELEASE ||
                 ni.id == NCKEY_INVALID ||
                 ni.id == NCKEY_MOTION ||
                 ni.id == NCKEY_SIGNAL);
    else {
        struct timespec ts = {
            .tv_sec = timeout_ms / 1000,
            .tv_nsec = (long)(timeout_ms % 1000) * 1000000L,
        };
        do {
            notcurses_get(ui->nc, &ts, &ni);
        } while (ni.evtype == NCTYPE_RELEASE || ni.id == NCKEY_INVALID);
    }
    // notcurses_cursor_disable(ui->nc);
    ev->key = translate_nckey(ni.id, &ni);
    ev->alt = ncinput_alt_p(&ni);
    ev->ctrl = ncinput_ctrl_p(&ni);
    ev->shift = ncinput_shift_p(&ni);
    if (ev->key == UIKEY_CHAR) {
        ev->ch = ni.id; /* Unicode codepoint */
    } else if (ev->key == UIKEY_MOUSE) {
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
    // notcurses_mice_disable(ui->nc);
    do {
        notcurses_get(ui->nc, NULL, &ni);
    } while (ni.id == NCKEY_INVALID);
    ev->key = translate_nckey(ni.id, &ni);
    ev->alt = ncinput_alt_p(&ni);
    ev->ctrl = ncinput_ctrl_p(&ni);
    ev->shift = ncinput_shift_p(&ni);
    if (ev->key == UIKEY_CHAR)
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
