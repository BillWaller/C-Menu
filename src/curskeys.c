//  curskeys.c
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
/// Test Curses Keys

#include "menu.h"
#include <ncursesw/ncurses.h>
#include <unistd.h>

#define KSTRLEN 100
#define MAXLEN 256
int display_curses_keys();

int display_curses_keys() {
    WINDOW *win;
    WINDOW *box;
    char Title[] = "TEST CURSES KEYS";
    int lines = 10;
    int cols = 55;
    char kstr[KSTRLEN];
    unsigned int c;
    char action[MAXLEN];
    char tmp[MAXLEN];
    MEVENT event;
    char *s;
    int maxy, maxx;

    getmaxyx(stdscr, maxy, maxx);
    int begy = (maxy - lines) / 3;
    int begx = (maxx - cols) / 2;

    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION | BUTTON_SHIFT |
                  BUTTON_CTRL | BUTTON_ALT,
              NULL);
    if (win_new(lines, cols, begy, begx, Title, 0)) {
        strnz__cpy(tmp, "win_new failed: ", MAXLEN - 1);
        strnz__cat(tmp, Title, MAXLEN - 1);
        Perror(tmp);
        return 0;
    }
    win = win_win[win_ptr];
    box = win_box[win_ptr];
    keypad(win, TRUE);
    wattron(win, WA_REVERSE);
    mvwaddstr(win, lines - 1, 0, " <ALT>END to exit ");
    wattroff(win, WA_REVERSE);
    wnoutrefresh(box);
    mvwaddstr(win, 1, 4, "Press a key or activate the mouse:");
    timeout(1000);
    c = '\0';
    while (!c) {
        kstr[0] = '\0';
        wmove(win, 1, 39);
        c = xwgetch(win);
        s = keybound(c, 0);
        switch (c) {
        case '\0':
            break;
        case KEY_CODE_YES:
            strnz__cpy(kstr, "KEY_CODE_YES A wchar_t contains a key code",
                       KSTRLEN - 1);
            break;
        case KEY_BREAK:
            strnz__cpy(kstr, "KEY_BREAK Break key(unreliable)", KSTRLEN - 1);
            break;
        case KEY_DOWN:
            strnz__cpy(kstr, "KEY_DOWN down - arrow key", KSTRLEN - 1);
            break;
        case KEY_UP:
            strnz__cpy(kstr, "KEY_UP up - arrow key", KSTRLEN - 1);
            break;
        case KEY_LEFT:
            strnz__cpy(kstr, "KEY_LEFT left - arrow key", KSTRLEN - 1);
            break;
        case KEY_RIGHT:
            strnz__cpy(kstr, "KEY_RIGHT right - arrow key", KSTRLEN - 1);
            break;
        case KEY_HOME:
            strnz__cpy(kstr, "KEY_HOME home key", KSTRLEN - 1);
            break;
        case KEY_BACKSPACE:
            strnz__cpy(kstr, "KEY_BACKSPACE backspace key", KSTRLEN - 1);
            break;
        case KEY_F(1):
            strnz__cpy(kstr, "KEY_F(1) KEY_F(1)", KSTRLEN - 1);
            break;
        case KEY_F(2):
            strnz__cpy(kstr, "KEY_F2 KEY_F2", KSTRLEN - 1);
            break;
        case KEY_F(3):
            strnz__cpy(kstr, "KEY_F3 KEY_F3", KSTRLEN - 1);
            break;
        case KEY_F(4):
            strnz__cpy(kstr, "KEY_F4 KEY_F4", KSTRLEN - 1);
            break;
        case KEY_F(5):
            strnz__cpy(kstr, "KEY_F5 KEY_F5", KSTRLEN - 1);
            break;
        case KEY_F(6):
            strnz__cpy(kstr, "KEY_F6 KEY_F6", KSTRLEN - 1);
            break;
        case KEY_F(7):
            strnz__cpy(kstr, "KEY_F7 KEY_F7", KSTRLEN - 1);
            break;
        case KEY_F(8):
            strnz__cpy(kstr, "KEY_F8 KEY_F8", KSTRLEN - 1);
            break;
        case KEY_F(9):
            strnz__cpy(kstr, "KEY_F9 KEY_F9", KSTRLEN - 1);
            break;
        case KEY_F(10):
            strnz__cpy(kstr, "KEY_F10 KEY_F10", KSTRLEN - 1);
            break;
        case KEY_F(11):
            strnz__cpy(kstr, "KEY_F11 KEY_F11", KSTRLEN - 1);
            break;
        case KEY_F(12):
            strnz__cpy(kstr, "KEY_F12 KEY_F12", KSTRLEN - 1);
            break;
        case KEY_DL:
            strnz__cpy(kstr, "KEY_DL delete - line key", KSTRLEN - 1);
            break;
        case KEY_IL:
            strnz__cpy(kstr, "KEY_IL insert - line key", KSTRLEN - 1);
            break;
        case KEY_DC:
            strnz__cpy(kstr, "KEY_DC delete - character key", KSTRLEN - 1);
            break;
        case KEY_IC:
            strnz__cpy(kstr, "KEY_IC insert - character key", KSTRLEN - 1);
            break;
        case KEY_EIC:
            strnz__cpy(kstr, "KEY_EIC sent by rmir or smir in insert mode",
                       KSTRLEN - 1);
            break;
        case KEY_CLEAR:
            strnz__cpy(kstr, "KEY_CLEAR clear - screen or erase key",
                       KSTRLEN - 1);
            break;
        case KEY_EOS:
            strnz__cpy(kstr, "KEY_EOS clear - to - end - of - screen key",
                       KSTRLEN - 1);
            break;
        case KEY_EOL:
            strnz__cpy(kstr, "KEY_EOL clear - to - end - of - line key",
                       KSTRLEN - 1);
            break;
        case KEY_SF:
            strnz__cpy(kstr, "KEY_SF scroll - forward key", KSTRLEN - 1);
            break;
        case KEY_SR:
            strnz__cpy(kstr, "KEY_SR scroll - backward key", KSTRLEN - 1);
            break;
        case KEY_NPAGE:
            strnz__cpy(kstr, "KEY_NPAGE next - page key", KSTRLEN - 1);
            break;
        case KEY_PPAGE:
            strnz__cpy(kstr, "KEY_PPAGE previous - page key", KSTRLEN - 1);
            break;
        case KEY_STAB:
            strnz__cpy(kstr, "KEY_STAB set - tab key", KSTRLEN - 1);
            break;
        case KEY_CTAB:
            strnz__cpy(kstr, "KEY_CTAB clear - tab key", KSTRLEN - 1);
            break;
        case KEY_CATAB:
            strnz__cpy(kstr, "KEY_CATAB clear - all - tabs key", KSTRLEN - 1);
            break;
        case '\n':
            strnz__cpy(kstr, "KEY_ENTER newline key", KSTRLEN - 1);
            break;
        case KEY_ENTER:
            strnz__cpy(kstr, "KEY_ENTER enter / send key", KSTRLEN - 1);
            break;
        case KEY_SRESET:
            strnz__cpy(kstr, "KEY_SRESET Soft(partial) reset(unreliable)",
                       KSTRLEN - 1);
            break;
        case KEY_RESET:
            strnz__cpy(kstr, "KEY_RESET Reset or hard reset(unreliable)",
                       KSTRLEN - 1);
            break;
        case KEY_PRINT:
            strnz__cpy(kstr, "KEY_PRINT print key", KSTRLEN - 1);
            break;
        case KEY_LL:
            strnz__cpy(kstr, "KEY_LL lower - left key(home down)", KSTRLEN - 1);
            break;
        case KEY_A1:
            strnz__cpy(kstr, "KEY_A1 upper left of keypad", KSTRLEN - 1);
            break;
        case KEY_A3:
            strnz__cpy(kstr, "KEY_A3 upper right of keypad", KSTRLEN - 1);
            break;
        case KEY_B2:
            strnz__cpy(kstr, "KEY_B2 center of keypad", KSTRLEN - 1);
            break;
        case KEY_C1:
            strnz__cpy(kstr, "KEY_C1 lower left of keypad", KSTRLEN - 1);
            break;
        case KEY_C3:
            strnz__cpy(kstr, "KEY_C3 lower right of keypad", KSTRLEN - 1);
            break;
        case KEY_BTAB:
            strnz__cpy(kstr, "KEY_BTAB back - tab key", KSTRLEN - 1);
            break;
        case KEY_BEG:
            strnz__cpy(kstr, "KEY_BEG begin key", KSTRLEN - 1);
            break;
        case KEY_CANCEL:
            strnz__cpy(kstr, "KEY_CANCEL cancel key", KSTRLEN - 1);
            break;
        case KEY_CLOSE:
            strnz__cpy(kstr, "KEY_CLOSE close key", KSTRLEN - 1);
            break;
        case KEY_COMMAND:
            strnz__cpy(kstr, "KEY_COMMAND command key", KSTRLEN - 1);
            break;
        case KEY_COPY:
            strnz__cpy(kstr, "KEY_COPY copy key", KSTRLEN - 1);
            break;
        case KEY_CREATE:
            strnz__cpy(kstr, "KEY_CREATE create key", KSTRLEN - 1);
            break;
        case KEY_END:
            strnz__cpy(kstr, "KEY_END end key", KSTRLEN - 1);
            break;
        case KEY_EXIT:
            strnz__cpy(kstr, "KEY_EXIT exit key", KSTRLEN - 1);
            break;
        case KEY_FIND:
            strnz__cpy(kstr, "KEY_FIND find key", KSTRLEN - 1);
            break;
        case KEY_HELP:
            strnz__cpy(kstr, "KEY_HELP help key", KSTRLEN - 1);
            break;
        case KEY_MARK:
            strnz__cpy(kstr, "KEY_MARK mark key", KSTRLEN - 1);
            break;
        case KEY_MESSAGE:
            strnz__cpy(kstr, "KEY_MESSAGE message key", KSTRLEN - 1);
            break;
        case KEY_MOVE:
            strnz__cpy(kstr, "KEY_MOVE move key", KSTRLEN - 1);
            break;
        case KEY_NEXT:
            strnz__cpy(kstr, "KEY_NEXT next key", KSTRLEN - 1);
            break;
        case KEY_OPEN:
            strnz__cpy(kstr, "KEY_OPEN open key", KSTRLEN - 1);
            break;
        case KEY_OPTIONS:
            strnz__cpy(kstr, "KEY_OPTIONS options key", KSTRLEN - 1);
            break;
        case KEY_PREVIOUS:
            strnz__cpy(kstr, "KEY_PREVIOUS previous key", KSTRLEN - 1);
            break;
        case KEY_REDO:
            strnz__cpy(kstr, "KEY_REDO redo key", KSTRLEN - 1);
            break;
        case KEY_REFERENCE:
            strnz__cpy(kstr, "KEY_REFERENCE reference key", KSTRLEN - 1);
            break;
        case KEY_REFRESH:
            strnz__cpy(kstr, "KEY_REFRESH refresh key", KSTRLEN - 1);
            break;
        case KEY_REPLACE:
            strnz__cpy(kstr, "KEY_REPLACE replace key", KSTRLEN - 1);
            break;
        case KEY_RESTART:
            strnz__cpy(kstr, "KEY_RESTART restart key", KSTRLEN - 1);
            break;
        case KEY_RESUME:
            strnz__cpy(kstr, "KEY_RESUME resume key", KSTRLEN - 1);
            break;
        case KEY_SAVE:
            strnz__cpy(kstr, "KEY_SAVE save key", KSTRLEN - 1);
            break;
        case KEY_SBEG:
            strnz__cpy(kstr, "KEY_SBEG shifted begin key", KSTRLEN - 1);
            break;
        case KEY_SCANCEL:
            strnz__cpy(kstr, "KEY_SCANCEL shifted cancel key", KSTRLEN - 1);
            break;
        case KEY_SCOMMAND:
            strnz__cpy(kstr, "KEY_SCOMMAND shifted command key", KSTRLEN - 1);
            break;
        case KEY_SCOPY:
            strnz__cpy(kstr, "KEY_SCOPY shifted copy key", KSTRLEN - 1);
            break;
        case KEY_SCREATE:
            strnz__cpy(kstr, "KEY_SCREATE shifted create key", KSTRLEN - 1);
            break;
        case KEY_SDC:
            strnz__cpy(kstr, "KEY_SDC shifted delete - character key",
                       KSTRLEN - 1);
            break;
        case KEY_SDL:
            strnz__cpy(kstr, "KEY_SDL shifted delete - line key", KSTRLEN - 1);
            break;
        case KEY_SELECT:
            strnz__cpy(kstr, "KEY_SELECT select key", KSTRLEN - 1);
            break;
        case KEY_SEND:
            strnz__cpy(kstr, "KEY_SEND shifted end key", KSTRLEN - 1);
            break;
        case KEY_SEOL:
            strnz__cpy(kstr,
                       "KEY_SEOL shifted clear - to - end - of - line key",
                       KSTRLEN - 1);
            break;
        case KEY_SEXIT:
            strnz__cpy(kstr, "KEY_SEXIT shifted exit key", KSTRLEN - 1);
            break;
        case KEY_SFIND:
            strnz__cpy(kstr, "KEY_SFIND shifted find key", KSTRLEN - 1);
            break;
        case KEY_SHELP:
            strnz__cpy(kstr, "KEY_SHELP shifted help key", KSTRLEN - 1);
            break;
        case KEY_SHOME:
            strnz__cpy(kstr, "KEY_SHOME shifted home key", KSTRLEN - 1);
            break;
        case KEY_SIC:
            strnz__cpy(kstr, "KEY_SIC shifted insert - character key",
                       KSTRLEN - 1);
            break;
        case KEY_SLEFT:
            strnz__cpy(kstr, "KEY_SLEFT shifted left - arrow key", KSTRLEN - 1);
            break;
        case KEY_SMESSAGE:
            strnz__cpy(kstr, "KEY_SMESSAGE shifted message key", KSTRLEN - 1);
            break;
        case KEY_SMOVE:
            strnz__cpy(kstr, "KEY_SMOVE shifted move key", KSTRLEN - 1);
            break;
        case KEY_SNEXT:
            strnz__cpy(kstr, "KEY_SNEXT shifted next key", KSTRLEN - 1);
            break;
        case KEY_SOPTIONS:
            strnz__cpy(kstr, "KEY_SOPTIONS shifted options key", KSTRLEN - 1);
            break;
        case KEY_SPREVIOUS:
            strnz__cpy(kstr, "KEY_SPREVIOUS shifted previous key", KSTRLEN - 1);
            break;
        case KEY_SPRINT:
            strnz__cpy(kstr, "KEY_SPRINT shifted print key", KSTRLEN - 1);
            break;
        case KEY_SREDO:
            strnz__cpy(kstr, "KEY_SREDO shifted redo key", KSTRLEN - 1);
            break;
        case KEY_SREPLACE:
            strnz__cpy(kstr, "KEY_SREPLACE shifted replace key", KSTRLEN - 1);
            break;
        case KEY_SRIGHT:
            strnz__cpy(kstr, "KEY_SRIGHT shifted right - arrow key",
                       KSTRLEN - 1);
            break;
        case KEY_SRSUME:
            strnz__cpy(kstr, "KEY_SRSUME shifted resume key", KSTRLEN - 1);
            break;
        case KEY_SSAVE:
            strnz__cpy(kstr, "KEY_SSAVE shifted save key", KSTRLEN - 1);
            break;
        case KEY_SSUSPEND:
            strnz__cpy(kstr, "KEY_SSUSPEND shifted suspend key", KSTRLEN - 1);
            break;
        case KEY_SUNDO:
            strnz__cpy(kstr, "KEY_SUNDO shifted undo key", KSTRLEN - 1);
            break;
        case KEY_SUSPEND:
            strnz__cpy(kstr, "KEY_SUSPEND suspend key", KSTRLEN - 1);
            break;
        case KEY_UNDO:
            strnz__cpy(kstr, "KEY_UNDO undo key", KSTRLEN - 1);
            break;
        case KEY_ALTDEL:
            strnz__cpy(kstr, "KEY_ALTDEL alt+delete", KSTRLEN - 1);
            break;
        case KEY_ALTDOWN:
            strnz__cpy(kstr, "KEY_ALTDOWN alt+down", KSTRLEN - 1);
            break;
        case KEY_ALTEND:
            strnz__cpy(kstr, "KEY_ALTEND alt+end", KSTRLEN - 1);
            break;
        case KEY_ALTHOME:
            strnz__cpy(kstr, "KEY_ALTHOME alt+home", KSTRLEN - 1);
            break;
        case KEY_ALTINS:
            strnz__cpy(kstr, "KEY_ALTINS alt+ins", KSTRLEN - 1);
            break;
        case KEY_ALTLEFT:
            strnz__cpy(kstr, "KEY_ALTLEFT alt+left", KSTRLEN - 1);
            break;
        case KEY_ALTPGDN:
            strnz__cpy(kstr, "KEY_ALTPGDN alt+pgdn", KSTRLEN - 1);
            break;
        case KEY_ALTPGUP:
            strnz__cpy(kstr, "KEY_ALTPGUP alt+pgup", KSTRLEN - 1);
            break;
        case KEY_ALTRIGHT:
            strnz__cpy(kstr, "KEY_ALTRIGHT alt+right", KSTRLEN - 1);
            break;
        case KEY_ALTUP:
            strnz__cpy(kstr, "KEY_ALTUP alt+up", KSTRLEN - 1);
            break;
        case KEY_ALTF(1):
            strnz__cpy(kstr, "KEY_ALTF1 alt+F1", KSTRLEN - 1);
            break;
        case KEY_ALTF(2):
            strnz__cpy(kstr, "KEY_ALTF1 alt+F2", KSTRLEN - 1);
            break;
        case KEY_ALTF(3):
            strnz__cpy(kstr, "KEY_ALTF3 alt+F3", KSTRLEN - 1);
            break;
        case KEY_ALTF(4):
            strnz__cpy(kstr, "KEY_ALTF4 alt+F4", KSTRLEN - 1);
            break;
        case KEY_ALTF(5):
            strnz__cpy(kstr, "KEY_ALTF5 alt+F5", KSTRLEN - 1);
            break;
        case KEY_ALTF(6):
            strnz__cpy(kstr, "KEY_ALTF6 alt+F6", KSTRLEN - 1);
            break;
        case KEY_ALTF(7):
            strnz__cpy(kstr, "KEY_ALTF7 alt+F7", KSTRLEN - 1);
            break;
        case KEY_ALTF(8):
            strnz__cpy(kstr, "KEY_ALTF8 alt+F8", KSTRLEN - 1);
            break;
        case KEY_ALTF(9):
            strnz__cpy(kstr, "KEY_ALTF9 alt+F9", KSTRLEN - 1);
            break;
        case KEY_ALTF(10):
            strnz__cpy(kstr, "KEY_ALTF10 alt+F10", KSTRLEN - 1);
            break;
        case KEY_ALTF(11):
            strnz__cpy(kstr, "KEY_ALTF11 alt+F11", KSTRLEN - 1);
            break;
        case KEY_ALTF(12):
            strnz__cpy(kstr, "KEY_ALTF12 alt+F12", KSTRLEN - 1);
            break;
            //  ╭───────────────────────────────────────────────────╮
            //  │ MOUSE FUNCTIONS                                   │
            //  ╰───────────────────────────────────────────────────╯
        case KEY_MOUSE:
            action[0] = '\0';
            if (getmouse(&event) == OK) {
                switch (event.bstate) {
                case BUTTON1_PRESSED:
                    strnz__cpy(action, "Button 1 pressed", KSTRLEN - 1);
                    break;
                case BUTTON1_RELEASED:
                    strnz__cpy(action, "Button 1 released", KSTRLEN - 1);
                    break;
                case BUTTON1_CLICKED:
                    strnz__cpy(action, "Button 1 clicked", KSTRLEN - 1);
                    break;
                case BUTTON1_DOUBLE_CLICKED:
                    strnz__cpy(action, "Button 1 double-clicked", KSTRLEN - 1);
                    break;
                case BUTTON2_PRESSED:
                    strnz__cpy(action, "Button 2 pressed", KSTRLEN - 1);
                    break;
                case BUTTON2_RELEASED:
                    strnz__cpy(action, "Button 2 released", KSTRLEN - 1);
                    break;
                case BUTTON2_CLICKED:
                    strnz__cpy(action, "Button 2 clicked", KSTRLEN - 1);
                    break;
                case BUTTON2_DOUBLE_CLICKED:
                    strnz__cpy(action, "Button 2 double-clicked", KSTRLEN - 1);
                    break;
                case BUTTON3_PRESSED:
                    strnz__cpy(action, "Button 3 pressed", KSTRLEN - 1);
                    break;
                case BUTTON3_RELEASED:
                    strnz__cpy(action, "Button 3 released", KSTRLEN - 1);
                    break;
                case BUTTON3_CLICKED:
                    strnz__cpy(action, "Button 3 clicked", KSTRLEN - 1);
                    break;
                case BUTTON3_DOUBLE_CLICKED:
                    strnz__cpy(action, "Button 3 double-clicked", KSTRLEN - 1);
                    break;
                case BUTTON4_PRESSED:
                    strnz__cpy(action, "Button 4 pressed", KSTRLEN - 1);
                    break;
                case BUTTON4_RELEASED:
                    strnz__cpy(action, "Button 4 released", KSTRLEN - 1);
                    break;
                case BUTTON4_CLICKED:
                    strnz__cpy(action, "Button 4 clicked", KSTRLEN - 1);
                    break;
                case BUTTON4_DOUBLE_CLICKED:
                    strnz__cpy(action, "Button 4 double-clicked", KSTRLEN - 1);
                    break;
                case BUTTON5_PRESSED:
                    strnz__cpy(action, "Scroll Up", KSTRLEN - 1);
                    break;
                default:
                    break;
                }
                mvwaddstr(win, 6, 3, "     Action:");
                mvwaddstr(win, 6, 16, action);
                wclrtoeol(win);

                strnz__cpy(tmp, "  With Key:", MAXLEN - 1);
                if (event.bstate & BUTTON_SHIFT)
                    strnz__cat(tmp, " Shift", MAXLEN - 1);
                if (event.bstate & BUTTON_CTRL)
                    strnz__cat(tmp, " Ctrl", MAXLEN - 1);
                if (event.bstate & BUTTON_ALT)
                    strnz__cat(tmp, " Alt", MAXLEN - 1);
                mvwaddstr(win, 3, 4, tmp);
                wclrtoeol(win);
                if (wenclose(win, event.y, event.x)) {
                    sprintf(tmp, "   Inside Win:  y: %3d, x: %3d",
                            event.y - begy, event.x - begx);
                } else {
                    sprintf(tmp, "  Outside Win:  y: %3d, x: %3d",
                            event.y - begy, event.x - begx);
                }
                mvwaddstr(win, 4, 4, tmp);
                wclrtoeol(win);
                wmove(win, 5, 0);
                wclrtoeol(win);
                wmove(win, 7, 0);
                wclrtoeol(win);
            }
            break;
        default:
            break;
        }
        if (c != KEY_MOUSE) {

            sprintf(tmp, "     Octal: %3o", c);
            mvwaddstr(win, 3, 4, tmp);
            wclrtoeol(win);

            sprintf(tmp, "   Decimal: %3d", c);
            mvwaddstr(win, 4, 4, tmp);
            wclrtoeol(win);

            sprintf(tmp, "       Hex: %3x", c);
            mvwaddstr(win, 5, 4, tmp);
            wclrtoeol(win);

            if (kstr[0]) {
                sprintf(tmp, "Description: %s", kstr);
            } else {
                sprintf(tmp, "      ASCII: %c", c);
            }
            mvwaddstr(win, 6, 3, tmp);
            wclrtoeol(win);
            mvwaddstr(win, 7, 2, "Key bound To: ");
            wclrtoeol(win);
            mvwaddstr(win, 7, 16, s ? s : "Not Bound");
            wrefresh(win);
        }
        if (c == KEY_ALTEND) {
            usleep(100000);
            break;
        }
        c = '\0';
    }
    wclear(win);
    wrefresh(win);
    clear();
    refresh();
    win_del();
    return 0;
}
