/** @file curskeys.c
    @brief Test Curses Keys
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"
#include "ui_backend.h"
#include <unistd.h>

#define KSTRLEN 100
#define MAXLEN 256
int popup_ckeys();

/** @brief Display Curses Keys
    Responds to curses keys and mouse events, displaying the key code and
    description.
    @return 0 on success, non-zero on failure
 */
#ifdef UAL_UI
#include "ui_ncurses_internal.h"
int ncurses_input(UiSurface *s, uint w);
#else
#include "ui_notcurses_internal.h"
int notcurses_input(UiSurface *s, uint w);
#endif

int popup_ckeys() {
    char Title[] = "TEST CURSES KEYS";
    unsigned lines = 10;
    unsigned cols = 55;
    char tmp[MAXLEN];
    uint maxy, maxx;
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
    uint begy = (maxy - lines) / 3;
    uint begx = (maxx - cols) / 2;
    if (box_win_new(lines, cols, begy, begx, Title)) {
        strnz__cpy(tmp, "box_win_new failed: ", MAXLEN - 1);
        strnz__cat(tmp, Title, MAXLEN - 1);
        Perror(tmp);
        exit(EXIT_FAILURE);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    ui_keypad(sfc, WIN, true);
    ui_bkgdset(sfc, WIN, &cell_nt_rev);
    ui_mvwaddstr(sfc, WIN, lines - 1, 0, " <ALT>END to exit ");
    ui_bkgdset(sfc, WIN, &cell_nt);
    ui_mvwaddstr(sfc, WIN, 1, 4, "Press a key or activate the mouse:");
#ifdef UAL_UI
    ncurses_input(sfc, WIN);
#else
    notcurses_input(sfc, WIN);
#endif
    ui_surface_destroy(sfc);
    return 0;
}

#ifdef UAL_UI
int ncurses_input(UiSurface *sfc, uint w) {
    char kstr[KSTRLEN];
    int c;
    char action[MAXLEN];
    c = '\0';
    char tmp[MAXLEN];
    UiEvent ev;
    while (!c) {
        kstr[0] = '\0';
        ui_cursor_move(sfc, WIN, 1, 39);
        do {
            ui_render();
            c = ui_get_event(sfc, WIN, &ev, -1);
            if (sig_received != 0) {
                if (handle_signal(sig_received))
                    c = display_error(em0, em1, em2, NULL);
                if (c == 'q' || c == KEY_F(9))
                    exit(EXIT_FAILURE);
                continue;
            }
        } while (c == ERR);
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
        case KEY_MOUSE:
            switch (ev.bstate) {
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
            ui_mvwaddstr(sfc, WIN, 6, 3, "     Action:");
            ui_mvwaddstr(sfc, WIN, 6, 16, action);
            ui_wclrtoeol(sfc, WIN);
            strnz__cpy(tmp, "  With Key:", MAXLEN - 1);
            if (ev.bstate & BUTTON_SHIFT)
                strnz__cat(tmp, " Shift", MAXLEN - 1);
            if (ev.bstate & BUTTON_CTRL)
                strnz__cat(tmp, " Ctrl", MAXLEN - 1);
            if (ev.bstate & BUTTON_ALT)
                strnz__cat(tmp, " Alt", MAXLEN - 1);
            ui_mvwaddstr(sfc, WIN, 3, 4, tmp);
            ui_wclrtoeol(sfc, WIN);
            if (ev.mouse_inside) {
                sprintf(tmp, "   Inside Win:  y: %3d, x: %3d",
                        ev.y, ev.x);
            } else {
                sprintf(tmp, "       stdwin:  y: %3d, x: %3d",
                        ev.y, ev.x);
            }
            ui_mvwaddstr(sfc, WIN, 4, 4, tmp);
            ui_wclrtoeol(sfc, WIN);
            ui_cursor_move(sfc, WIN, 5, 0);
            ui_wclrtoeol(sfc, WIN);
            ui_cursor_move(sfc, WIN, 7, 0);
            ui_wclrtoeol(sfc, WIN);
        default:
            break;
        }
        if (c != KEY_MOUSE) {
            sprintf(tmp, "     Octal: %3o", c);
            ui_mvwaddstr(sfc, WIN, 3, 4, tmp);
            ui_wclrtoeol(sfc, WIN);

            sprintf(tmp, "   Decimal: %3d", c);
            ui_mvwaddstr(sfc, WIN, 4, 4, tmp);
            ui_wclrtoeol(sfc, WIN);

            sprintf(tmp, "       Hex: %3x", c);
            ui_mvwaddstr(sfc, WIN, 5, 4, tmp);
            ui_wclrtoeol(sfc, WIN);

            if (kstr[0]) {
                sprintf(tmp, "Description: %s", kstr);
            } else {
                sprintf(tmp, "      ASCII: %c", c);
            }
            ui_mvwaddstr(sfc, WIN, 6, 3, tmp);
            ui_wclrtoeol(sfc, WIN);
            ui_render();
        }
        if (c == KEY_ALTEND) {
            usleep(100000);
            break;
        }
        c = '\0';
    }
    return 0;
#else
int notcurses_input(UiSurface *sfc, uint w) {
    char kstr[KSTRLEN];
    int c = -1;
    char action[MAXLEN];
    uint key_id = 0;
    UiEvent ev;
    while (!c) {
        kstr[0] = '\0';
        ui_cursor_move(sfc, WIN, 1, 39);
        do {
            ui_render();
            c = ui_get_event(sfc, WIN, &ev, -1);
            if (sig_received != 0) {
                if (handle_signal(sig_received))
                    c = display_error(em0, em1, em2, NULL);
                if (c == 'q' || c == KEY_F09)
                    exit(EXIT_FAILURE);
                continue;
            }
        } while (c == -1);
        switch (c) {
        case NCKEY_INVALID:
            strcpy(kstr, "NCKEY_INVALID");
            break;
        case NCKEY_RESIZE:
            strcpy(kstr, "NCKEY_RESIZE");
            break;
        case NCKEY_UP:
            strcpy(kstr, "NCKEY_UP");
            break;
        case NCKEY_RIGHT:
            strcpy(kstr, "NCKEY_RIGHT");
            break;
        case NCKEY_DOWN:
            strcpy(kstr, "NCKEY_DOWN");
            break;
        case NCKEY_LEFT:
            strcpy(kstr, "NCKEY_LEFT");
            break;
        case NCKEY_INS:
            strcpy(kstr, "NCKEY_INS");
            break;
        case NCKEY_DEL:
            strcpy(kstr, "NCKEY_DEL");
            break;
        case NCKEY_BACKSPACE:
            strcpy(kstr, "NCKEY_BACKSPACE");
            break;
        case NCKEY_PGDOWN:
            strcpy(kstr, "NCKEY_PGDOWN");
            break;
        case NCKEY_PGUP:
            strcpy(kstr, "NCKEY_PGUP");
            break;
        case NCKEY_HOME:
            strcpy(kstr, "NCKEY_HOME");
            break;
        case NCKEY_END:
            strcpy(kstr, "NCKEY_END");
            break;
        case NCKEY_F00:
            strcpy(kstr, "NCKEY_F00");
            break;
        case NCKEY_F01:
            strcpy(kstr, "NCKEY_F01");
            break;
        case NCKEY_F02:
            strcpy(kstr, "NCKEY_F02");
            break;
        case NCKEY_F03:
            strcpy(kstr, "NCKEY_F03");
            break;
        case NCKEY_F04:
            strcpy(kstr, "NCKEY_F04");
            break;
        case NCKEY_F05:
            strcpy(kstr, "NCKEY_F05");
            break;
        case NCKEY_F06:
            strcpy(kstr, "NCKEY_F06");
            break;
        case NCKEY_F07:
            strcpy(kstr, "NCKEY_F07");
            break;
        case NCKEY_F08:
            strcpy(kstr, "NCKEY_F08");
            break;
        case NCKEY_F09:
            strcpy(kstr, "NCKEY_F09");
            break;
        case NCKEY_F10:
            strcpy(kstr, "NCKEY_F10");
            break;
        case NCKEY_F11:
            strcpy(kstr, "NCKEY_F11");
            break;
        case NCKEY_F12:
            strcpy(kstr, "NCKEY_F12");
            break;
        case NCKEY_F13:
            strcpy(kstr, "NCKEY_F13");
            break;
        case NCKEY_F14:
            strcpy(kstr, "NCKEY_F14");
            break;
        case NCKEY_F15:
            strcpy(kstr, "NCKEY_F15");
            break;
        case NCKEY_F16:
            strcpy(kstr, "NCKEY_F16");
            break;
        case NCKEY_F17:
            strcpy(kstr, "NCKEY_F17");
            break;
        case NCKEY_F18:
            strcpy(kstr, "NCKEY_F18");
            break;
        case NCKEY_F19:
            strcpy(kstr, "NCKEY_F19");
            break;
        case NCKEY_F20:
            strcpy(kstr, "NCKEY_F20");
            break;
        case NCKEY_F21:
            strcpy(kstr, "NCKEY_F21");
            break;
        case NCKEY_F22:
            strcpy(kstr, "NCKEY_F22");
            break;
        case NCKEY_F23:
            strcpy(kstr, "NCKEY_F23");
            break;
        case NCKEY_F24:
            strcpy(kstr, "NCKEY_F24");
            break;
        case NCKEY_F25:
            strcpy(kstr, "NCKEY_F25");
            break;
        case NCKEY_F26:
            strcpy(kstr, "NCKEY_F26");
            break;
        case NCKEY_F27:
            strcpy(kstr, "NCKEY_F27");
            break;
        case NCKEY_F28:
            strcpy(kstr, "NCKEY_F28");
            break;
        case NCKEY_F29:
            strcpy(kstr, "NCKEY_F29");
            break;
        case NCKEY_F30:
            strcpy(kstr, "NCKEY_F30");
            break;
        case NCKEY_F31:
            strcpy(kstr, "NCKEY_F31");
            break;
        case NCKEY_F32:
            strcpy(kstr, "NCKEY_F32");
            break;
        case NCKEY_F33:
            strcpy(kstr, "NCKEY_F33");
            break;
        case NCKEY_F34:
            strcpy(kstr, "NCKEY_F34");
            break;
        case NCKEY_F35:
            strcpy(kstr, "NCKEY_F35");
            break;
        case NCKEY_F36:
            strcpy(kstr, "NCKEY_F36");
            break;
        case NCKEY_F37:
            strcpy(kstr, "NCKEY_F37");
            break;
        case NCKEY_F38:
            strcpy(kstr, "NCKEY_F38");
            break;
        case NCKEY_F39:
            strcpy(kstr, "NCKEY_F39");
            break;
        case NCKEY_F40:
            strcpy(kstr, "NCKEY_F40");
            break;
        case NCKEY_F41:
            strcpy(kstr, "NCKEY_F41");
            break;
        case NCKEY_F42:
            strcpy(kstr, "NCKEY_F42");
            break;
        case NCKEY_F43:
            strcpy(kstr, "NCKEY_F43");
            break;
        case NCKEY_F44:
            strcpy(kstr, "NCKEY_F44");
            break;
        case NCKEY_F45:
            strcpy(kstr, "NCKEY_F45");
            break;
        case NCKEY_F46:
            strcpy(kstr, "NCKEY_F46");
            break;
        case NCKEY_F47:
            strcpy(kstr, "NCKEY_F47");
            break;
        case NCKEY_F48:
            strcpy(kstr, "NCKEY_F48");
            break;
        case NCKEY_F49:
            strcpy(kstr, "NCKEY_F49");
            break;
        case NCKEY_F50:
            strcpy(kstr, "NCKEY_F50");
            break;
        case NCKEY_F51:
            strcpy(kstr, "NCKEY_F51");
            break;
        case NCKEY_F52:
            strcpy(kstr, "NCKEY_F52");
            break;
        case NCKEY_F53:
            strcpy(kstr, "NCKEY_F53");
            break;
        case NCKEY_F54:
            strcpy(kstr, "NCKEY_F54");
            break;
        case NCKEY_F55:
            strcpy(kstr, "NCKEY_F55");
            break;
        case NCKEY_F56:
            strcpy(kstr, "NCKEY_F56");
            break;
        case NCKEY_F57:
            strcpy(kstr, "NCKEY_F57");
            break;
        case NCKEY_F58:
            strcpy(kstr, "NCKEY_F58");
            break;
        case NCKEY_F59:
            strcpy(kstr, "NCKEY_F59");
            break;
        case NCKEY_F60:
            strcpy(kstr, "NCKEY_F60");
            break;
        case NCKEY_ENTER:
            strcpy(kstr, "NCKEY_ENTER");
            break;
        case NCKEY_CLS:
            strcpy(kstr, "NCKEY_CLS");
            break;
        case NCKEY_DLEFT:
            strcpy(kstr, "NCKEY_DLEFT");
            break;
        case NCKEY_DRIGHT:
            strcpy(kstr, "NCKEY_DRIGHT");
            break;
        case NCKEY_ULEFT:
            strcpy(kstr, "NCKEY_ULEFT");
            break;
        case NCKEY_URIGHT:
            strcpy(kstr, "NCKEY_URIGHT");
            break;
        case NCKEY_CENTER:
            strcpy(kstr, "NCKEY_CENTER");
            break;
        case NCKEY_BEGIN:
            strcpy(kstr, "NCKEY_BEGIN");
            break;
        case NCKEY_CANCEL:
            strcpy(kstr, "NCKEY_CANCEL");
            break;
        case NCKEY_CLOSE:
            strcpy(kstr, "NCKEY_CLOSE");
            break;
        case NCKEY_COMMAND:
            strcpy(kstr, "NCKEY_COMMAND");
            break;
        case NCKEY_COPY:
            strcpy(kstr, "NCKEY_COPY");
            break;
        case NCKEY_EXIT:
            strcpy(kstr, "NCKEY_EXIT");
            break;
        case NCKEY_PRINT:
            strcpy(kstr, "NCKEY_PRINT");
            break;
        case NCKEY_REFRESH:
            strcpy(kstr, "NCKEY_REFRESH");
            break;
        case NCKEY_SEPARATOR:
            strcpy(kstr, "NCKEY_SEPARATOR");
            break;
        case NCKEY_CAPS_LOCK:
            strcpy(kstr, "NCKEY_CAPS_LOCK");
            break;
        case NCKEY_SCROLL_LOCK:
            strcpy(kstr, "NCKEY_SCROLL_LOCK");
            break;
        case NCKEY_NUM_LOCK:
            strcpy(kstr, "NCKEY_NUM_LOCK");
            break;
        case NCKEY_PRINT_SCREEN:
            strcpy(kstr, "NCKEY_PRINT_SCREEN");
            break;
        case NCKEY_PAUSE:
            strcpy(kstr, "NCKEY_PAUSE");
            break;
        case NCKEY_MENU:
            strcpy(kstr, "NCKEY_MENU");
            break;
        case NCKEY_MEDIA_PLAY:
            strcpy(kstr, "NCKEY_MEDIA_PLAY");
            break;
        case NCKEY_MEDIA_PAUSE:
            strcpy(kstr, "NCKEY_MEDIA_PAUSE");
            break;
        case NCKEY_MEDIA_PPAUSE:
            strcpy(kstr, "NCKEY_MEDIA_PPAUSE");
            break;
        case NCKEY_MEDIA_REV:
            strcpy(kstr, "NCKEY_MEDIA_REV");
            break;
        case NCKEY_MEDIA_STOP:
            strcpy(kstr, "NCKEY_MEDIA_STOP");
            break;
        case NCKEY_MEDIA_FF:
            strcpy(kstr, "NCKEY_MEDIA_FF");
            break;
        case NCKEY_MEDIA_REWIND:
            strcpy(kstr, "NCKEY_MEDIA_REWIND");
            break;
        case NCKEY_MEDIA_NEXT:
            strcpy(kstr, "NCKEY_MEDIA_NEXT");
            break;
        case NCKEY_MEDIA_PREV:
            strcpy(kstr, "NCKEY_MEDIA_PREV");
            break;
        case NCKEY_MEDIA_RECORD:
            strcpy(kstr, "NCKEY_MEDIA_RECORD");
            break;
        case NCKEY_MEDIA_LVOL:
            strcpy(kstr, "NCKEY_MEDIA_LVOL");
            break;
        case NCKEY_MEDIA_RVOL:
            strcpy(kstr, "NCKEY_MEDIA_RVOL");
            break;
        case NCKEY_MEDIA_MUTE:
            strcpy(kstr, "NCKEY_MEDIA_MUTE");
            break;
        case NCKEY_LSHIFT:
            strcpy(kstr, "NCKEY_LSHIFT");
            break;
        case NCKEY_LCTRL:
            strcpy(kstr, "NCKEY_LCTRL");
            break;
        case NCKEY_LALT:
            strcpy(kstr, "NCKEY_LALT");
            break;
        case NCKEY_LSUPER:
            strcpy(kstr, "NCKEY_LSUPER");
            break;
        case NCKEY_LHYPER:
            strcpy(kstr, "NCKEY_LHYPER");
            break;
        case NCKEY_LMETA:
            strcpy(kstr, "NCKEY_LMETA");
            break;
        case NCKEY_RSHIFT:
            strcpy(kstr, "NCKEY_RSHIFT");
            break;
        case NCKEY_RCTRL:
            strcpy(kstr, "NCKEY_RCTRL");
            break;
        case NCKEY_RALT:
            strcpy(kstr, "NCKEY_RALT");
            break;
        case NCKEY_RSUPER:
            strcpy(kstr, "NCKEY_RSUPER");
            break;
        case NCKEY_RHYPER:
            strcpy(kstr, "NCKEY_RHYPER");
            break;
        case NCKEY_RMETA:
            strcpy(kstr, "NCKEY_RMETA");
            break;
        case NCKEY_L3SHIFT:
            strcpy(kstr, "NCKEY_L3SHIFT");
            break;
        case NCKEY_L5SHIFT:
            strcpy(kstr, "NCKEY_L5SHIFT");
            break;
        case NCKEY_MOTION:
            strcpy(kstr, "NCKEY_MOTION");
            break;
        case NCKEY_BUTTON1:
            strcpy(kstr, "NCKEY_BUTTON1");
            break;
        case NCKEY_BUTTON2:
            strcpy(kstr, "NCKEY_BUTTON2");
            break;
        case NCKEY_BUTTON3:
            strcpy(kstr, "NCKEY_BUTTON3");
            break;
        case NCKEY_BUTTON4:
            strcpy(kstr, "NCKEY_BUTTON4");
            break;
        case NCKEY_BUTTON5:
            strcpy(kstr, "NCKEY_BUTTON5");
            break;
        case NCKEY_BUTTON6:
            strcpy(kstr, "NCKEY_BUTTON6");
            break;
        case NCKEY_BUTTON7:
            strcpy(kstr, "NCKEY_BUTTON7");
            break;
        case NCKEY_BUTTON8:
            strcpy(kstr, "NCKEY_BUTTON8");
            break;
        case NCKEY_BUTTON9:
            strcpy(kstr, "NCKEY_BUTTON9");
            break;
        case NCKEY_BUTTON10:
            strcpy(kstr, "NCKEY_BUTTON10");
            break;
        case NCKEY_BUTTON11:
            strcpy(kstr, "NCKEY_BUTTON11");
            break;
        case NCKEY_SIGNAL:
            strcpy(kstr, "NCKEY_SIGNAL");
            break;
        case NCKEY_EOF:
            strcpy(kstr, "NCKEY_EOF");
            break;
        default:
            snprintf(kstr, 32, "NCKEY_%d", key_id);
            break;
        }
        ui_mvwaddstr(sfc, w, 6, 3, "     Action:");
        ui_mvwaddstr(sfc, w, 6, 16, action);
        ui_wclrtoeol(sfc, w);
        char tmp[MAXLEN];
        if (ev.mouse_inside) {
            sprintf(tmp, "   Inside Win:  y: %3d, x: %3d",
                    ev.y, ev.x);
        } else {
            sprintf(tmp, "       stdwin:  y: %3d, x: %3d",
                    ev.y, ev.x);
        }
        ui_mvwaddstr(sfc, w, 4, 4, tmp);
        ui_wclrtoeol(sfc, w);
        ui_cursor_move(sfc, w, 5, 0);
        ui_wclrtoeol(sfc, w);
        ui_cursor_move(sfc, w, 7, 0);
        ui_wclrtoeol(sfc, w);
        if (c != KEY_MOUSE) {
            sprintf(tmp, "     Octal: %3o", c);
            ui_mvwaddstr(sfc, w, 3, 4, tmp);
            ui_wclrtoeol(sfc, w);

            sprintf(tmp, "   Decimal: %3d", c);
            ui_mvwaddstr(sfc, w, 4, 4, tmp);
            ui_wclrtoeol(sfc, w);

            sprintf(tmp, "       Hex: %3x", c);
            ui_mvwaddstr(sfc, w, 5, 4, tmp);
            ui_wclrtoeol(sfc, w);

            if (kstr[0]) {
                sprintf(tmp, "Description: %s", kstr);
            } else {
                sprintf(tmp, "      ASCII: %c", c);
            }
            ui_mvwaddstr(sfc, w, 6, 3, tmp);
            ui_wclrtoeol(sfc, w);
            ui_render();
        }
        if (c == KEY_ALTEND) {
            usleep(100000);
            break;
        }
        c = '\0';
    }
#endif
    return 0;
}
