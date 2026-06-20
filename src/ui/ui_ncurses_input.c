#include "ui_ncurses_internal.h"
#include <string.h>

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

int ui_get_event(UiRuntime *ui, UiSurface *target, UiEvent *ev, int timeout_ms) {
    int ch;
    WINDOW *win;

    (void)ui;
    if (!ev)
        return -1;
    memset(ev, 0, sizeof(*ev));

    win = target ? target->win : stdscr;

    if (timeout_ms < 0) {
        wtimeout(win, -1);
    } else {
        wtimeout(win, timeout_ms);
    }

    ch = wgetch(win);
    ev->key = translate_key(ch);

    if (ev->key == UI_KEY_CHAR) {
        ev->ch = (uint32_t)ch;
    } else if (ev->key == UI_KEY_MOUSE) {
        MEVENT me;
        if (getmouse(&me) == OK) {
            ev->y = me.y;
            ev->x = me.x;
#ifdef BUTTON4_PRESSED
            if (me.bstate & BUTTON4_PRESSED) {
                ev->mouse_action = UI_MOUSE_SCROLL_UP;
#endif
#ifdef BUTTON5_PRESSED
            } else if (me.bstate & BUTTON5_PRESSED) {
                ev->mouse_action = UI_MOUSE_SCROLL_DOWN;
#endif
            } else if (me.bstate & BUTTON1_PRESSED) {
                ev->mouse_action = UI_MOUSE_PRESS;
            } else if (me.bstate & BUTTON1_RELEASED) {
                ev->mouse_action = UI_MOUSE_RELEASE;
            }
        }
    }

    return 0;
}
