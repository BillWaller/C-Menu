/** @file ui_ncurses_input.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — input handling.

   Translates raw NCurses key codes and mouse events into the portable
   UiEvent representation defined in ui_backend.h.
*/

#include "ui_backend.h"
#include "ui_ncurses_internal.h"
#include <stddef.h>
#include <string.h>
#include <termios.h>
/* -------------------------------------------------------------------------
   Key translation
   ------------------------------------------------------------------------- */

static UiKey translate_key(int ch) {
    switch (ch) {
    case ERR:
        return UIKEY_NONE;
    case '\n':
    case '\r':
        return UIKEY_ENTER;
    case 27:
        return UIKEY_ESCAPE;
    case '\t':
        return UIKEY_TAB;
#ifdef KEY_BTAB
    case KEY_BTAB:
        return UIKEY_BTAB;
#endif
    case KEY_UP:
        return UIKEY_UP;
    case KEY_DOWN:
        return UIKEY_DOWN;
    case KEY_LEFT:
        return UIKEY_LEFT;
    case KEY_RIGHT:
        return UIKEY_RIGHT;
    case KEY_HOME:
        return UIKEY_HOME;
    case KEY_END:
        return UIKEY_END;
    case KEY_PPAGE:
        return UIKEY_PPAGE;
    case KEY_NPAGE:
        return UIKEY_NPAGE;
    case KEY_IC:
        return UIKEY_IC;
    case KEY_DC:
        return UIKEY_DC;
    case KEY_BACKSPACE:
        return UIKEY_BACKSPACE;
#ifdef KEY_RESIZE
    case KEY_RESIZE:
        return UIKEY_RESIZE;
#endif
    case KEY_MOUSE:
        return UIKEY_MOUSE;
    case KEY_F(1):
        return UIKEY_F01;
    case KEY_F(2):
        return UIKEY_F02;
    case KEY_F(3):
        return UIKEY_F03;
    case KEY_F(4):
        return UIKEY_F04;
    case KEY_F(5):
        return UIKEY_F05;
    case KEY_F(6):
        return UIKEY_F06;
    case KEY_F(7):
        return UIKEY_F07;
    case KEY_F(8):
        return UIKEY_F08;
    case KEY_F(9):
        return UIKEY_F09;
    case KEY_F(10):
        return UIKEY_F10;
    case KEY_F(11):
        return UIKEY_F11;
    case KEY_F(12):
        return UIKEY_F12;
    case UIKEY_BUTTON1_CLICKED:
        return UIKEY_BUTTON1;
    default:
        if (ch >= 32 && ch <= 126)
            return UIKEY_CHAR;
        return UIKEY_NONE;
    }
}

/* -------------------------------------------------------------------------
   Event retrieval
   ------------------------------------------------------------------------- */

/** @brief Wait for an input event on @p target (or stdscr if NULL).
   @param ui         UI runtime context (unused — event comes from the window).
   @param target     Surface to read from, or NULL for stdscr.
   @param ev         Output UiEvent structure.
   @param timeout_ms Milliseconds to wait; -1 = block indefinitely.
   @return 0 on success, -1 if @p ev is NULL.
*/
int ui_get_event(UiSurface *s, uint w, UiChyron *chyron, UiEvent *ev, int timeout_ms) {
    if (!ev)
        return -1;
    memset(ev, 0, sizeof(*ev));
    tcflush(2, TCIFLUSH);
    if (timeout_ms <= 0) {
        timeout_ms = -1;
    } else
        wtimeout(s->mwin[w], timeout_ms);
    curs_set(2);
    int ch = wgetch(s->mwin[w]);

    if (ev->key == UIKEY_CHAR) {
        ev->ch = (uint32_t)ch;
    } else if (ev->key == UIKEY_MOUSE) {
        MEVENT me;
        if (getmouse(&me) == OK) {
            ev->y = me.y;
            ev->x = me.x;
            if (me.bstate & BUTTON4_PRESSED)
                ev->mouse_action = UI_MOUSE_SCROLL_UP;
            else if (me.bstate & BUTTON5_PRESSED)
                ev->mouse_action = UI_MOUSE_SCROLL_DOWN;
            else if (me.bstate & BUTTON1_PRESSED)
                ev->mouse_action = UI_MOUSE_PRESS;
            else if (me.bstate & BUTTON1_RELEASED)
                ev->mouse_action = UI_MOUSE_RELEASE;
            if (wenclose(s->mwin[w], me.y, me.x) &&
                wmouse_trafo(s->mwin[w], &me.y, &me.x, false)) {
                ev->bstate = me.bstate;
                ev->mouse_inside = true;
                ev->y = me.y;
                ev->x = me.x;
            } else {
                ev->mouse_inside = false;
            }
        }
        //==========================================================================
        ev->key = translate_key(ch);
        if (ev->key == UIKEY_CHAR)
            ev->ch = (uint32_t)ch; /* Unicode codepoint */
        else if (ev->key == UIKEY_MOUSE) {
            if (ch == UIKEY_BUTTON4_PRESSED)
                ev->mouse_action = UIKEY_SCROLL_UP;
            else if (ch == UIKEY_BUTTON5_PRESSED)
                ev->mouse_action = UIKEY_SCROLL_DOWN;
            else if (ch == UIKEY_BUTTON1_CLICKED) {
                ev->mouse_action = UIKEY_BUTTON1_CLICKED;
                for (int i = WIN; i < SUB_SFC_MAX; i++) {
                    if (s->mwin[i] != NULL &&
                        wenclose(s->mwin[i], me.y, me.x) &&
                        wmouse_trafo(s->mwin[i], &me.y, &me.x, false)) {
                        ev->bstate = me.bstate;
                        ev->in_win = i;
                        ev->y = me.y;
                        ev->x = me.x;
                        break;
                    }
                }
                if (chyron) {
                    if (ev->in_win == chyron->win && ev->y == chyron->y) {
                        ev->mouse_action = UIKEY_BUTTON1_CLICKED;
                        ev->key = ui_get_chyron_key(chyron, ev->x);
                        return ev->key;
                    } else
                        return 0;
                }
            }
        }
    }
    curs_set(0);
    return ch;
}

int ui_get_event_no_mouse(UiSurface *s, uint w, UiEvent *ev) {
    int ch;
    mousemask(0, NULL);
    curs_set(2);
    qiflush();
    tcflush(2, TCIFLUSH);
    cbreak();
    do {
        curs_set(2);
        ch = wgetch(s->mwin[w]);
        curs_set(0);
        ev->key = translate_key(ch);
        if (ev->key == UIKEY_CHAR) {
            ev->ch = (uint32_t)ch;
            break;
        }
    } while (ch == ERR);
    curs_set(0);
    return ch;
}
/* -------------------------------------------------------------------------
   Mice
   ------------------------------------------------------------------------- */
int ui_mousemask(int mask) {
    if (!ui)
        return -1;
    if (mask)
        mousemask(mask, nullptr);
    else
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
    return 0;
}
int ui_mice_enable(int mask) {
    if (!ui)
        return -1;
    if (mask)
        mousemask(mask, nullptr);
    else
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
    return 0;
}
