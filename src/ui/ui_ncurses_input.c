/** @file ui_ncurses_input.c
   @ingroup ui_ncurses
   @brief NCurses UI backend — input handling.

   Translates raw NCurses key codes and mouse events into the portable
   UiEvent representation defined in ui_backend.h.
*/

#include "ui_ncurses_internal.h"
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
int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev,
                 int timeout_ms) {
    (void)ui;
    char *keybound_p = NULL;
    if (!ev)
        return -1;
    memset(ev, 0, sizeof(*ev));

    WINDOW *win = target ? target->win : stdscr;
    keypad(win, true);
    timeout_ms = timeout_ms < -1 ? -1 : timeout_ms;
    // wtimeout(win, timeout_ms);

    int ch = wgetch(win);
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
            if (wenclose(target->win, me.y, me.x)) {
                ev->mouse_inside = true;
            } else {
                ev->mouse_inside = false;
            }
        }
    }
    keybound_p = keybound(ch, 0);
    strncpy(ev->keybound, keybound_p, sizeof(ev->keybound) - 1);
    return ch;
}
