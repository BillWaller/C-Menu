/** @file dwin.c
    @brief Window support for C-Menu - EXPERIMENTAL
    @details This file contains functions for managing NCurses windows and color
   settings for the UiChyron structure for function key labels and mouse click
   handling. This file is a work in progress and may be subject to change as the
   C-Menu project evolves. Generally, don't try to use it yet unless you want
   complete the half-done code modifications.
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup window_support Window Support
    @brief Manage NCurses windows and color settings
 */

#include "cm.h"
#include "ui_backend.h"
#include <fcntl.h>
#include <iso646.h>
#include <math.h>
#ifdef UAL_UI
#include "ui_ncurses_internal.h"
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>
#endif
#ifdef NOTCURSES_UI
#include "ui_notcurses_internal.h"
#include <notcurses/notcurses.h>
#endif
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

int click_y;
int click_x;

/** colors_text
    @brief Color names for .minitrc overrides
    @details These names are used in .minitrc to specify color overrides The
   order of these names corresponds to the ColorsEnum values */
char const colors_text[][10] = {
    "black", "red", "green", "yellow", "blue", "magenta", "cyan",
    "white", "orange", "bg", "abg", "bblack", "bred", "bgreen",
    "byellow", "bblue", "bcyan", "bmagenta", "bwhite", "borange", ""};

#ifdef ASDF
typedef enum Cells {
    _DEFAULT,
    _FILL_CHAR,
    _BRKTL,
    _BRKTR,
    _NT,
    _NT_REV,
    _NT_HL,
    _NT_HL_REV,
    _BOX,
    _IND,
    _CMDLN,
    _TITLE,
    _LN,
    _RAN,
    _CHK,
    _LS,
    _RS,
    _TS,
    _BS,
    _TL,
    _TR,
    _HO,
    _VE,
    _BL,
    _BR,
    _LT,
    _RT,
    _TT,
    _BT,
    _CR,
    _SP
} cells_t;
#endif

UiCell cell_default;
UiCell cell_fill_char;
UiCell cell_brktl;
UiCell cell_brktr;
UiCell cell_nt;
UiCell cell_nt_rev;
UiCell cell_nt_hl;
UiCell cell_nt_hl_rev;
UiCell cell_box;
UiCell cell_ind;
UiCell cell_cmdln;
UiCell cell_title;
UiCell cell_ln;
UiCell cell_ran;
UiCell cell_chk;
UiCell cell_ls;
UiCell cell_rs;
UiCell cell_ts;
UiCell cell_bs;
UiCell cell_tl;
UiCell cell_tr;
UiCell cell_ho;
UiCell cell_ve;
UiCell cell_bl;
UiCell cell_br;
UiCell cell_lt;
UiCell cell_rt;
UiCell cell_tt;
UiCell cell_bt;
UiCell cell_cr;
UiCell cell_sp;

ushort cp_box;
ushort cp_ind;
ushort cp_cmdln;
ushort cp_title;
ushort cp_nt;
ushort cp_nt_rev;
ushort cp_nt_hl;
ushort cp_nt_hl_rev;
ushort cp_ln;
ushort cp_default;
ushort cp_fill_char;
ushort cp_brackets;
ushort cp_red;
ushort cp_green;
ushort cp_yellow;
ushort cp_blue;

double GRAY_GAMMA = 1.2; /**< Gamma correction. Set in .minitrc */
double RED_GAMMA = 1.2;
double GREEN_GAMMA = 1.2;
double BLUE_GAMMA = 1.2;

int exit_code;
unsigned int cmd_key;
bool f_sigwench = false;
uint16_t win_attr;
uint16_t box_attr;
uint m_lines;
uint m_cols;
uint m_begy = -1;
uint m_begx = -1;
uint mouse_support;
int stdin_fd;
int stdout_fd;
uint src_line;
char *src_name;
char fn[MAXLEN];
char em0[MAXLEN];
char em1[MAXLEN];
char em2[MAXLEN];
char em3[MAXLEN];

int stdin_fd, stdout_fd, stderr_fd, tty_fd, pipe_in, pipe_out;

/** @brief Initialize local color variables and color pairs based on SIO
 * settings
    @ingroup color_management
    @param sio Pointer to SIO struct with color settings
    @details This function initializes local color variables and color pairs
   based on the settings in the SIO struct. It applies gamma correction to
   colors and sets up cchar_t variables for use in NCurses functions. The color
   pair indices are stored in global variables for easy reference throughout the
   code when applying colors to various parts of the interface using NCurses
   functions that accept color pair indices.
 */
void initialize_cells(SIO *sio) {
    /** gamma correction values */
    /** These are read from ~/.minitrc */
    /** used when initializing colors */
    RED_GAMMA = sio->red_gamma;
    GREEN_GAMMA = sio->green_gamma;
    BLUE_GAMMA = sio->blue_gamma;
    GRAY_GAMMA = sio->gray_gamma;
    init_clr_palette(sio);
    //
    // Standardized color pairs
    //
    cp_default = ui_add_pair(CLR_NT_FG, CLR_NT_BG);
    cp_fill_char = ui_add_pair(CLR_FILL_CHAR_FG, CLR_FILL_CHAR_BG);
    cp_brackets = ui_add_pair(CLR_BRACKETS_FG, CLR_BRACKETS_BG);
    cp_nt = ui_add_pair(CLR_NT_FG, CLR_NT_BG);
    cp_nt_rev = ui_add_pair(CLR_NT_REV_FG, CLR_NT_REV_BG);
    cp_nt_hl_rev = ui_add_pair(CLR_NT_HL_REV_FG, CLR_NT_HL_REV_BG);
    cp_nt_hl = ui_add_pair(CLR_NT_HL_FG, CLR_NT_HL_BG);
    cp_box = ui_add_pair(CLR_BOX_FG, CLR_BOX_BG);
    cp_ind = ui_add_pair(CLR_IND_FG, CLR_IND_BG);
    cp_title = ui_add_pair(CLR_TITLE_FG, CLR_TITLE_BG);
    cp_ln = ui_add_pair(CLR_LN_FG, CLR_LN_BG);
    cp_cmdln = ui_add_pair(CLR_CMDLN_FG, CLR_CMDLN_BG);
    cp_red = ui_add_pair(CLR_FG, CLR_RED);
    cp_green = ui_add_pair(CLR_FG, CLR_GREEN);
    cp_yellow = ui_add_pair(CLR_BG, CLR_YELLOW);
    cp_blue = ui_add_pair(CLR_FG, CLR_BLUE);
    //
    // Standardized UiCells
    //
    cell_default = ui_cell_from_ucp(bw_sp, &sio->nt_fg, &sio->nt_bg);
    cell_fill_char = ui_cell_from_ucp(bw_sp, &sio->fill_char_fg, &sio->fill_char_bg);

    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    wchar_t brktl = sio->brackets[0];
    cell_brktl = ui_cell_from_ucp(&brktl, &sio->brackets_fg, &sio->brackets_bg);
    wchar_t brktr = sio->brackets[1];
    cell_brktr = ui_cell_from_ucp(&brktr, &sio->brackets_fg, &sio->brackets_bg);
    cell_nt = ui_cell_from_ucp(bw_sp, &sio->nt_fg, &sio->nt_bg);
    cell_nt_rev = ui_cell_from_ucp(bw_sp, &sio->nt_rev_fg, &sio->nt_rev_bg);
    cell_nt_hl = ui_cell_from_ucp(bw_sp, &sio->nt_hl_fg, &sio->nt_hl_bg);
    cell_nt_hl_rev = ui_cell_from_ucp(bw_sp, &sio->nt_hl_rev_fg, &sio->nt_hl_rev_bg);
    cell_box = ui_cell_from_ucp(bw_sp, &sio->box_fg, &sio->box_bg);
    cell_cmdln = ui_cell_from_ucp(bw_sp, &sio->cmdln_fg, &sio->cmdln_bg);
    cell_title = ui_cell_from_ucp(bw_sp, &sio->title_fg, &sio->title_bg);
    cell_ln = ui_cell_from_ucp(bw_sp, &sio->ln_fg, &sio->ln_bg);
    cell_ind = ui_cell_from_ucp(bw_sp, &sio->ind_fg, &sio->ind_bg);
    cell_ran = ui_cell_from_ucp(bw_ran, &sio->ran_fg, &sio->ran_bg);
    cell_chk = ui_cell_from_ucp(bw_chk, &sio->box_fg, &sio->box_bg);
    cell_tl = ui_cell_from_ucp(&bw_tl, &sio->box_fg, &sio->box_bg);
    cell_tr = ui_cell_from_ucp(&bw_tr, &sio->box_fg, &sio->box_bg);
    cell_bl = ui_cell_from_ucp(&bw_bl, &sio->box_fg, &sio->box_bg);
    cell_br = ui_cell_from_ucp(&bw_br, &sio->box_fg, &sio->box_bg);
    cell_ho = ui_cell_from_ucp(&bw_ho, &sio->box_fg, &sio->box_bg);
    cell_ve = ui_cell_from_ucp(&bw_ve, &sio->box_fg, &sio->box_bg);
    cell_lt = ui_cell_from_ucp(&bw_lt, &sio->box_fg, &sio->box_bg);
    cell_rt = ui_cell_from_ucp(&bw_rt, &sio->box_fg, &sio->box_bg);
    cell_tt = ui_cell_from_ucp(&bw_tt, &sio->box_fg, &sio->box_bg);
    cell_bt = ui_cell_from_ucp(&bw_bt, &sio->box_fg, &sio->box_bg);
    cell_cr = ui_cell_from_ucp(&bw_cr, &sio->box_fg, &sio->box_bg);
    cell_sp = ui_cell_from_ucp(bw_sp, &sio->box_fg, &sio->box_bg);
}
/** rgb_to_xterm256_idx
    @brief Convert RGB color to XTerm 256 color index
    @ingroup color_management
    @param rgb RGB color
    @return XTerm 256 color index
    @details This function converts an RGB color to the nearest XTerm 256 color
   index. It first checks if the color is a shade of gray, and if so, it uses
   the gray ramp. Otherwise, it calculates the nearest color in the 6x6x6 color
   cube. */
uint rgb_to_xterm256_idx(RGB *rgb) {
    if (rgb->r == rgb->g && rgb->g == rgb->b) {
        if (rgb->r < 8)
            return 16;
        if (rgb->r > 248)
            return 231;
        return ((rgb->r - 8) / 10) + 231;
    } else {
        uint r_index = (rgb->r < 45) ? 0 : (rgb->r - 60) / 40 + 1;
        uint g_index = (rgb->g < 45) ? 0 : (rgb->g - 60) / 40 + 1;
        uint b_index = (rgb->b < 45) ? 0 : (rgb->b - 60) / 40 + 1;
        return 16 + (36 * r_index) + (6 * g_index) + b_index;
    }
}
/** xterm256_idx_to_rgb
    @brief Convert XTerm 256 color index to RGB
    @ingroup color_management
    @param idx XTerm 256 color index
    @return RGB color
    @details This function converts an XTerm 256 color index to an RGB color. It
   first checks if the index is in the standard 16 colors, then checks if it's
   in the 6x6x6 color cube, and finally checks if it's in the gray ramp. */
RGB xterm256_idx_to_rgb(uint idx) {
    /** Convert XTerm 256 color index to RGB
        @param idx - XTerm 256 color index
        @return RGB struct */
    RGB rgb;
    if (idx > 255)
        idx = 255;
    rgb.r = rgb.g = rgb.b = 0;
    if (idx < 16) {
        rgb.r = std_color[idx].r;
        rgb.g = std_color[idx].g;
        rgb.b = std_color[idx].b;
    } else if (idx >= 16 && idx <= 231) {
        idx -= 16;
        rgb.r = (idx / 36) % 6 * 51;
        rgb.g = (idx / 6) % 6 * 51;
        rgb.b = (idx % 6) * 51;
    } else if (idx >= 232 && idx <= 255) {
        uint gray = (idx - 232) * 11;
        rgb.r = rgb.g = rgb.b = gray;
    }
    return rgb;
}

/** apply_gamma
    @brief Apply gamma correction to RGB color
    @ingroup color_management
    @param rgb Pointer to RGB color
    @details This function modifies the RGB color in place. It applies gamma
   correction to the RGB color based on the gamma values set in the SIO struct.
   If the color is a shade of gray, it applies the gray gamma correction.
   Otherwise, it applies the individual red, green, and blue gamma corrections.
 */
void apply_gamma(RGB *rgb) {
    if (rgb->r == rgb->g && rgb->r == rgb->b) {
        if (GRAY_GAMMA > 0.0f && GRAY_GAMMA != 1.0f) {
            rgb->r = (int)(pow((rgb->r / 255.0f), 1.0f / GRAY_GAMMA) * 255.0f);
            rgb->g = rgb->r;
            rgb->b = rgb->r;
        }
        return;
    }
    if (rgb->r != 0 && RED_GAMMA > 0.0f && RED_GAMMA != 1.0f)
        rgb->r = (int)(pow((rgb->r / 255.0f), 1.0f / RED_GAMMA) * 255.0f);
    if (rgb->g != 0 && GREEN_GAMMA > 0.0f && GREEN_GAMMA != 1.0f)
        rgb->g = (int)(pow((rgb->g / 255.0f), 1.0f / GREEN_GAMMA) * 255.0f);
    if (rgb->b != 0 && BLUE_GAMMA > 0.0f && BLUE_GAMMA != 1.0f)
        rgb->b = (int)(pow((rgb->b / 255.0f), 1.0f / BLUE_GAMMA) * 255.0f);
}
/** init_clr_palette
    @brief Initialize color palette based on SIO settings
    @ingroup color_management
    @param sio Pointer to SIO struct with color settings
    @return true if successful, false if error
    @details This function initializes the xterm256 color cube and applies any
   color overrides specified in the SIO struct. The color strings in the SIO
   struct are expected to be six-digit HTML style hex color codes (e.g.,
   "#RRGGBB"). If a color override is specified for any of the standard colors,
   it is applied using the ui_add_color_hex function.
    @note This function should be called after the SIO struct has been populated with
 color settings, typically from a configuration file or user input. It ensures
 that the color palette used by the application matches the user's preferences.
 As I have stated before, you don't really need a 16 or 256 bit color pallette, or an array of rgb color values, or an array of color pairs. You almost certainly have a true color display, so you have 1,666,216 colors to choose from.
 */
bool init_clr_palette(SIO *sio) {
    ui_chg_color(CLR_BLACK, &sio->black);
    ui_chg_color(CLR_RED, &sio->red);
    ui_chg_color(CLR_GREEN, &sio->green);
    ui_chg_color(CLR_YELLOW, &sio->yellow);
    ui_chg_color(CLR_BLUE, &sio->blue);
    ui_chg_color(CLR_MAGENTA, &sio->magenta);
    ui_chg_color(CLR_CYAN, &sio->cyan);
    ui_chg_color(CLR_WHITE, &sio->white);
    ui_chg_color(CLR_BBLACK, &sio->bblack);
    ui_chg_color(CLR_BRED, &sio->bred);
    ui_chg_color(CLR_BGREEN, &sio->bgreen);
    ui_chg_color(CLR_BYELLOW, &sio->byellow);
    ui_chg_color(CLR_BBLUE, &sio->bblue);
    ui_chg_color(CLR_BMAGENTA, &sio->bmagenta);
    ui_chg_color(CLR_BCYAN, &sio->bcyan);
    ui_chg_color(CLR_BWHITE, &sio->bwhite);
    ui_chg_color(CLR_BORANGE, &sio->borange);
    ui_chg_color(CLR_FG, &sio->fg);
    ui_chg_color(CLR_BG, &sio->bg);
    ui_chg_color(CLR_BOX_FG, &sio->box_fg);
    ui_chg_color(CLR_BOX_BG, &sio->box_bg);
    ui_chg_color(CLR_IND_FG, &sio->ind_fg);
    ui_chg_color(CLR_IND_BG, &sio->ind_bg);
    ui_chg_color(CLR_TITLE_BG, &sio->title_bg);
    ui_chg_color(CLR_LN_FG, &sio->ln_fg);
    ui_chg_color(CLR_LN_BG, &sio->ln_bg);
    ui_chg_color(CLR_NT_FG, &sio->nt_fg);
    ui_chg_color(CLR_NT_BG, &sio->nt_bg);
    ui_chg_color(CLR_NT_REV_FG, &sio->nt_rev_fg);
    ui_chg_color(CLR_NT_REV_BG, &sio->nt_rev_bg);
    ui_chg_color(CLR_NT_HL_FG, &sio->nt_hl_fg);
    ui_chg_color(CLR_NT_HL_BG, &sio->nt_hl_bg);
    ui_chg_color(CLR_NT_HL_REV_FG, &sio->nt_hl_rev_fg);
    ui_chg_color(CLR_NT_HL_REV_BG, &sio->nt_hl_rev_bg);
    ui_chg_color(CLR_FILL_CHAR_FG, &sio->fill_char_fg);
    ui_chg_color(CLR_FILL_CHAR_BG, &sio->fill_char_bg);
    ui_chg_color(CLR_BRACKETS_FG, &sio->brackets_fg);
    ui_chg_color(CLR_BRACKETS_BG, &sio->brackets_bg);
    ui_color_cnt = CLR_NCOLORS;
    return true;
}
/** destroy_curses
    @brief Gracefully shut down NCurses and restore terminal settings
    @ingroup window_support
    @details This function should be called before exiting the program to ensure
   that the terminal is left in a usable state. It checks if NCurses was
   initialized and, if so, it erases the screen, and ends the
   NCurses session. It also restores the original terminal settings using
   restore_shell_tioctl and resets signal handlers to their default state with
   sig_dfl_mode. */
void destroy_curses() {
    if (!f_curses_open)
        return;
    ui_shutdown();
    f_curses_open = false;
    restore_shell_tioctl();
    sig_dfl_mode();
    return;
}
/** @defgroup error_handling Error Handling
    @brief Display Error messages
 */

/** answer_yn
    @brief Accept a single letter answer
    @ingroup error_handling
    @param msg0 First error message line
    @param msg1 Second error message line
    @param msg2 Third error message line
    @param msg3 Fourth error message line
    @return Key code of user command */
int answer_yn(char *msg0, char *msg1, char *msg2, char *msg3) {
    char title[MAXLEN];
    uint line, pos, msg_l, msg0_l, msg1_l, msg2_l, msg3_l;

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n%s\n%s\n%s\n\n", msg0, msg1, msg2, msg3);
        return 1;
    }

    UiChyron *chyron = ui_new_chyron();
    ui_set_chyron_key(chyron, 1, "F1 Help", UIKEY_F01);
    ui_set_chyron_key(chyron, 2, "N - No", 'n');
    ui_set_chyron_key(chyron, 3, "Y - Yes", 'y');
    ui_compile_chyron(chyron);

    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    msg0_l = strnz(msg0, maxx - 4);
    msg1_l = strnz(msg1, maxx - 4);
    msg2_l = strnz(msg2, maxx - 4);
    msg3_l = strnz(msg1, maxx - 4);
    msg_l = max(msg0_l, msg1_l);
    msg_l = max(msg_l, msg2_l);
    msg_l = max(msg_l, msg3_l);
    msg_l = max(msg_l, chyron->l);
    msg_l = min(msg_l, maxx - 4);

    pos = ((maxx - msg_l) - 4) / 2;
    line = (maxy - 6) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (ui_surface_box_win_new(5, msg_l + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "ui_surface_box_win_new(%d, %d, %d, %d, %s) failed", 5,
                  msg_l + 2, line, pos, title);
        ui_destroy_chyron(chyron);
        abend(-1, title);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, msg0);
    ui_draw_text(sfc, WIN, 1, 1, msg1);
    ui_draw_text(sfc, WIN, 2, 1, msg2);
    ui_draw_text(sfc, WIN, 3, 1, msg3);
    ui_display_chyron(sfc, WIN, chyron, 4, chyron->l + 1);

    do {
        ui_curs_set(1);
        event.y = event.x = -1;
        cmd_key = ui_get_event(sfc, WIN, chyron, &event, -1);
        if (cmd_key == UIKEY_F01 || cmd_key == 'N' || cmd_key == 'n' || cmd_key == 'Y' || cmd_key == 'y')
            break;
    } while (1);
    ui_cm_surface_destroy(sfc);
    ui_destroy_chyron(chyron);
    return (cmd_key);
}
/** display_error
    @brief Display an error message window or print to stderr
    @ingroup error_handling
    @param msg0 First error message line
    @param msg1 Second error message line
    @param msg2 Third error message line
    @param msg3 Fourth error message line
    @return Key code of user command */
int display_error(char *msg0, char *msg1, char *msg2, char *msg3) {
    char title[MAXLEN];
    uint line, pos, msg_l, msg0_l, msg1_l, msg2_l, msg3_l;

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n", msg0);
        fprintf(stderr, "%s\n", msg1);
        fprintf(stderr, "%s\n", msg2);
        fprintf(stderr, "%s\n\n", msg3);
        return 1;
    }

    UiChyron *chyron = ui_new_chyron();
    ui_set_chyron_key(chyron, 1, "F1 Help", UIKEY_F01);
    ui_set_chyron_key(chyron, 9, "F9 Cancel", UIKEY_F09);
    ui_set_chyron_key(chyron, 10, "F10 Continue", UIKEY_F10);
    ui_compile_chyron(chyron);

    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    msg0_l = strnz(msg0, maxx - 4);
    msg1_l = strnz(msg1, maxx - 4);
    msg2_l = strnz(msg2, maxx - 4);
    msg3_l = strnz(msg1, maxx - 4);
    msg_l = max(msg0_l, msg1_l);
    msg_l = max(msg_l, msg2_l);
    msg_l = max(msg_l, msg3_l);
    msg_l = max(msg_l, chyron->l);
    msg_l = min(msg_l, maxx - 4);

    pos = ((maxx - msg_l) - 4) / 2;
    line = (maxy - 6) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (ui_surface_box_win_new(5, msg_l + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "box_win_new(%d, %d, %d, %d, %s) failed", 5,
                  msg_l + 2, line, pos, title);
        ui_destroy_chyron(chyron);
        abend(-1, title);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, msg0);
    ui_draw_text(sfc, WIN, 1, 1, msg1);
    ui_draw_text(sfc, WIN, 2, 1, msg2);
    ui_draw_text(sfc, WIN, 3, 1, msg3);

    ui_display_chyron(sfc, WIN, chyron, 4, chyron->l + 1);
    do {
        event.y = event.x = -1;
        cmd_key = ui_get_event(sfc, WIN, chyron, &event, -1);
        if (cmd_key == UIKEY_F09 || cmd_key == UIKEY_F10 || cmd_key == 'q' || cmd_key == 'Q')
            break;
    } while (1);
    ui_cm_surface_destroy(sfc);
    ui_destroy_chyron(chyron);
    return (cmd_key);
}

/** Perror
    @brief Display a simple error message window or print to stderr
    @ingroup error_handling
    @param emsg_str Error message string
    @return Key code of user command */
int Perror(char *emsg_str) {
    char emsg[MAXLEN];
    unsigned in_key;
    uint line, pos, cols;
    char title[MAXLEN];
    bool f_xwgetch = true;
    if (emsg_str[0] == '' && emsg_str[1] == 'w') {
        emsg_str += 2;
        f_xwgetch = false;
    }
    strnz__cpy(emsg, emsg_str, 79);
    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", emsg);
        return 1;
    }
    UiChyron *chyron = ui_new_chyron();
    ui_set_chyron_key(chyron, 1, "F1 Help", UIKEY_F01);
    ui_set_chyron_key(chyron, 9, "F9 Cancel", UIKEY_F09);
    ui_set_chyron_key(chyron, 10, "F10 Continue", UIKEY_F10);
    ui_compile_chyron(chyron);
    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    cols = strnz(emsg, maxx - 4);
    cols = max(cols, chyron->l);
    ui_get_screen_size(&maxy, &maxx);
    pos = (maxx - cols - 4) / 2;
    line = (maxy - 4) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (ui_surface_box_win_new(2, cols + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "ui_surface_box_win_new(%d, %d, %d, %d, %s, %b) failed",
                  4, line, line, pos, title);
        ui_destroy_chyron(chyron);
        abend(-1, title);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, emsg_str);
    ui_display_chyron(sfc, WIN, chyron, 1, chyron->l + 1);
    if (f_xwgetch) {
        event.y = event.x = -1;
        in_key = ui_get_event(sfc, WIN, chyron, &event, -1);
    } else {
        in_key = UIKEY_F10;
    }
    ui_destroy_chyron(chyron);
    ui_cm_surface_destroy(sfc);
    return (in_key);
}
/** action_disposition
    @brief Display a simple action disposition message window or print to stderr
    @ingroup error_handling
    @param title Window title
    @param action_str Action description string
    @return true if successful */
bool action_disposition(char *title, char *action_str) {
    uint len;
    uint line, col;

    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", title);
        fprintf(stderr, "%s\n", action_str);
        return true;
    }
    UiChyron *chyron = ui_new_chyron();
    ui_set_chyron_key(chyron, 10, "F10 Continue", UIKEY_F10);
    ui_compile_chyron(chyron);
    len = max(strlen(title), strlen(action_str));
    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    col = (maxx - len - 4) / 2;
    line = (maxy - 4) / 2;
    if (ui_surface_box_win_new(2, len + 2, line, col, title)) {
        ssnprintf(em0, MAXLEN - 1, "ui_surface_box_win_new(%d, %d, %d, %d, %s) failed", 4,
                  line, line, col, title);
        Perror(em0);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, action_str);
    ui_display_chyron(sfc, WIN, chyron, 1, 0);
    event.y = event.x = -1;
    cmd_key = ui_get_event(sfc, WIN, chyron, &event, -1);
    ui_cm_surface_destroy(sfc);
    ui_destroy_chyron(chyron);
    return true;
}
/** abend
    @brief Abnormal program termination
    @ingroup error_handling
    @param ec Exit code
    @param s Error message */
void abend(int ec, char *s) {
    destroy_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    fprintf(stderr, "\n\nABEND: %s (code: %d)\n", s, ec);
    exit(EXIT_FAILURE);
}
