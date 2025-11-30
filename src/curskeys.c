/* fckeys.c
 * Test Curses Keys
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <ncursesw/ncurses.h>
#include <string.h>
#include <unistd.h>

int display_curses_keys();

int display_curses_keys() {
    WINDOW *win;
    WINDOW *box;
    char Title[] = "TEST CURSES KEYS";
    int lines = 9;
    int cols = 55;
    char kstr[64];
    int c;
    char action[MAXLEN];
    char tmp[MAXLEN];
    MEVENT event;
    /*
       typedef struct {
           short id;
           int x, y, z;
           mmask_t bstate;
       } MEVENT;
     */
    int maxy, maxx;
    // int winy, winx;

    getmaxyx(stdscr, maxy, maxx);
    int begy = (maxy - lines) / 3;
    int begx = (maxx - cols) / 2;

    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION | BUTTON_SHIFT |
                  BUTTON_CTRL | BUTTON_ALT,
              NULL);
    if (win_new(lines, cols, begy, begx, Title)) {
        strncpy(tmp, "win_new failed: ", MAXLEN - 1);
        strncat(tmp, Title, MAXLEN - 1);
        display_error_message(tmp);
        return (-1);
    }
    win = win_win[win_ptr];
    box = win_box[win_ptr];
    keypad(win, TRUE);
    wattron(win, A_REVERSE);
    mvwaddstr(win, lines - 1, 0, " ESC to exit ");
    wattroff(win, A_REVERSE);
    wnoutrefresh(box);
    mvwaddstr(win, 1, 4, "Press a key or activate the mouse:");
    c = '\0';
    while (!c) {
        kstr[0] = '\0';
        c = mvwgetch(win, 1, 39);
        switch (c) {
        case '\0':
            break;
        case '\033':
            strcpy(kstr, "0033 Escape Key");
            break;
        case KEY_CODE_YES:
            strcpy(kstr, "KEY_CODE_YES A wchar_t contains a key code");
            break;
        case KEY_BREAK:
            strcpy(kstr, "KEY_BREAK Break key(unreliable)");
            break;
        case KEY_DOWN:
            strcpy(kstr, "KEY_DOWN down - arrow key");
            break;
        case KEY_UP:
            strcpy(kstr, "KEY_UP up - arrow key");
            break;
        case KEY_LEFT:
            strcpy(kstr, "KEY_LEFT left - arrow key");
            break;
        case KEY_RIGHT:
            strcpy(kstr, "KEY_RIGHT right - arrow key");
            break;
        case KEY_HOME:
            strcpy(kstr, "KEY_HOME home key");
            break;
        case KEY_BACKSPACE:
            strcpy(kstr, "KEY_BACKSPACE backspace key");
            break;
        case KEY_F0:
            strcpy(kstr, "KEY_F0 Function Keys");
            break;
        case KEY_F1:
            strcpy(kstr, "KEY_F1 KEY_F1");
            break;
        case KEY_F2:
            strcpy(kstr, "KEY_F2 KEY_F2");
            break;
        case KEY_F3:
            strcpy(kstr, "KEY_F3 KEY_F3");
            break;
        case KEY_F4:
            strcpy(kstr, "KEY_F4 KEY_F4");
            break;
        case KEY_F5:
            strcpy(kstr, "KEY_F5 KEY_F5");
            break;
        case KEY_F6:
            strcpy(kstr, "KEY_F6 KEY_F6");
            break;
        case KEY_F7:
            strcpy(kstr, "KEY_F7 KEY_F7");
            break;
        case KEY_F8:
            strcpy(kstr, "KEY_F8 KEY_F8");
            break;
        case KEY_F9:
            strcpy(kstr, "KEY_F9 KEY_F9");
            break;
        case KEY_F10:
            strcpy(kstr, "KEY_F10 KEY_F10");
            break;
        case KEY_F11:
            strcpy(kstr, "KEY_F11 KEY_F11");
            break;
        case KEY_F12:
            strcpy(kstr, "KEY_F12 KEY_F12");
            break;
        case KEY_F13:
            strcpy(kstr, "KEY_F13 KEY_F13");
            break;
        case KEY_F14:
            strcpy(kstr, "KEY_F14 KEY_F14");
            break;
        case KEY_DL:
            strcpy(kstr, "KEY_DL delete - line key");
            break;
        case KEY_IL:
            strcpy(kstr, "KEY_IL insert - line key");
            break;
        case KEY_DC:
            strcpy(kstr, "KEY_DC delete - character key");
            break;
        case KEY_IC:
            strcpy(kstr, "KEY_IC insert - character key");
            break;
        case KEY_EIC:
            strcpy(kstr, "KEY_EIC sent by rmir or smir in insert mode");
            break;
        case KEY_CLEAR:
            strcpy(kstr, "KEY_CLEAR clear - screen or erase key");
            break;
        case KEY_EOS:
            strcpy(kstr, "KEY_EOS clear - to - end - of - screen key");
            break;
        case KEY_EOL:
            strcpy(kstr, "KEY_EOL clear - to - end - of - line key");
            break;
        case KEY_SF:
            strcpy(kstr, "KEY_SF scroll - forward key");
            break;
        case KEY_SR:
            strcpy(kstr, "KEY_SR scroll - backward key");
            break;
        case KEY_NPAGE:
            strcpy(kstr, "KEY_NPAGE next - page key");
            break;
        case KEY_PPAGE:
            strcpy(kstr, "KEY_PPAGE previous - page key");
            break;
        case KEY_STAB:
            strcpy(kstr, "KEY_STAB set - tab key");
            break;
        case KEY_CTAB:
            strcpy(kstr, "KEY_CTAB clear - tab key");
            break;
        case KEY_CATAB:
            strcpy(kstr, "KEY_CATAB clear - all - tabs key");
            break;
        case KEY_ENTER:
            strcpy(kstr, "KEY_ENTER enter / send key");
            break;
        case KEY_SRESET:
            strcpy(kstr, "KEY_SRESET Soft(partial) reset(unreliable)");
            break;
        case KEY_RESET:
            strcpy(kstr, "KEY_RESET Reset or hard reset(unreliable)");
            break;
        case KEY_PRINT:
            strcpy(kstr, "KEY_PRINT print key");
            break;
        case KEY_LL:
            strcpy(kstr, "KEY_LL lower - left key(home down)");
            break;
        case KEY_A1:
            strcpy(kstr, "KEY_A1 upper left of keypad");
            break;
        case KEY_A3:
            strcpy(kstr, "KEY_A3 upper right of keypad");
            break;
        case KEY_B2:
            strcpy(kstr, "KEY_B2 center of keypad");
            break;
        case KEY_C1:
            strcpy(kstr, "KEY_C1 lower left of keypad");
            break;
        case KEY_C3:
            strcpy(kstr, "KEY_C3 lower right of keypad");
            break;
        case KEY_BTAB:
            strcpy(kstr, "KEY_BTAB back - tab key");
            break;
        case KEY_BEG:
            strcpy(kstr, "KEY_BEG begin key");
            break;
        case KEY_CANCEL:
            strcpy(kstr, "KEY_CANCEL cancel key");
            break;
        case KEY_CLOSE:
            strcpy(kstr, "KEY_CLOSE close key");
            break;
        case KEY_COMMAND:
            strcpy(kstr, "KEY_COMMAND command key");
            break;
        case KEY_COPY:
            strcpy(kstr, "KEY_COPY copy key");
            break;
        case KEY_CREATE:
            strcpy(kstr, "KEY_CREATE create key");
            break;
        case KEY_END:
            strcpy(kstr, "KEY_END end key");
            break;
        case KEY_EXIT:
            strcpy(kstr, "KEY_EXIT exit key");
            break;
        case KEY_FIND:
            strcpy(kstr, "KEY_FIND find key");
            break;
        case KEY_HELP:
            strcpy(kstr, "KEY_HELP help key");
            break;
        case KEY_MARK:
            strcpy(kstr, "KEY_MARK mark key");
            break;
        case KEY_MESSAGE:
            strcpy(kstr, "KEY_MESSAGE message key");
            break;
        case KEY_MOVE:
            strcpy(kstr, "KEY_MOVE move key");
            break;
        case KEY_NEXT:
            strcpy(kstr, "KEY_NEXT next key");
            break;
        case KEY_OPEN:
            strcpy(kstr, "KEY_OPEN open key");
            break;
        case KEY_OPTIONS:
            strcpy(kstr, "KEY_OPTIONS options key");
            break;
        case KEY_PREVIOUS:
            strcpy(kstr, "KEY_PREVIOUS previous key");
            break;
        case KEY_REDO:
            strcpy(kstr, "KEY_REDO redo key");
            break;
        case KEY_REFERENCE:
            strcpy(kstr, "KEY_REFERENCE reference key");
            break;
        case KEY_REFRESH:
            strcpy(kstr, "KEY_REFRESH refresh key");
            break;
        case KEY_REPLACE:
            strcpy(kstr, "KEY_REPLACE replace key");
            break;
        case KEY_RESTART:
            strcpy(kstr, "KEY_RESTART restart key");
            break;
        case KEY_RESUME:
            strcpy(kstr, "KEY_RESUME resume key");
            break;
        case KEY_SAVE:
            strcpy(kstr, "KEY_SAVE save key");
            break;
        case KEY_SBEG:
            strcpy(kstr, "KEY_SBEG shifted begin key");
            break;
        case KEY_SCANCEL:
            strcpy(kstr, "KEY_SCANCEL shifted cancel key");
            break;
        case KEY_SCOMMAND:
            strcpy(kstr, "KEY_SCOMMAND shifted command key");
            break;
        case KEY_SCOPY:
            strcpy(kstr, "KEY_SCOPY shifted copy key");
            break;
        case KEY_SCREATE:
            strcpy(kstr, "KEY_SCREATE shifted create key");
            break;
        case KEY_SDC:
            strcpy(kstr, "KEY_SDC shifted delete - character key");
            break;
        case KEY_SDL:
            strcpy(kstr, "KEY_SDL shifted delete - line key");
            break;
        case KEY_SELECT:
            strcpy(kstr, "KEY_SELECT select key");
            break;
        case KEY_SEND:
            strcpy(kstr, "KEY_SEND shifted end key");
            break;
        case KEY_SEOL:
            strcpy(kstr, "KEY_SEOL shifted clear - to - end - of - line key");
            break;
        case KEY_SEXIT:
            strcpy(kstr, "KEY_SEXIT shifted exit key");
            break;
        case KEY_SFIND:
            strcpy(kstr, "KEY_SFIND shifted find key");
            break;
        case KEY_SHELP:
            strcpy(kstr, "KEY_SHELP shifted help key");
            break;
        case KEY_SHOME:
            strcpy(kstr, "KEY_SHOME shifted home key");
            break;
        case KEY_SIC:
            strcpy(kstr, "KEY_SIC shifted insert - character key");
            break;
        case KEY_SLEFT:
            strcpy(kstr, "KEY_SLEFT shifted left - arrow key");
            break;
        case KEY_SMESSAGE:
            strcpy(kstr, "KEY_SMESSAGE shifted message key");
            break;
        case KEY_SMOVE:
            strcpy(kstr, "KEY_SMOVE shifted move key");
            break;
        case KEY_SNEXT:
            strcpy(kstr, "KEY_SNEXT shifted next key");
            break;
        case KEY_SOPTIONS:
            strcpy(kstr, "KEY_SOPTIONS shifted options key");
            break;
        case KEY_SPREVIOUS:
            strcpy(kstr, "KEY_SPREVIOUS shifted previous key");
            break;
        case KEY_SPRINT:
            strcpy(kstr, "KEY_SPRINT shifted print key");
            break;
        case KEY_SREDO:
            strcpy(kstr, "KEY_SREDO shifted redo key");
            break;
        case KEY_SREPLACE:
            strcpy(kstr, "KEY_SREPLACE shifted replace key");
            break;
        case KEY_SRIGHT:
            strcpy(kstr, "KEY_SRIGHT shifted right - arrow key");
            break;
        case KEY_SRSUME:
            strcpy(kstr, "KEY_SRSUME shifted resume key");
            break;
        case KEY_SSAVE:
            strcpy(kstr, "KEY_SSAVE shifted save key");
            break;
        case KEY_SSUSPEND:
            strcpy(kstr, "KEY_SSUSPEND shifted suspend key");
            break;
        case KEY_SUNDO:
            strcpy(kstr, "KEY_SUNDO shifted undo key");
            break;
        case KEY_SUSPEND:
            strcpy(kstr, "KEY_SUSPEND suspend key");
            break;
        case KEY_UNDO:
            strcpy(kstr, "KEY_UNDO undo key");
            break;
        case KEY_ALTDEL:
            strcpy(kstr, "KEY_ALTDEL alt+delete");
            break;
        case KEY_ALTDOWN:
            strcpy(kstr, "KEY_ALTDOWN alt+down");
            break;
        case KEY_ALTEND:
            strcpy(kstr, "KEY_ALTEND alt+end");
            break;
        case KEY_ALTHOME:
            strcpy(kstr, "KEY_ALTHOME alt+home");
            break;
        case KEY_ALTINS:
            strcpy(kstr, "KEY_ALTINS alt+ins");
            break;
        case KEY_ALTLEFT:
            strcpy(kstr, "KEY_ALTLEFT alt+left");
            break;
        case KEY_ALTPGDN:
            strcpy(kstr, "KEY_ALTPGDN alt+pgdn");
            break;
        case KEY_ALTPGUP:
            strcpy(kstr, "KEY_ALTPGUP alt+pgup");
            break;
        case KEY_ALTRIGHT:
            strcpy(kstr, "KEY_ALTRIGHT alt+right");
            break;
        case KEY_ALTUP:
            strcpy(kstr, "KEY_ALTUP alt+up");
            break;
        case KEY_MOUSE:
            action[0] = '\0';
            if (getmouse(&event) == OK) {
                switch (event.bstate) {
                case BUTTON1_PRESSED:
                    strcpy(action, "Button 1 pressed");
                    break;
                case BUTTON1_RELEASED:
                    strcpy(action, "Button 1 released");
                    break;
                case BUTTON1_CLICKED:
                    strcpy(action, "Button 1 clicked");
                    break;
                case BUTTON1_DOUBLE_CLICKED:
                    strcpy(action, "Button 1 double-clicked");
                    break;
                case BUTTON2_PRESSED:
                    strcpy(action, "Button 2 pressed");
                    break;
                case BUTTON2_RELEASED:
                    strcpy(action, "Button 2 released");
                    break;
                case BUTTON2_CLICKED:
                    strcpy(action, "Button 2 clicked");
                    break;
                case BUTTON2_DOUBLE_CLICKED:
                    strcpy(action, "Button 2 double-clicked");
                    break;
                case BUTTON3_PRESSED:
                    strcpy(action, "Button 3 pressed");
                    break;
                case BUTTON3_RELEASED:
                    strcpy(action, "Button 3 released");
                    break;
                case BUTTON3_CLICKED:
                    strcpy(action, "Button 3 clicked");
                    break;
                case BUTTON3_DOUBLE_CLICKED:
                    strcpy(action, "Button 3 double-clicked");
                    break;
                case BUTTON4_PRESSED:
                    strcpy(action, "Scroll Down");
                    break;
                case BUTTON5_PRESSED:
                    strcpy(action, "Scroll Up");
                    break;
                default:
                    break;
                }
                mvwaddstr(win, 6, 3, "     Action:");
                mvwaddstr(win, 6, 16, action);
                wclrtoeol(win);

                strcpy(tmp, "  With Key:");
                if (event.bstate & BUTTON_SHIFT)
                    strcat(tmp, " Shift");
                if (event.bstate & BUTTON_CTRL)
                    strcat(tmp, " Ctrl");
                if (event.bstate & BUTTON_ALT)
                    strcat(tmp, " Alt");
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
            wrefresh(win);
        }
        if (c == '\33') {
            usleep(100000);
            break;
        }
        c = '\0';
    }
    wclear(win);
    wrefresh(win);
    win_del();
    return 0;
}
