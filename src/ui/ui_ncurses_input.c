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
int ui_get_event(UiSurface *s, uint w, UiEvent *ev, int timeout_ms) {
    if (!ev)
        return -1;
    memset(ev, 0, sizeof(*ev));
    tcflush(2, TCIFLUSH);
    if (timeout_ms <= 0) {
        timeout_ms = -1;
    } else
        wtimeout(s->mwin[w], timeout_ms);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION | BUTTON_SHIFT | BUTTON_CTRL | BUTTON_ALT, NULL);
    curs_set(2);
    int ch = wgetch(s->mwin[w]);
    curs_set(0);
    ev->key = translate_key(ch);
    if (ev->key == UI_KEY_CHAR) {
        ev->ch = (uint32_t)ch;
    } else if (ev->key == UI_KEY_MOUSE) {
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
    }
    return ch;
}

int ui_get_event_multi(UiSurface *s, uint w, UiEvent *ev, int timeout_ms) {
    int i;
    if (!ev)
        return -1;
    memset(ev, 0, sizeof(*ev));
    keypad(s->mwin[w], true);
    if (timeout_ms <= 0) {
        timeout_ms = -1;
    } else
        wtimeout(s->mwin[w], timeout_ms);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION | BUTTON_SHIFT | BUTTON_CTRL | BUTTON_ALT, NULL);
    ev->chyron = -1;
    qiflush();
    tcflush(2, TCIFLUSH);
    cbreak();
    curs_set(2);
    int ch = wgetch(s->mwin[w]);
    curs_set(0);
    ev->mouse_action = UI_MOUSE_NONE;
    ev->key = translate_key(ch);
    if (ev->key == UI_KEY_CHAR) {
        ev->ch = (uint32_t)ch;
    } else if (ev->key == UI_KEY_MOUSE) {
        MEVENT me;
        if (getmouse(&me) == OK) {
            ev->y = me.y;
            ev->x = me.x;
            if (me.bstate & BUTTON4_PRESSED)
                ev->mouse_action = UI_MOUSE_SCROLL_UP;
            else if (me.bstate & BUTTON5_PRESSED)
                ev->mouse_action = UI_MOUSE_SCROLL_DOWN;
            else if (me.bstate & BUTTON1_CLICKED ||
                     me.bstate & BUTTON1_PRESSED ||
                     me.bstate & BUTTON1_DOUBLE_CLICKED) {
                ev->mouse_action = UI_MOUSE_PRESS;
                ev->in_win = -1;
                for (i = WIN; i < SUB_SFC_MAX; i++) {
                    if (s->mwin[i] != NULL &&
                        wenclose(s->mwin[i], me.y, me.x) &&
                        wmouse_trafo(s->mwin[i], &me.y, &me.x, false)) {
                        ev->in_win = i;
                        ev->key = 0;
                        break;
                    }
                }
                ev->bstate = me.bstate;
                ev->y = me.y;
                ev->x = me.x;
                ev->key = 0;
                return 0;
            }
        }
    }
    curs_set(0);
    return ch;
}

int ui_get_event_no_mouse(UiSurface *s, uint w, UiEvent *ev) {
    int ch;
    mousemask(0, NULL);

    qiflush();
    tcflush(2, TCIFLUSH);
    cbreak();
    do {
        curs_set(2);
        ch = wgetch(s->mwin[w]);
        curs_set(0);
        ev->key = translate_key(ch);
        if (ev->key == UI_KEY_CHAR) {
            ev->ch = (uint32_t)ch;
            break;
        }
    } while (ch == ERR);
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
