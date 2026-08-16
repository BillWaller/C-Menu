/** @file dwin.c
    @brief Window support for C-Menu - EXPERIMENTAL
    @details This file contains functions for managing NCurses windows and color
   settings for the Chyron structure for function key labels and mouse click
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
#include <errno.h>
#include <fcntl.h>
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

UiRuntime *ui_runtime;
UiConfig *ui_config;
UiSurface *ui_surface[MAXWIN];

int click_y;
int click_x;

bool init_clr_palette(SIO *);
bool open_curses(SIO *);
void destroy_curses();
void abend(int, char *);
int nf_error(int, char *);
int Perror(char *);
RGB xterm256_idx_to_rgb(int);
int rgb_to_xterm256_idx(RGB *);
void apply_gamma(RGB *);
Chyron *new_chyron();
void set_chyron_key(Chyron *, int, char *, int);
void set_chyron_key_cp(Chyron *, int, char *, int, int);
bool is_set_chyron_key(Chyron *, int);
void unset_chyron_key(Chyron *, int);
void compile_chyron(Chyron *);
Chyron *destroy_chyron(Chyron *chyron);
void activate_chyron_key(Chyron *chyron, int k);
void activate_all_chyron_keys(Chyron *chyron);
void deactivate_chyron_key(Chyron *chyron, int k);
void deactivate_all_chyron_keys(Chyron *chyron);
int border_draw(UiSurface *sfc);
int border_title(UiSurface *sfc, char *title);
int border_ysplit(UiSurface *, int);
int border_ysplit_text(UiSurface *, char *, int);
void win_resize(int, int, char *);
void ui_restore_wins();

/** colors_text
    @brief Color names for .minitrc overrides
    @details These names are used in .minitrc to specify color overrides The
   order of these names corresponds to the ColorsEnum values */
char const colors_text[][10] = {
    "black", "red", "green", "yellow", "blue", "magenta", "cyan",
    "white", "orange", "bg", "abg", "bblack", "bred", "bgreen",
    "byellow", "bblue", "bcyan", "bmagenta", "bwhite", "borange", ""};

const wchar_t bw_ho = BW_HO;   /**< horizontal line */
const wchar_t bw_ve = BW_VE;   /**< vertical line */
const wchar_t bw_tl = BW_RTL;  /**< top left corner */
const wchar_t bw_tr = BW_RTR;  /**< top right corner */
const wchar_t bw_bl = BW_RBL;  /**< bottom left corner */
const wchar_t bw_br = BW_RBR;  /**< bottom right corner */
const wchar_t bw_lt = BW_LT;   /**< left tee */
const wchar_t bw_rt = BW_RT;   /**< right tee */
const wchar_t bw_sp = BW_SP;   /**< tee space */
const wchar_t bw_ra = BW_RA;   /**< right arrow */
const wchar_t bw_la = BW_LA;   /**< left arrow */
const wchar_t bw_ua = BW_UA;   /**< up arrow */
const wchar_t bw_da = BW_DA;   /**< down arrow */
const wchar_t bw_ran = BW_RAN; /**< right angle */
const wchar_t bw_chk = BW_CHK; /**< check mark */

UiStyle style_default;
UiStyle style_fill_char;
UiStyle style_brktl;
UiStyle style_brktr;
UiStyle style_nt;
UiStyle style_nt_rev;
UiStyle style_nt_hl;
UiStyle style_nt_hl_rev;
UiStyle style_box;
UiStyle style_ind;
UiStyle style_cmdln;
UiStyle style_title;
UiStyle style_ln;
UiStyle style_ran;
UiStyle style_chk;
UiStyle style_ls;
UiStyle style_rs;
UiStyle style_ts;
UiStyle style_bs;

double GRAY_GAMMA = 1.2; /**< Gamma correction. Set in .minitrc */
double RED_GAMMA = 1.2;
double GREEN_GAMMA = 1.2;
double BLUE_GAMMA = 1.2;

int exit_code;
unsigned int cmd_key;
bool f_sigwench = false;
int win_attr;
int box_attr;
int sfc_ptr;
int m_lines;
int m_cols;
int m_begy = -1;
int m_begx = -1;
int mouse_support;
int stdin_fd;
int stdout_fd;
int src_line;
char *src_name;
char fn[MAXLEN];
char em0[MAXLEN];
char em1[MAXLEN];
char em2[MAXLEN];
char em3[MAXLEN];
short cp_box;
short cp_ind;
short cp_cmdln;
short cp_title;
short cp_nt;
short cp_nt_rev;
short cp_nt_hl;
short cp_nt_hl_rev;
short cp_ln;
short cp_fill_char;
short cp_brackets;
short cp_red;
short cp_green;
short cp_yellow;
short cp_blue;

int tty_fd, pipe_in, pipe_out;

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
void initialize_styles(SIO *sio) {
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
    // Standardized UiStyle variables
    //
    void mbc_to_wc(wchar_t wc[2], const char mbc);
    style_default = ui_style_from_hex("#d0d0d0", "#000000", WA_NORMAL, nullptr);
    style_fill_char = ui_style_from_hex(sio->fill_char_fg, sio->fill_char_bg, WA_NORMAL, nullptr);
    wchar_t brktl[2];
    mbc_to_wc(brktl, sio->brackets[0]);
    style_brktl = ui_style_from_hex(sio->brackets_fg, sio->brackets_bg, WA_NORMAL, brktl);
    wchar_t brktr[2];
    mbc_to_wc(brktr, sio->brackets[0]);
    style_brktr = ui_style_from_hex(sio->brackets_fg, sio->brackets_bg, WA_NORMAL, brktr);
    style_nt = ui_style_from_hex(sio->nt_fg, sio->nt_bg, WA_NORMAL, nullptr);
    style_nt_rev = ui_style_from_hex(sio->nt_rev_fg, sio->nt_rev_bg, WA_NORMAL, nullptr);
    style_nt_hl = ui_style_from_hex(sio->nt_hl_fg, sio->nt_hl_bg, WA_NORMAL, nullptr);
    style_nt_hl_rev = ui_style_from_hex(sio->nt_hl_rev_fg, sio->nt_hl_rev_bg, WA_NORMAL, nullptr);
    style_box = ui_style_from_hex(sio->box_fg, sio->box_bg, WA_NORMAL, nullptr);
    style_ind = ui_style_from_hex(sio->ind_fg, sio->ind_bg, WA_NORMAL, nullptr);
    style_cmdln = ui_style_from_hex(sio->cmdln_fg, sio->cmdln_bg, WA_NORMAL, nullptr);
    style_title = ui_style_from_hex(sio->title_fg, sio->title_bg, WA_NORMAL, nullptr);
    style_ln = ui_style_from_hex(sio->ln_fg, sio->ln_bg, WA_NORMAL, nullptr);
    style_ran = ui_style_from_hex(sio->ran_fg, sio->ran_bg, WA_NORMAL, &bw_ran);
    style_chk = ui_style_from_hex(sio->ind_fg, sio->ind_bg, WA_NORMAL, &bw_chk);
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
int rgb_to_xterm256_idx(RGB *rgb) {
    if (rgb->r == rgb->g && rgb->g == rgb->b) {
        if (rgb->r < 8)
            return 16;
        if (rgb->r > 248)
            return 231;
        return ((rgb->r - 8) / 10) + 231;
    } else {
        int r_index = (rgb->r < 45) ? 0 : (rgb->r - 60) / 40 + 1;
        int g_index = (rgb->g < 45) ? 0 : (rgb->g - 60) / 40 + 1;
        int b_index = (rgb->b < 45) ? 0 : (rgb->b - 60) / 40 + 1;
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
RGB xterm256_idx_to_rgb(int idx) {
    /** Convert XTerm 256 color index to RGB
        @param idx - XTerm 256 color index
        @return RGB struct */
    RGB rgb;
    if (idx > 255)
        idx = 255;
    if (idx < 0)
        idx = 0;
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
        int gray = (idx - 232) * 11;
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
   it is applied using the ui_add_color_hex function. */
bool init_clr_palette(SIO *sio) {
    if (sio->black[0])
        ui_chg_color_hex(CLR_BLACK, sio->black);
    if (sio->red[0])
        ui_chg_color_hex(CLR_RED, sio->red);
    if (sio->green[0])
        ui_chg_color_hex(CLR_GREEN, sio->green);
    if (sio->yellow[0])
        ui_chg_color_hex(CLR_YELLOW, sio->yellow);
    if (sio->blue[0])
        ui_chg_color_hex(CLR_BLUE, sio->blue);
    if (sio->magenta[0])
        ui_chg_color_hex(CLR_MAGENTA, sio->magenta);
    if (sio->cyan[0])
        ui_chg_color_hex(CLR_CYAN, sio->cyan);
    if (sio->white[0])
        ui_chg_color_hex(CLR_WHITE, sio->white);
    if (sio->bblack[0])
        ui_chg_color_hex(CLR_BBLACK, sio->bblack);
    if (sio->bred[0])
        ui_chg_color_hex(CLR_BRED, sio->bred);
    if (sio->bgreen[0])
        ui_chg_color_hex(CLR_BGREEN, sio->bgreen);
    if (sio->byellow[0])
        ui_chg_color_hex(CLR_BYELLOW, sio->byellow);
    if (sio->bblue[0])
        ui_chg_color_hex(CLR_BBLUE, sio->bblue);
    if (sio->bmagenta[0])
        ui_chg_color_hex(CLR_BMAGENTA, sio->bmagenta);
    if (sio->bcyan[0])
        ui_chg_color_hex(CLR_BCYAN, sio->bcyan);
    if (sio->bwhite[0])
        ui_chg_color_hex(CLR_BWHITE, sio->bwhite);
    if (sio->borange[0])
        ui_chg_color_hex(CLR_BORANGE, sio->borange);
    if (sio->fg[0])
        ui_chg_color_hex(CLR_FG, sio->fg);
    if (sio->bg[0])
        ui_chg_color_hex(CLR_BG, sio->bg);
    if (sio->box_fg[0])
        ui_chg_color_hex(CLR_BOX_FG, sio->box_fg);
    if (sio->box_bg[0])
        ui_chg_color_hex(CLR_BOX_BG, sio->box_bg);
    if (sio->ind_fg[0])
        ui_chg_color_hex(CLR_IND_FG, sio->ind_fg);
    if (sio->ind_bg[0])
        ui_chg_color_hex(CLR_IND_BG, sio->ind_bg);
    if (sio->title_fg[0])
        ui_chg_color_hex(CLR_TITLE_FG, sio->title_fg);
    if (sio->title_bg[0])
        ui_chg_color_hex(CLR_TITLE_BG, sio->title_bg);
    if (sio->ln_fg[0])
        ui_chg_color_hex(CLR_LN_FG, sio->ln_fg);
    if (sio->ln_bg[0])
        ui_chg_color_hex(CLR_LN_BG, sio->ln_bg);
    if (sio->nt_fg[0])
        ui_chg_color_hex(CLR_NT_FG, sio->nt_fg);
    if (sio->nt_bg[0])
        ui_chg_color_hex(CLR_NT_BG, sio->nt_bg);
    if (sio->nt_rev_fg[0])
        ui_chg_color_hex(CLR_NT_REV_FG, sio->nt_rev_fg);
    if (sio->nt_rev_bg[0])
        ui_chg_color_hex(CLR_NT_REV_BG, sio->nt_rev_bg);
    if (sio->nt_hl_fg[0])
        ui_chg_color_hex(CLR_NT_HL_FG, sio->nt_hl_fg);
    if (sio->nt_hl_bg[0])
        ui_chg_color_hex(CLR_NT_HL_BG, sio->nt_hl_bg);
    if (sio->nt_hl_rev_fg[0])
        ui_chg_color_hex(CLR_NT_HL_REV_FG, sio->nt_hl_rev_fg);
    if (sio->nt_hl_rev_bg[0])
        ui_chg_color_hex(CLR_NT_HL_REV_BG, sio->nt_hl_rev_bg);
    if (sio->fill_char_fg[0])
        ui_chg_color_hex(CLR_FILL_CHAR_FG, sio->fill_char_fg);
    if (sio->fill_char_bg[0])
        ui_chg_color_hex(CLR_FILL_CHAR_BG, sio->fill_char_bg);
    if (sio->brackets_fg[0])
        ui_chg_color_hex(CLR_BRACKETS_FG, sio->brackets_fg);
    if (sio->brackets_bg[0])
        ui_chg_color_hex(CLR_BRACKETS_BG, sio->brackets_bg);
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
    ui_shutdown(ui_runtime);
    restore_shell_tioctl();
    sig_dfl_mode();
    return;
}
void mbc_to_wc(wchar_t wc[2], const char mbc) {
    wc[0] = wc[1] = L'\0';
    mbstate_t state = {0};
    size_t len = mbrtowc(wc, &mbc, 0, &state);
    if (len <= 0) {
        wc[0] = L'?';
        wc[1] = L'\0';
        len = 1;
    }
}

wchar_t *mbstr_to_wcstr(const char *mb_str) {
    const char *src_ptr = mb_str;
    mbstate_t state = {0};
    size_t wc_count = mbsrtowcs(NULL, &src_ptr, 0, &state);
    if (wc_count == (size_t)-1)
        return nullptr;
    src_ptr = mb_str;
    wmemset((wchar_t *)&state, 0, sizeof(state) / sizeof(wchar_t));
    wchar_t *wc_str = malloc((wc_count + 1) * sizeof(wchar_t));
    if (!wc_str)
        return nullptr;
    mbsrtowcs(wc_str, &src_ptr, wc_count + 1, &state);
    return wc_str;
}

/** mbstr_to_cellstr
    @brief Convert multibyte string to complex character array
    @ingroup Chyron
    @param cmplx_buf Output buffer for complex characters
    @param str Input multibyte string
    @param attr Attributes to apply to the complex characters
    @param cpx Color pair index for the complex characters
    @param pos Pointer to current position in the output buffer, updated as
   characters are added
    @param maxlen Maximum length of the output buffer
    @return Number of bytes processed from the input string
    @details This function converts a multibyte string to an array of complex
   characters (cchar_t) that can be used with NCurses functions. It handles
   multibyte characters and applies the specified color pair to each character.
   The pos parameter is updated to reflect the current position in the output
   buffer, and the function ensures that it does not exceed the maximum length.
*/
#ifdef NCURSES_UI
uint mbstr_to_cellstr(UiCell *cmplx_buf, char *str, attr_t attr, uint cpx, uint *p, uint maxlen) {
    uint p1 = 0;
    uint *pos = &p1;
    if (p)
        pos = p;
    else
        pos = &p1;
    uint i = 0, len = 0;
    const char *s;
    UiCell cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    attr = WA_NORMAL;
    if (pos && *pos >= maxlen - 1)
        return 0;
    while (str[i] != '\0') {
        s = &str[i];
        len = mbrtowc(wstr, s, MB_CUR_MAX, &mbstate);
        if (len <= 0) {
            wstr[0] = L'?';
            wstr[1] = L'\0';
            len = 1;
        }
        wstr[1] = L'\0';
        if (*pos > maxlen)
            break;
        if (ui_setcchar(&cc, wstr, attr, cpx, nullptr) != ERR) {
            if (len > 0 && (*pos + len) < maxlen)
                cmplx_buf[(*pos)++] = cc;
        }
        i += len;
    }
    wstr[0] = L'\0';
    wstr[1] = L'\0';
    ui_setcchar(&cc, wstr, attr, cpx, nullptr);
    cmplx_buf[*pos] = cc;
    return *pos;
}
#endif
/** @brief Converts a Unicode code point to a UTF-8 encoded string.
    @ingroup utility_functions
    @param cp - Unicode code point to convert
    @param buffer - buffer to receive UTF-8 encoded string (must be at least 4 bytes)
    @returns number of bytes written to buffer, or 0 if cp is invalid
    @details This function encodes the given Unicode code point into its UTF-8 representation and stores it in the provided buffer. The caller must ensure that the buffer has enough space to hold the resulting UTF-8 string (up to 4 bytes for code points up to U+10FFFF). If the code point is invalid (e.g., greater than U+10FFFF), this function returns 0 and does not modify the buffer. */
int wccp_to_str(wchar_t cp, uint8_t *buffer) {
    if (cp <= 0x7F) {
        // 1-byte sequence: 0xxxxxxx
        buffer[0] = (uint8_t)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        buffer[0] = (uint8_t)(0xC0 | ((cp >> 6) & 0x1F));
        buffer[1] = (uint8_t)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp <= 0xFFFF) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        buffer[0] = (uint8_t)(0xE0 | ((cp >> 12) & 0x0F));
        buffer[1] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        buffer[2] = (uint8_t)(0x80 | (cp & 0x3F));
        return 3;
    } else if (cp <= 0x10FFFF) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        buffer[0] = (uint8_t)(0xF0 | ((cp >> 18) & 0x07));
        buffer[1] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
        buffer[2] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        buffer[3] = (uint8_t)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0; // Invalid Unicode code point
}
// ------------------->    box_win_new    <-------------------
int box_win_new(uint wlines, uint wcols, uint wbegy, uint wbegx, char *wtitle) {
    if (sfc_ptr >= MAXWIN) {
        Perror("Maximum number of windows (%d) exceeded");
        exit(EXIT_FAILURE);
    }
    uint maxy, maxx;
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
    wlines = min(wlines, maxy - 2);
    wcols = min(wcols, maxx - 2);
    sfc_ptr++;
    // ------------------->    UAL_win_box    <-------------------
    ui_surface[sfc_ptr] = ui_box_surface_new(ui_runtime, nullptr, 0, wlines, wcols, wbegy, wbegx, wtitle);
    UiSurface *sfc = ui_surface[sfc_ptr];
    ui_surface_addwin(sfc, WIN, BOX, wlines, wcols, 1, 1);
    return 0;
}
// ------------------->    box_split_new    <-------------------
int split_box_win_new(uint wlines, uint wcols, uint split_y, uint split_x, uint wbegy, uint wbegx, char *wtitle) {
    if (sfc_ptr >= MAXWIN) {
        Perror("Maximum number of windows (%d) exceeded");
        exit(EXIT_FAILURE);
    }
    if (sfc_ptr >= MAXWIN) {
        Perror("Maximum number of windows (%d) exceeded");
        exit(EXIT_FAILURE);
    }
    uint maxy, maxx;
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
    wlines = min(wlines, maxy - 2);
    wcols = min(wcols, maxx - 2);
    split_x = min(split_x, maxx - 2); // not implemented yet
    int split_wlines = min(wlines + split_y + 1, maxy - 2);
    wcols = min(wcols, maxx - 2);
    sfc_ptr++;
    // ------------------->    surface_new    <-------------------
    ui_surface[sfc_ptr] = ui_box_surface_new(ui_runtime, nullptr, 0, split_wlines, wcols, wbegy, wbegx, wtitle);
    UiSurface *sfc = ui_surface[sfc_ptr];

    ui_surface_addwin(sfc, WIN, BOX, wlines, wcols, 1, 1);
    ui_render(ui_runtime);

    border_ysplit(sfc, wlines + 1);
    ui_render(ui_runtime);
    ui_surface_addwin(sfc, WIN2, BOX, 2, wcols, wlines + 2, 1);
    ui_curs_set(0);
    return 0;
}
/** cm_destroy_surface
    @brief Destroy the most recently created surface
    @ingroup window_support
    @return 0 on success, -1 if no surfaces exist
    @details This function destroys the most recently created surface and decrements the surface pointer. It should be called when a surface is no longer needed to free up resources. If there are no surfaces to destroy, it returns -1.
    @note The difference between this function and ui_surface_destroy() is that this function destroys the surface pointed to by the surface pointer (sfc_ptr) and decrements the surface pointer after destroying the surface.
 */
int cm_surface_destroy(UiSurface *sfc) {
    if (sfc_ptr < 0)
        return -1;
    if (sfc != ui_surface[sfc_ptr])
        return -1;
    ui_surface_destroy(ui_surface[sfc_ptr]);
    sfc_ptr--;
    return 0;
}
int border_draw(UiSurface *sfc) {
    uint maxy = ui_getmaxy(sfc, BOX);
    uint maxx = ui_getmaxx(sfc, BOX);
    uint y = 0;
    uint x = 0;
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_tl, 1);
    ui_render(ui_runtime);
    for (x = 1; x < maxx - 1; x++)
        ui_mvwaddnwstr(sfc, BOX, y, x, &style_box, &bw_ho, 1);
    ui_mvwaddnwstr(sfc, BOX, y, maxx - 1, &style_box, &bw_tr, 1);
    for (y = 1; y < maxy - 1; y++) {
        ui_mvwaddnwstr(sfc, BOX, y, 0, &style_box, &bw_ve, 1);
        ui_mvwaddnwstr(sfc, BOX, y, maxx - 1, &style_box, &bw_ve, 1);
    }
    ui_mvwaddnwstr(sfc, BOX, y, 0, &style_box, &bw_bl, 1);
    for (x = 1; x < maxx - 1; x++)
        ui_mvwaddnwstr(sfc, BOX, y, x, &style_box, &bw_ho, 1);
    ui_mvwaddnwstr(sfc, BOX, y, maxx - 1, &style_box, &bw_br, 1);
    ui_render(ui_runtime);
    return 0;
}
/** border-ysplit
    @brief Draw a box with a separator line around the specified window
    @ingroup window_support
    @param box Pointer to the window to draw the box around
    @param y Line number where the separator line should be drawn
    @details This function draws a box around the specified window, similar to
   border_draw(), but it also includes a horizontal separator line that divides the box
   into two sections. The separator line is drawn at a fixed position (line 00,
   page 00) and extends across the width of the box. Use this function when you
   want to visually separate two sections within a window, such as for a header
   and content area. */
int border_ysplit(UiSurface *sfc, int y) {
    int maxx = ui_getmaxx(sfc, BOX);
    ui_mvwaddnwstr(sfc, BOX, y, 0, &style_box, &bw_lt, 1);
    for (int x = 1; x < maxx - 1; x++)
        ui_waddnwstr(sfc, BOX, &style_box, &bw_ho, 1);
    ui_mvwaddnwstr(sfc, BOX, y, maxx - 1, &style_box, &bw_rt, 1);
    return 0;
}
/** border_ysplit_text
    @brief Draw a box with a separator line and text around the specified window
    @ingroup window_support
    @param box Pointer to the window to draw the box around
    @param text Text to display in the middle of the separator line
    @param separator_line Line number where the separator line should be drawn
    @details This function draws a box around the specified window, similar to
   border_draw(), but it also includes a horizontal separator line that divides the box
   into two sections. The separator line is drawn at the specified line number and
   extends across the width of the box, with the provided text displayed in the
   middle of the line. Use this function when you want to visually separate two
   sections within a window and label the separator with descriptive text. */
int border_ysplit_text(UiSurface *sfc, char *text, int separator_line) {
    uint maxx = ui_getmaxx(sfc, BOX);
    uint l;
    uint y = separator_line;
    uint x = 0;
    // Draw the horizontal line with text in the middle, so we start by drawing
    // the left edge, then the text, then the right edge, and finally fill in the
    // horizontal line on either side of the text.
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_lt, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_ho, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_rt, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_sp, 1);
    strnz(text, maxx - 7);
    wchar_t *text_wc;
    text_wc = mbstr_to_wcstr(text);
    l = wcswidth(text_wc, wcslen(text_wc));
    ui_mvwaddnwstr(sfc, BOX, y, x, &style_box, text_wc, l);
    x += l;
    l = min(l, maxx - 7);
    free(text_wc);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_sp, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_lt, 1);

    while (x < maxx - 1)
        ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_ho, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_rt, 1);
    return 0;
}
/** border_title
    @brief Draw a box with a title around the specified window
    @ingroup window_support
    @param box Pointer to the window to draw the box around
    @param title Title text to display at the top of the box
    @details This function draws a box around the specified window, similar to
   border_draw(), but it also includes a title at the top of the box. The title
   is displayed in the center of the top edge of the box, and the horizontal line
   is drawn on either side of the title. Use this function when you want to
   visually label a window with a title. */
int border_title(UiSurface *sfc, char *title) {
    if (!title || !*title)
        return 0;
    uint y = 0;
    uint x = 0;
    uint l;
    uint maxx = ui_getmaxx(sfc, BOX);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_tl, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_rt, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_sp, 1);
    wchar_t *title_wc;
    title_wc = mbstr_to_wcstr(title);
    l = wcswidth(title_wc, wcslen(title_wc));
    l = min(l, maxx - 7);
    ui_mvwaddnwstr(sfc, BOX, y, x, &style_title, title_wc, l);
    x += l;
    free(title_wc);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_sp, 1);
    ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_lt, 1);
    while (x < maxx - 1)
        ui_mvwaddnwstr(sfc, BOX, y, x++, &style_box, &bw_ho, 1);
    ui_render(ui_runtime);
    return 0;
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

    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(chyron, 2, "N - No", 'n');
    set_chyron_key(chyron, 3, "Y - Yes", 'y');
    compile_chyron(chyron);

    uint maxy, maxx;
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
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
    if (box_win_new(5, msg_l + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "box_win_new(%d, %d, %d, %d, %s) failed", 5,
                  msg_l + 2, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, NULL, msg0);
    ui_draw_text(sfc, WIN, 1, 1, NULL, msg1);
    ui_draw_text(sfc, WIN, 2, 1, NULL, msg2);
    ui_draw_text(sfc, WIN, 3, 1, NULL, msg3);
    display_chyron(sfc, WIN, chyron, 4, chyron->l + 1);

    do {
        ui_curs_set(1);
        event.y = event.x = -1;
        cmd_key = ui_get_event(sfc, WIN, &event, -1);
        if (cmd_key == KEY_F(1) || cmd_key == 'N' || cmd_key == 'n' || cmd_key == 'Y' || cmd_key == 'y')
            break;
    } while (1);
    cm_surface_destroy(sfc);
    destroy_chyron(chyron);
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

    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(chyron, 9, "F9 Cancel", KEY_F(9));
    set_chyron_key(chyron, 10, "F10 Continue", KEY_F(10));
    compile_chyron(chyron);

    uint maxy, maxx;
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
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
    if (box_win_new(5, msg_l + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "box_win_new(%d, %d, %d, %d, %s) failed", 5,
                  msg_l + 2, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, NULL, msg0);
    ui_draw_text(sfc, WIN, 1, 1, NULL, msg1);
    ui_draw_text(sfc, WIN, 2, 1, NULL, msg2);
    ui_draw_text(sfc, WIN, 3, 1, NULL, msg3);

    display_chyron(sfc, WIN, chyron, 4, chyron->l + 1);
    do {
        event.y = event.x = -1;
        cmd_key = ui_get_event(sfc, WIN, &event, -1);
        if (cmd_key == KEY_F(9) || cmd_key == KEY_F(10) || cmd_key == 'q' || cmd_key == 'Q')
            break;
    } while (1);
    cm_surface_destroy(sfc);
    destroy_chyron(chyron);
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
    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(chyron, 9, "F9 Cancel", KEY_F(9));
    set_chyron_key(chyron, 10, "F10 Continue", KEY_F(10));
    compile_chyron(chyron);
    uint maxy, maxx;
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
    cols = strnz(emsg, maxx - 4);
    cols = max(cols, chyron->l);
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
    pos = (maxx - cols - 4) / 2;
    line = (maxy - 4) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (box_win_new(2, cols + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "box_win_new(%d, %d, %d, %d, %s, %b) failed",
                  4, line, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, NULL, emsg_str);
    display_chyron(sfc, WIN, chyron, 1, chyron->l + 1);
    if (f_xwgetch) {
        event.y = event.x = -1;
        in_key = ui_get_event(sfc, WIN, &event, -1);
    } else {
        in_key = KEY_F(10);
    }
    destroy_chyron(chyron);
    cm_surface_destroy(sfc);
    return (in_key);
}
/** action_disposition
    @brief Display a simple action disposition message window or print to stderr
    @ingroup error_handling
    @param title Window title
    @param action_str Action description string
    @return true if successful */
bool action_disposition(char *title, char *action_str) {
    int len;
    int line, col;

    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", title);
        fprintf(stderr, "%s\n", action_str);
        return true;
    }
    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 10, "F10 Continue", KEY_F(10));
    compile_chyron(chyron);
    len = max(strlen(title), strlen(action_str));
    uint maxy, maxx;
    ui_get_screen_size(ui_runtime, &maxy, &maxx);
    col = (maxx - len - 4) / 2;
    line = (maxy - 4) / 2;
    if (box_win_new(2, len + 2, line, col, title)) {
        ssnprintf(em0, MAXLEN - 1, "box_win_new(%d, %d, %d, %d, %s) failed", 4,
                  line, line, col, title);
        Perror(em0);
    }
    UiSurface *sfc = ui_surface[sfc_ptr];
    UiEvent event;
    ui_draw_text(sfc, WIN, 0, 1, NULL, action_str);
    display_chyron(sfc, WIN, chyron, 1, 0);
    event.y = event.x = -1;
    cmd_key = ui_get_event(sfc, WIN, &event, -1);
    cm_surface_destroy(sfc);
    destroy_chyron(chyron);
    return true;
}
/** @defgroup Chyron Chyron Management
    @brief Create and manage the Chyron
 */

/** new_chyron
   @brief Create and initialize Chyron structure
    @ingroup Chyron
    @return pointer to new Chyron structure
    @details This function allocates memory for a new Chyron structure and
   initializes the key pointers. Each key pointer is allocated memory for a
   ChyronKey structure. The Chyron structure is used to manage function key
   labels and their associated keycodes for mouse click handling in the chyron
   area of the interface.
    The use of calloc ensures that the allocated memory is initialized to
   zero, which means that the text for each key will be initialized to an empty
   string and the keycodes will be initialized to zero. This allows the
   is_set_chyron_key function to check if a key is set by checking if the first
   character of the text is not '\0'. If any memory allocation fails, the
   function will call abend to handle the error and return nullptr.
 */
Chyron *new_chyron() {
    Chyron *chyron = (Chyron *)calloc(1, sizeof(Chyron));
    if (!chyron) {
        abend(-1, "calloc chyron failed");
        return nullptr;
    }
    for (int i = 0; i < CHYRON_KEYS; i++) {
        chyron->key[i] = (ChyronKey *)calloc(1, sizeof(ChyronKey));
        if (!chyron->key[i]) {
            abend(-1, "calloc chyron->key[i] failed");
            return nullptr;
        }
    }
    return chyron;
}
int assign_chyron_win(Chyron *chyron, UiSurface *sfc, int win, char *y) {
    bool a_toi_error = false;
    if (!sfc)
        return -1;
    chyron->sfc = sfc;
    chyron->win = win;
    if (*y == '-')
        chyron->y = ui_getmaxy(sfc, win) - 1;
    else {
        chyron->y = a_toi(y, &a_toi_error);
        if (a_toi_error)
            return -1;
        chyron->y = min(chyron->y, ui_getmaxy(sfc, win) - 1);
    }
    return 0;
}
/** destroy_chyron
    @brief Destroy Chyron structure
    @ingroup Chyron
    @param chyron pointer to Chyron structure
    @return nullptr
 */
Chyron *destroy_chyron(Chyron *chyron) {
    int i;

    if (!chyron)
        return nullptr;
    for (i = 0; i < CHYRON_KEYS; i++) {
        if (chyron->key[i]) {
            free(chyron->key[i]);
            chyron->key[i] = nullptr;
        }
    }
    free(chyron);
    chyron = nullptr;
    return chyron;
}
/** is_set_chyron_key
    @brief Check if function key label is set
    @ingroup Chyron
    @param chyron structure
    @param k Function key index (0-19)
    @return true if set, false if not set */
bool is_set_chyron_key(Chyron *chyron, int k) {
    if (chyron->key[k]->text[0] != '\0')
        return true;
    else
        return false;
}
/** set_chyron_key_cp
    @brief Set chyron key with color pair (cp)
    @ingroup Chyron
    @param chyron structure
    @param k chyron key index (0-19)
    @param s chyron key label
    @param kc chyron key code
    @param cp color pair index for the key label
    @details This function is like set_chyron_key, except it includes a color
   pair numbers */
void set_chyron_key_cp(Chyron *chyron, int k, char *s, int kc, int cp) {
    if (*s != '\0')
        ssnprintf(chyron->key[k]->text, CHYRON_KEY_MAXLEN - 1, "%s", s);
    else
        chyron->key[k]->text[0] = '\0';
    chyron->key[k]->keycode = kc;
    chyron->key[k]->active = true;
    chyron->key[k]->cp = cp;
}
/** set_chyron_key
    @brief Set chyron key with default color pair (cp_nt_rev)
    @ingroup Chyron
    @param chyron structure
    @param k chyron key index (0-19)
    @param s chyron key label
    @param kc chyron key code
    @details This function sets the label and keycode for a function key in the
   chyron structure. It uses the default color pair cp_nt_rev for the key
   label. If the input string s is not empty, it copies the string into the
   chyron key's text field. If the input string is empty, it sets the first
   character of the text field to '\0' to indicate that the key is not set.
   The keycode is stored in the chyron key's keycode field, and the color pair
   index is set to cp_nt_rev.
 */
void set_chyron_key(Chyron *chyron, int k, char *s, int kc) {
    if (*s != '\0')
        ssnprintf(chyron->key[k]->text, CHYRON_KEY_MAXLEN - 1, "%s", s);
    else
        chyron->key[k]->text[0] = '\0';
    chyron->key[k]->keycode = kc;
    chyron->key[k]->active = true;
    chyron->key[k]->cp = cp_nt_rev;
}
/** unset_chyron_key
    @brief Unset chyron key
    @ingroup Chyron
    @param chyron structure
    @param k chyron_key index
*/
void unset_chyron_key(Chyron *chyron, int k) {
    chyron->key[k]->text[0] = '\0';
}
/** activate_chyron_key
    @brief Activate chyron key
    @ingroup Chyron
    @param chyron structure
    @param k chyron_key index
*/
void activate_chyron_key(Chyron *chyron, int k) {
    chyron->key[k]->active = true;
}
/** deactivate_chyron_key
    @brief Deactivate chyron key
    @ingroup Chyron
    @param chyron structure
    @param k chyron_key index
*/
void activate_all_chyron_keys(Chyron *chyron) {
    for (int k = 0; k < CHYRON_KEYS; k++)
        chyron->key[k]->active = true;
}
/** deactivate_chyron_key
    @brief Deactivate chyron key
    @ingroup Chyron
    @param chyron structure
    @param k chyron_key index
*/
void deactivate_chyron_key(Chyron *chyron, int k) {
    chyron->key[k]->active = false;
}
/** deactivate_all_chyron_keys
    @brief Deactivate all chyron keys
    @ingroup Chyron
    @param chyron structure
*/
void deactivate_all_chyron_keys(Chyron *chyron) {
    for (int k = 0; k < CHYRON_KEYS; k++)
        chyron->key[k]->active = false;
}
/**  compile_chyron
   @brief construct the chyron string from the chyron structure
    @ingroup Chyron
    @param chyron
    @details The chyron string is constructed by concatenating the labels of the
   set keys, separated by " | ". The end_pos values for each key are set to
   determine the zones for mouse clicks. When a mouse click occurs, the
   get_chyron_key function uses the end_pos values to determine which key was
   clicked based on the X position of the click.
*/
void compile_chyron(Chyron *chyron) {
    uint end_pos = 0;
    uint k = 0;
    uint pos = 0;
    uint cp = cp_nt_rev;
    UiCell *cx;
    char tmp_str[MAXLEN];
    while (k < CHYRON_KEYS) {
        if (chyron->key[k]->text[0] == '\0' || !chyron->key[k]->active) {
            k++;
            continue;
        }
        if (end_pos == 0) {
            cx = chyron->cmplx_buf;
            mbstr_to_cellstr(cx, " ", WA_NORMAL, cp_nt_rev, &pos, MAXLEN - 1);
        } else {
            mbstr_to_cellstr(chyron->cmplx_buf, "|", WA_NORMAL, cp_nt_rev, &pos,
                             MAXLEN - 1);
        }
        cx = chyron->cmplx_buf;
        if (chyron->key[k]->cp)
            cp = chyron->key[k]->cp;
        mbstr_to_cellstr(cx, chyron->key[k]->text, WA_NORMAL, cp, &pos, MAXLEN - 1);
        end_pos = pos;
        chyron->l = end_pos;
        chyron->key[k]->end_pos = end_pos;
        ssnprintf(tmp_str, MAXLEN - 1, "k=%d, text=%s, end_pos=%d", k,
                  chyron->key[k]->text, chyron->key[k]->end_pos);
        k++;
    }
    mbstr_to_cellstr(chyron->cmplx_buf, " ", WA_NORMAL, cp, &pos, MAXLEN - 1);
    chyron->l = end_pos;
}
/** display_chyron
 *   @brief Display chyron on the specified line and column of the surface
    @ingroup Chyron
    @param sfc Pointer to the UiSurface structure
    @param w Window index (WIN or BOX)
    @param chyron Pointer to the Chyron structure
    @param line Line number where the chyron should be displayed
    @param col Column number where the chyron should start
    @details This function displays the compiled chyron string on the specified
   line and column of the given surface. It first moves the cursor to the
   specified position, clears to the end of the line, and then writes the chyron
   string. Finally, it moves the cursor back to the specified column for user
   input.
 */
void display_chyron(UiSurface *sfc, int w, Chyron *chyron, int line, int col) {
    ui_cursor_move(sfc, w, line, 0);
    ui_wclrtoeol(sfc, w);
    ui_cursor_move(sfc, w, line, 0);
    ui_mvwadd_cellstr(sfc, w, line, 0, chyron->cmplx_buf);
    ui_cursor_move(sfc, w, line, col);
    return;
}
/** get_chyron_key
    @brief Get keycode from chyron
    @ingroup Chyron
    @param chyron structure
    @param x Mouse X position
    @return Keycode
    @details This function uses the end_pos values set in compile_chyron
    to determine which key was clicked.
    The chyron functions provide xwgetch() with a mechanism to translate
    mouse click positions into key codes based on the labels set in the chyron
   structure. When a mouse click occurs, xwgetch() can call get_chyron_key()
   with the X position of the click to determine which function key was clicked,
   allowing for dynamic and customizable function key behavior in the chyron
   area of the interface.
*/
int get_chyron_key(Chyron *chyron, uint x) {
    uint i = 0;
    int k = -1;
    while (i < CHYRON_KEYS - 1) {
        if (chyron->key[i]->text[0] != '\0' && chyron->key[i]->active)
            if (chyron->key[i]->end_pos >= x) {
                k = i;
                break;
            }
        i++;
    }
    if (k == -1)
        return 0;
    return chyron->key[k]->keycode;
}
/** nf_error
 *   @brief Display an error message and wait for user input
    @ingroup error_handling
    @param ec Error code
    @param s Error message
    @return Error code
    @details This function prints an error message to stderr, waits for the user
   to press a key, and then returns the provided error code. It is useful for
   displaying error messages in a non-curses environment or when curses is not
   initialized.
 */
int nf_error(int ec, char *s) {
    fprintf(stderr, "ERROR: %s code: %d\n", s, ec);
    fprintf(stderr, "Press a key to continue");
    di_getch();
    fprintf(stderr, "\n");
    return ec;
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
