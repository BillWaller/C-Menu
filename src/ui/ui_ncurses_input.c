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

/* -------------------------------------------------------------------------
   Key translation
   ------------------------------------------------------------------------- */

static UiKey translate_key(int ch) {
    switch (ch) {
    case ERR:
        return UI_KEY_NONE;
    case '\n':
    case '\r':
        return UI_KEY_ENTER;
    case 27:
        return UI_KEY_ESCAPE;
    case '\t':
        return UI_KEY_TAB;
#ifdef KEY_BTAB
    case KEY_BTAB:
        return UI_KEY_BTAB;
#endif
    case KEY_UP:
        return UI_KEY_UP;
    case KEY_DOWN:
        return UI_KEY_DOWN;
    case KEY_LEFT:
        return UI_KEY_LEFT;
    case KEY_RIGHT:
        return UI_KEY_RIGHT;
    case KEY_HOME:
        return UI_KEY_HOME;
    case KEY_END:
        return UI_KEY_END;
    case KEY_PPAGE:
        return UI_KEY_PGUP;
    case KEY_NPAGE:
        return UI_KEY_PGDN;
    case KEY_IC:
        return UI_KEY_INSERT;
    case KEY_DC:
        return UI_KEY_DELETE;
    case KEY_BACKSPACE:
        return UI_KEY_BACKSPACE;
#ifdef KEY_RESIZE
    case KEY_RESIZE:
        return UI_KEY_RESIZE;
#endif
    case KEY_MOUSE:
        return UI_KEY_MOUSE;
    case KEY_F(1):
        return UI_KEY_F1;
    case KEY_F(2):
        return UI_KEY_F2;
    case KEY_F(3):
        return UI_KEY_F3;
    case KEY_F(4):
        return UI_KEY_F4;
    case KEY_F(5):
        return UI_KEY_F5;
    case KEY_F(6):
        return UI_KEY_F6;
    case KEY_F(7):
        return UI_KEY_F7;
    case KEY_F(8):
        return UI_KEY_F8;
    case KEY_F(9):
        return UI_KEY_F9;
    case KEY_F(10):
        return UI_KEY_F10;
    case KEY_F(11):
        return UI_KEY_F11;
    case KEY_F(12):
        return UI_KEY_F12;
    default:
        if (ch >= 32 && ch <= 126)
            return UI_KEY_CHAR;
        return UI_KEY_NONE;
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
int ui_get_event(UiRuntime *ui, UiSurface *sfc, int w, UiEvent *ev,
                 int timeout_ms) {
    (void)ui;
    if (!ev)
        return -1;
    memset(ev, 0, sizeof(*ev));
    qiflush();
    curs_set(1);
    if (timeout_ms <= 0) {
        timeout_ms = -1;
    } else
        wtimeout(sfc->mwin[w], timeout_ms);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION | BUTTON_SHIFT | BUTTON_CTRL | BUTTON_ALT, NULL);
    int ch = wgetch(sfc->mwin[w]);
    ev->key = translate_key(ch);
    if (ev->key == UI_KEY_CHAR) {
        ev->ch = (uint32_t)ch;
    } else if (ev->key == UI_KEY_MOUSE) {
        MEVENT me;
        if (getmouse(&me) == OK) {
            ev->y = me.y;
            ev->x = me.x;
#ifdef BUTTON4_PRESSED
            if (me.bstate & BUTTON4_PRESSED)
                ev->mouse_action = UI_MOUSE_SCROLL_UP;
            else
#endif
#ifdef BUTTON5_PRESSED
                if (me.bstate & BUTTON5_PRESSED)
                ev->mouse_action = UI_MOUSE_SCROLL_DOWN;
            else
#endif
                if (me.bstate & BUTTON1_PRESSED)
                ev->mouse_action = UI_MOUSE_PRESS;
            else if (me.bstate & BUTTON1_RELEASED)
                ev->mouse_action = UI_MOUSE_RELEASE;
            if (wenclose(sfc->mwin[w], me.y, me.x) &&
                wmouse_trafo(sfc->mwin[w], &me.y, &me.x, false)) {
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

int ui_get_event_multi(UiRuntime *ui, UiSurface *sfc, int w, UiEvent *ev, int timeout_ms) {
    (void)ui;
    int i;
    if (!ev)
        return -1;
    memset(ev, 0, sizeof(*ev));
    keypad(sfc->mwin[w], true);
    if (timeout_ms <= 0) {
        timeout_ms = -1;
    } else
        wtimeout(sfc->mwin[w], timeout_ms);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION | BUTTON_SHIFT | BUTTON_CTRL | BUTTON_ALT, NULL);
    ev->chyron = -1;
    qiflush();
    cbreak();
    curs_set(1);
    int ch = wgetch(sfc->mwin[w]);
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
            else if (me.bstate & BUTTON1_CLICKED || me.bstate & BUTTON1_DOUBLE_CLICKED) {
                ev->mouse_action = UI_MOUSE_PRESS;
                for (i = 0; i < 4; i++) {
                    if (sfc->mwin[i] != NULL &&
                        wenclose(sfc->mwin[i], me.y, me.x) &&
                        wmouse_trafo(sfc->mwin[i], &me.y, &me.x, false)) {
                        ev->in_win = i;
                        ev->key = 0;
                        break;
                    }
                }
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

int ui_get_event_no_mouse(UiSurface *surface, int w, UiEvent *ev) {
    int ch;
    mousemask(0, NULL);

    qiflush();
    curs_set(1);
    cbreak();
    do {
        ch = wgetch(surface->mwin[w]);
        ev->key = translate_key(ch);
        if (ev->key == UI_KEY_CHAR) {
            ev->ch = (uint32_t)ch;
            break;
        }
    } while (ch == ERR);
    curs_set(0);
    return ch;
}
