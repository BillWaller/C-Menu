/* fckeys.c
 * Test Curses Keys
 * Bill Waller
 * billxwaller@gmail.com
 */

#include "menu.h"
#include <ncursesw/ncurses.h>
#include <string.h>
#include <unistd.h>

int ckeys(int, int);
void curseskeys(WINDOW *win);

int ckeys(int begy, int begx) {
    WINDOW *win;
    WINDOW *box;
    char Title[] = "TEST CURSES KEYS";
    int lines = 9;
    int cols = 50;
    if (win_new(lines, cols, begy, begx, Title)) {
        strncpy(tmp_str, "win_new failed: ", MAXLEN);
        strncat(tmp_str, Title, MAXLEN);
        display_error_message(tmp_str);
        return (-1);
    }
    win = win_win[win_ptr];
    box = win_box[win_ptr];
    wattron(win, A_REVERSE);
    mvwaddstr(win, lines - 1, 0, " ESC or End to exit ");
    wattroff(win, A_REVERSE);
    wnoutrefresh(box);
    curseskeys(win);
    return (0);
}
void curseskeys(WINDOW *win) {
    char kstr[64];
    int c, p;
    char tmp[256];
    char *tstr;
    MEVENT event;
    mousemask(ALL_MOUSE_EVENTS, NULL);

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
            strcpy(kstr, "0400 A wchar_t contains a key code");
            break;
        case KEY_BREAK:
            strcpy(kstr, "0401 Break key (unreliable)");
            break;
        case KEY_DOWN:
            strcpy(kstr, "0402 down-arrow key");
            break;
        case KEY_UP:
            strcpy(kstr, "0403 up-arrow key");
            break;
        case KEY_LEFT:
            strcpy(kstr, "0404 left-arrow key");
            break;
        case KEY_RIGHT:
            strcpy(kstr, "0405 right-arrow key");
            break;
        case KEY_HOME:
            strcpy(kstr, "0406 home key");
            break;
        case KEY_BACKSPACE:
            strcpy(kstr, "0407 backspace key");
            break;
        case KEY_F0:
            strcpy(kstr, "0410 Function Keys");
            break;
        case KEY_F1:
            strcpy(kstr, "0411 KEY_F1");
            break;
        case KEY_F2:
            strcpy(kstr, "0412 KEY_F2");
            break;
        case KEY_F3:
            strcpy(kstr, "0413 KEY_F3");
            break;
        case KEY_F4:
            strcpy(kstr, "0414 KEY_F4");
            break;
        case KEY_F5:
            strcpy(kstr, "0415 KEY_F5");
            break;
        case KEY_F6:
            strcpy(kstr, "0416 KEY_F6");
            break;
        case KEY_F7:
            strcpy(kstr, "0417 KEY_F7");
            break;
        case KEY_F8:
            strcpy(kstr, "0418 KEY_F8");
            break;
        case KEY_F9:
            strcpy(kstr, "0419 KEY_F9");
            break;
        case KEY_F10:
            strcpy(kstr, "0420 KEY_F10");
            break;
        case KEY_F11:
            strcpy(kstr, "0421 KEY_F11");
            break;
        case KEY_F12:
            strcpy(kstr, "0422 KEY_F12");
            break;
        case KEY_F13:
            strcpy(kstr, "0423 KEY_F13");
            break;
        case KEY_F14:
            strcpy(kstr, "0424 KEY_F14");
            break;
        case KEY_DL:
            strcpy(kstr, "0510 delete-line key");
            break;
        case KEY_IL:
            strcpy(kstr, "0511 insert-line key");
            break;
        case KEY_DC:
            strcpy(kstr, "0512 delete-character key");
            break;
        case KEY_IC:
            strcpy(kstr, "0513 insert-character key");
            break;
        case KEY_EIC:
            strcpy(kstr, "0514 sent by rmir or smir in insert mode");
            break;
        case KEY_CLEAR:
            strcpy(kstr, "0515 clear-screen or erase key");
            break;
        case KEY_EOS:
            strcpy(kstr, "0516 clear-to-end-of-screen key");
            break;
        case KEY_EOL:
            strcpy(kstr, "0517 clear-to-end-of-line key");
            break;
        case KEY_SF:
            strcpy(kstr, "0520 scroll-forward key");
            break;
        case KEY_SR:
            strcpy(kstr, "0520 scroll-backward key");
            break;
        case KEY_NPAGE:
            strcpy(kstr, "0522 next-page key");
            break;
        case KEY_PPAGE:
            strcpy(kstr, "0523 previous-page key");
            break;
        case KEY_STAB:
            strcpy(kstr, "0524 set-tab key");
            break;
        case KEY_CTAB:
            strcpy(kstr, "0525 clear-tab key");
            break;
        case KEY_CATAB:
            strcpy(kstr, "0526 clear-all-tabs key");
            break;
        case KEY_ENTER:
            strcpy(kstr, "0527 enter/send key");
            break;
        case KEY_SRESET:
            strcpy(kstr, "0530 Soft (partial) reset (unreliable)");
            break;
        case KEY_RESET:
            strcpy(kstr, "0531 Reset or hard reset (unreliable)");
            break;
        case KEY_PRINT:
            strcpy(kstr, "0532 print key");
            break;
        case KEY_LL:
            strcpy(kstr, "0533 lower-left key (home down)");
            break;
        case KEY_A1:
            strcpy(kstr, "0534 upper left of keypad");
            break;
        case KEY_A3:
            strcpy(kstr, "0535 upper right of keypad");
            break;
        case KEY_B2:
            strcpy(kstr, "0536 center of keypad");
            break;
        case KEY_C1:
            strcpy(kstr, "0537 lower left of keypad");
            break;
        case KEY_C3:
            strcpy(kstr, "0540 lower right of keypad");
            break;
        case KEY_BTAB:
            strcpy(kstr, "0540 back-tab key");
            break;
        case KEY_BEG:
            strcpy(kstr, "0542 begin key");
            break;
        case KEY_CANCEL:
            strcpy(kstr, "0543 cancel key");
            break;
        case KEY_CLOSE:
            strcpy(kstr, "0544 close key");
            break;
        case KEY_COMMAND:
            strcpy(kstr, "0545 command key");
            break;
        case KEY_COPY:
            strcpy(kstr, "0546 copy key");
            break;
        case KEY_CREATE:
            strcpy(kstr, "0547 create key");
            break;
        case KEY_END:
            strcpy(kstr, "0550 end key");
            break;
        case KEY_EXIT:
            strcpy(kstr, "0551 exit key");
            break;
        case KEY_FIND:
            strcpy(kstr, "0552 find key");
            break;
        case KEY_HELP:
            strcpy(kstr, "0553 help key");
            break;
        case KEY_MARK:
            strcpy(kstr, "0554 mark key");
            break;
        case KEY_MESSAGE:
            strcpy(kstr, "0555 message key");
            break;
        case KEY_MOVE:
            strcpy(kstr, "0556 move key");
            break;
        case KEY_NEXT:
            strcpy(kstr, "0557 next key");
            break;
        case KEY_OPEN:
            strcpy(kstr, "0560 open key");
            break;
        case KEY_OPTIONS:
            strcpy(kstr, "0561 options key");
            break;
        case KEY_PREVIOUS:
            strcpy(kstr, "0562 previous key");
            break;
        case KEY_REDO:
            strcpy(kstr, "0563 redo key");
            break;
        case KEY_REFERENCE:
            strcpy(kstr, "0564 reference key");
            break;
        case KEY_REFRESH:
            strcpy(kstr, "0565 refresh key");
            break;
        case KEY_REPLACE:
            strcpy(kstr, "0566 replace key");
            break;
        case KEY_RESTART:
            strcpy(kstr, "0567 restart key");
            break;
        case KEY_RESUME:
            strcpy(kstr, "0570 resume key");
            break;
        case KEY_SAVE:
            strcpy(kstr, "0571 save key");
            break;
        case KEY_SBEG:
            strcpy(kstr, "0572 shifted begin key");
            break;
        case KEY_SCANCEL:
            strcpy(kstr, "0573 shifted cancel key");
            break;
        case KEY_SCOMMAND:
            strcpy(kstr, "0574 shifted command key");
            break;
        case KEY_SCOPY:
            strcpy(kstr, "0575 shifted copy key");
            break;
        case KEY_SCREATE:
            strcpy(kstr, "0576 shifted create key");
            break;
        case KEY_SDC:
            strcpy(kstr, "0577 shifted delete-character key");
            break;
        case KEY_SDL:
            strcpy(kstr, "0600 shifted delete-line key");
            break;
        case KEY_SELECT:
            strcpy(kstr, "0601 select key");
            break;
        case KEY_SEND:
            strcpy(kstr, "0602 shifted end key");
            break;
        case KEY_SEOL:
            strcpy(kstr, "0603 shifted clear-to-end-of-line key");
            break;
        case KEY_SEXIT:
            strcpy(kstr, "0604 shifted exit key");
            break;
        case KEY_SFIND:
            strcpy(kstr, "0605 shifted find key");
            break;
        case KEY_SHELP:
            strcpy(kstr, "0606 shifted help key");
            break;
        case KEY_SHOME:
            strcpy(kstr, "0607 shifted home key");
            break;
        case KEY_SIC:
            strcpy(kstr, "0610 shifted insert-character key");
            break;
        case KEY_SLEFT:
            strcpy(kstr, "0611 shifted left-arrow key");
            break;
        case KEY_SMESSAGE:
            strcpy(kstr, "0612 shifted message key");
            break;
        case KEY_SMOVE:
            strcpy(kstr, "0613 shifted move key");
            break;
        case KEY_SNEXT:
            strcpy(kstr, "0614 shifted next key");
            break;
        case KEY_SOPTIONS:
            strcpy(kstr, "0615 shifted options key");
            break;
        case KEY_SPREVIOUS:
            strcpy(kstr, "0616 shifted previous key");
            break;
        case KEY_SPRINT:
            strcpy(kstr, "0617 shifted print key");
            break;
        case KEY_SREDO:
            strcpy(kstr, "0620 shifted redo key");
            break;
        case KEY_SREPLACE:
            strcpy(kstr, "0621 shifted replace key");
            break;
        case KEY_SRIGHT:
            strcpy(kstr, "0622 shifted right-arrow key");
            break;
        case KEY_SRSUME:
            strcpy(kstr, "0623 shifted resume key");
            break;
        case KEY_SSAVE:
            strcpy(kstr, "0624 shifted save key");
            break;
        case KEY_SSUSPEND:
            strcpy(kstr, "0625 shifted suspend key");
            break;
        case KEY_SUNDO:
            strcpy(kstr, "0626 shifted undo key");
            break;
        case KEY_SUSPEND:
            strcpy(kstr, "0627 suspend key");
            break;
        case KEY_UNDO:
            strcpy(kstr, "0630 undo key");
            break;
        case KEY_MOUSE:
            if (getmouse(&event) == OK) {
                switch (event.bstate) {
                case BUTTON1_PRESSED:
                    strcpy(tmp, "Button 1 pressed");
                    break;
                case BUTTON1_RELEASED:
                    strcpy(tmp, "Button 1 released");
                    break;
                case BUTTON1_CLICKED:
                    strcpy(tmp, "Button 1 clicked");
                    break;
                case BUTTON1_DOUBLE_CLICKED:
                    strcpy(tmp, "Button 1 double-clicked");
                    break;
                case BUTTON2_PRESSED:
                    strcpy(tmp, "Button 2 pressed");
                    break;
                case BUTTON2_RELEASED:
                    strcpy(tmp, "Button 2 released");
                    break;
                case BUTTON2_CLICKED:
                    strcpy(tmp, "Button 2 clicked");
                    break;
                case BUTTON2_DOUBLE_CLICKED:
                    strcpy(tmp, "Button 2 double-clicked");
                    break;
                case BUTTON3_PRESSED:
                    strcpy(tmp, "Button 3 pressed");
                    break;
                case BUTTON3_RELEASED:
                    strcpy(tmp, "Button 3 released");
                    break;
                case BUTTON3_CLICKED:
                    strcpy(tmp, "Button 3 clicked");
                    break;
                case BUTTON3_DOUBLE_CLICKED:
                    strcpy(tmp, "Button 3 double-clicked");
                    break;
                case BUTTON4_PRESSED:
                    strcpy(tmp, "Scroll Down");
                    break;
                case BUTTON5_PRESSED:
                    strcpy(tmp, "Scroll Up");
                    break;
                default:
                    break;
                }
                mvwaddstr(win, 1, 1, tmp);
                wclrtoeol(win);
                p = strlen(tmp);
                wmove(win, 1, p + 1);
                sprintf(tmp, "      Event  %6d", event.bstate);
                mvwaddstr(win, 3, 4, tmp);
                wclrtoeol(win);
                sprintf(tmp, "       Line  %3d", event.y);
                mvwaddstr(win, 4, 4, tmp);
                wclrtoeol(win);
                sprintf(tmp, "     Column  %3d", event.x);
                mvwaddstr(win, 5, 4, tmp);
                wclrtoeol(win);
                wmove(win, 6, 1);
                wclrtoeol(win);
            }
            break;
        default:
            break;
        }
        if (c != KEY_MOUSE) {

            sprintf(tmp, "      Octal  %3o", c);
            mvwaddstr(win, 3, 4, tmp);
            wclrtoeol(win);

            sprintf(tmp, "    Decimal  %3d", c);
            mvwaddstr(win, 4, 4, tmp);
            wclrtoeol(win);

            sprintf(tmp, "        Hex  %3x", c);
            mvwaddstr(win, 5, 4, tmp);
            wclrtoeol(win);

            if (kstr[0]) {
                tstr = kstr + 5;
                sprintf(tmp, "Description  %s", tstr);
            } else {
                sprintf(tmp, "      ASCII    %c", c);
            }
            mvwaddstr(win, 6, 4, tmp);
            wclrtoeol(win);
            wrefresh(win);
        }
        if (c == '\33' || c == KEY_END) {
            usleep(100000);
            break;
        }
        c = '\0';
    }
    wclear(win);
    wrefresh(win);
    win_del();
}
