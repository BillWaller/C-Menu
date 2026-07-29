/** @file ui_ual.c
    @author Bill Waller
    Copyright (c) 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

/** @defgroup ui_ual UI Abstraction Layer
    @brief Platform Independent User Interface
 */

#include "../include/ui_backend.h"
#include "ui_ual.h"
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

#define NC true

enum WinFlags {
    WF_BOX = 0b00000001,
    WF_WIN = 0b00000010,
    WF_WIN2 = 0b00000100
};

UiRuntime *ui_runtime;
UiConfig *ui_config;
UiSurface *ui_box[MAXWIN];
UiSurface *ui_win[MAXWIN];
UiSurface *ui_win2[MAXWIN];
UiRect ui_rect;

void ui_rect_set(UiRect *, int, int, int, int);
void abend(int, char *);
int nf_error(int, char *);
int Perror(char *);
int click_y;
int click_x;
int clr_name_to_idx(char *);
void init_hex_clr(int, char *);
RGB hex_clr_str_to_rgb(char *);
RGB xterm256_idx_to_rgb(int);
int rgb_to_xterm256_idx(RGB *);
void apply_gamma(RGB *);

cchar_t ls, rs, ts, bs, tl, tr, bl, br, lt, rt, sp, ra, la, ua, da, ran, chk;

/** StdColors
    @details Standard 16 colors for xterm256 color conversions These colors can
   be overridden in ".minitrc" */
RGB StdColors[16] = {
    {0, 0, 0}, {128, 0, 0}, {0, 128, 0}, {128, 128, 0}, {0, 0, 128}, {128, 0, 128}, {0, 128, 128}, {192, 192, 192}, {128, 128, 128}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};
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

double GRAY_GAMMA = 1.2;  /**< Gamma correction value for gray colors. Set in .minitrc */
double RED_GAMMA = 1.2;   /**< Gamma correction value for red colors. Set in .minitrc */
double GREEN_GAMMA = 1.2; /**< Gamma correction value for green colors. Set in .minitrc */
double BLUE_GAMMA = 1.2;  /**< Gamma correction value for blue colors. Set in .minitrc */

int exit_code;
unsigned int cmd_key;
bool f_sigwench = false;
int win_attr;
int box_attr;
int win_ptr;
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
int cp_box;
int cp_ind;
int cp_cmdln;
int cp_title;
int cp_nt;
int cp_nt_rev;
int cp_nt_hl;
int cp_nt_hl_rev;
int cp_ln;
int cp_norm;
int cp_fill_char;
int cp_brackets;
int cp_red;
int cp_green;
int cp_yellow;
int cp_blue;
int clr_cnt = 0;
int clr_pair_idx = 1;
int clr_pair_cnt = 1;

void initialize_local_colors(SIO *sio) {
    RED_GAMMA = sio->red_gamma;
    GREEN_GAMMA = sio->green_gamma;
    BLUE_GAMMA = sio->blue_gamma;
    GRAY_GAMMA = sio->gray_gamma;
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
        rgb.r = StdColors[idx].r;
        rgb.g = StdColors[idx].g;
        rgb.b = StdColors[idx].b;
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
   it is applied using the init_hex_clr function. After processing all colors,
   the clr_cnt variable is set to CLR_NCOLORS to indicate that the standard
   colors have been initialized. */
bool init_clr_palette(SIO *sio) {
    if (sio->black[0])
        init_hex_clr(CLR_BLACK, sio->black);
    if (sio->red[0])
        init_hex_clr(CLR_RED, sio->red);
    if (sio->green[0])
        init_hex_clr(CLR_GREEN, sio->green);
    if (sio->yellow[0])
        init_hex_clr(CLR_YELLOW, sio->yellow);
    if (sio->blue[0])
        init_hex_clr(CLR_BLUE, sio->blue);
    if (sio->magenta[0])
        init_hex_clr(CLR_MAGENTA, sio->magenta);
    if (sio->cyan[0])
        init_hex_clr(CLR_CYAN, sio->cyan);
    if (sio->white[0])
        init_hex_clr(CLR_WHITE, sio->white);
    if (sio->bblack[0])
        init_hex_clr(CLR_BBLACK, sio->bblack);
    if (sio->bred[0])
        init_hex_clr(CLR_BRED, sio->bred);
    if (sio->bgreen[0])
        init_hex_clr(CLR_BGREEN, sio->bgreen);
    if (sio->byellow[0])
        init_hex_clr(CLR_BYELLOW, sio->byellow);
    if (sio->bblue[0])
        init_hex_clr(CLR_BBLUE, sio->bblue);
    if (sio->bmagenta[0])
        init_hex_clr(CLR_BMAGENTA, sio->bmagenta);
    if (sio->bcyan[0])
        init_hex_clr(CLR_BCYAN, sio->bcyan);
    if (sio->bwhite[0])
        init_hex_clr(CLR_BWHITE, sio->bwhite);
    if (sio->borange[0])
        init_hex_clr(CLR_BORANGE, sio->borange);
    if (sio->fg[0])
        init_hex_clr(CLR_FG, sio->fg);
    if (sio->bg[0])
        init_hex_clr(CLR_BG, sio->bg);
    if (sio->box_fg[0])
        init_hex_clr(CLR_BOX_FG, sio->box_fg);
    if (sio->box_bg[0])
        init_hex_clr(CLR_BOX_BG, sio->box_bg);
    if (sio->ind_fg[0])
        init_hex_clr(CLR_IND_FG, sio->ind_fg);
    if (sio->ind_bg[0])
        init_hex_clr(CLR_IND_BG, sio->ind_bg);
    if (sio->title_fg[0])
        init_hex_clr(CLR_TITLE_FG, sio->title_fg);
    if (sio->title_bg[0])
        init_hex_clr(CLR_TITLE_BG, sio->title_bg);
    if (sio->ln_fg[0])
        init_hex_clr(CLR_LN_FG, sio->ln_fg);
    if (sio->ln_bg[0])
        init_hex_clr(CLR_LN_BG, sio->ln_bg);
    if (sio->nt_fg[0])
        init_hex_clr(CLR_NT_FG, sio->nt_fg);
    if (sio->nt_bg[0])
        init_hex_clr(CLR_NT_BG, sio->nt_bg);
    if (sio->nt_rev_fg[0])
        init_hex_clr(CLR_NT_REV_FG, sio->nt_rev_fg);
    if (sio->nt_rev_bg[0])
        init_hex_clr(CLR_NT_REV_BG, sio->nt_rev_bg);
    if (sio->nt_hl_fg[0])
        init_hex_clr(CLR_NT_HL_FG, sio->nt_hl_fg);
    if (sio->nt_hl_bg[0])
        init_hex_clr(CLR_NT_HL_BG, sio->nt_hl_bg);
    if (sio->nt_hl_rev_fg[0])
        init_hex_clr(CLR_NT_HL_REV_FG, sio->nt_hl_rev_fg);
    if (sio->nt_hl_rev_bg[0])
        init_hex_clr(CLR_NT_HL_REV_BG, sio->nt_hl_rev_bg);
    if (sio->fill_char_fg[0])
        init_hex_clr(CLR_FILL_CHAR_FG, sio->fill_char_fg);
    if (sio->fill_char_bg[0])
        init_hex_clr(CLR_FILL_CHAR_BG, sio->fill_char_bg);
    if (sio->brackets_fg[0])
        init_hex_clr(CLR_BRACKETS_FG, sio->brackets_fg);
    if (sio->brackets_bg[0])
        init_hex_clr(CLR_BRACKETS_BG, sio->brackets_bg);
    clr_cnt = CLR_NCOLORS;
    return true;
}
/** init_hex_clr
    @brief Initialize extended ncurses color from HTML style hex string
    @ingroup color_management
    @param idx Color index
    @param s Hex color string
    @details NCurses uses 0-1000 for RGB values, so the RGB values from the hex
   string are converted to this range before initializing the color. If the
   color index is less than 16, the RGB values are also stored in the StdColors
   array for reference.
    */
void init_hex_clr(int idx, char *s) {
    RGB rgb;
    rgb = hex_clr_str_to_rgb(s);
    apply_gamma(&rgb);
    if (idx < 16) {
        StdColors[idx].r = rgb.r;
        StdColors[idx].g = rgb.g;
        StdColors[idx].b = rgb.b;
    }
    rgb.r = (rgb.r * 1000) / 255;
    rgb.g = (rgb.g * 1000) / 255;
    rgb.b = (rgb.b * 1000) / 255;
    init_extended_color(idx, rgb.r, rgb.g, rgb.b);
}
/** hex_clr_str_to_rgb
    @brief Convert six-digit HTML style hex color code to RGB struct
    @ingroup color_management
    @param s six-digit HTML style hex color code */
RGB hex_clr_str_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
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
/** ui_rect_set
    @brief Set the properties of a UiRect structure
    @ingroup utility_functions
    @param r Pointer to the UiRect structure to set
    @param y Y-coordinate (row) of the rectangle's top-left corner
    @param x X-coordinate (column) of the rectangle's top-left corner
    @param h Height (number of rows) of the rectangle
    @param w Width (number of columns) of the rectangle
    @details This function initializes the fields of a UiRect structure
    with the specified values for position and size. It is used to define
    rectangular areas in a user interface. */
void ui_rect_set(UiRect *r, int y, int x, int h, int w) {
    r->y = y;
    r->x = x;
    r->rows = h;
    r->cols = w;
}
/** @brief Display a message in a window or print to stderr if curses is not
 * available
    @ingroup error_handling
    @param msg Message to display
    @return Pointer to the created window, or nullptr if curses is not available or screen is too small */
WINDOW *
message_win(char *msg) {
    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n\n", msg);
        return nullptr;
    }
    if (LINES < 4 || COLS < 42)
        return nullptr;
    int wlines = 3, wcols = 40;
    int wbegy = 0;
    int wbegx = COLS - wcols - 2;
    WINDOW *win = subwin(stdscr, wlines, wcols, wbegy, wbegx);
    if (win == nullptr)
        return win;
    wbkgrndset(win, &CC_BOX);
    wborder_set(win, &ls, &rs, &ts, &bs, &tl, &tr, &bl, &br);
    strnz(msg, 40);
    mvwaddstr(win, 0, 1, msg);
    update_panels();
    doupdate();
    return win;
}
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
    int line, pos, msg_l, msg0_l, msg1_l, msg2_l, msg3_l;
    WINDOW *error_win;

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n%s\n%s\n%s\n\n", msg0, msg1, msg2, msg3);
        return 1;
    }

    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(chyron, 2, "N - No", 'n');
    set_chyron_key(chyron, 3, "Y - Yes", 'y');
    compile_chyron(chyron);

    msg0_l = strnz(msg0, COLS - 4);
    msg1_l = strnz(msg1, COLS - 4);
    msg2_l = strnz(msg2, COLS - 4);
    msg3_l = strnz(msg1, COLS - 4);
    msg_l = max(msg0_l, msg1_l);
    msg_l = max(msg_l, msg2_l);
    msg_l = max(msg_l, msg3_l);
    msg_l = max(msg_l, chyron->l);
    msg_l = min(msg_l, COLS - 4);

    pos = ((COLS - msg_l) - 4) / 2;
    line = (LINES - 6) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (box_new(5, msg_l + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s) failed", 5,
                  msg_l + 2, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, msg0);
    mvwaddstr(error_win, 1, 1, msg1);
    mvwaddstr(error_win, 2, 1, msg2);
    mvwaddstr(error_win, 3, 1, msg3);
    display_chyron(error_win, chyron, 4, chyron->l + 1);

    do {
        curs_set(1);
        cmd_key = xwgetch(error_win, chyron, -1);
        curs_set(0);
        if (cmd_key == KEY_F(1) || cmd_key == 'N' || cmd_key == 'n' || cmd_key == 'Y' || cmd_key == 'y')
            break;
    } while (1);
    win_del();
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
    int line, pos, msg_l, msg0_l, msg1_l, msg2_l, msg3_l;
    WINDOW *error_win;

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

    msg0_l = strnz(msg0, COLS - 4);
    msg1_l = strnz(msg1, COLS - 4);
    msg2_l = strnz(msg2, COLS - 4);
    msg3_l = strnz(msg1, COLS - 4);
    msg_l = max(msg0_l, msg1_l);
    msg_l = max(msg_l, msg2_l);
    msg_l = max(msg_l, msg3_l);
    msg_l = max(msg_l, chyron->l);
    msg_l = min(msg_l, COLS - 4);

    pos = ((COLS - msg_l) - 4) / 2;
    line = (LINES - 6) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (box_new(5, msg_l + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s) failed", 5,
                  msg_l + 2, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, msg0);
    mvwaddstr(error_win, 1, 1, msg1);
    mvwaddstr(error_win, 2, 1, msg2);
    mvwaddstr(error_win, 3, 1, msg3);
    display_chyron(error_win, chyron, 4, chyron->l + 1);
    do {
        cmd_key = xwgetch(error_win, chyron, -1);
        if (cmd_key == KEY_F(9) || cmd_key == KEY_F(10) || cmd_key == 'q' || cmd_key == 'Q')
            break;
    } while (1);
    win_del();
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
    WINDOW *error_win;
    int line, pos, cols;
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
    cols = strnz(emsg, COLS - 4);
    cols = max(cols, chyron->l);
    pos = (COLS - cols - 4) / 2;
    line = (LINES - 4) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (box_new(2, cols + 2, line, pos, title)) {
        ssnprintf(title, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s, %b) failed",
                  4, line, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, emsg);
    display_chyron(error_win, chyron, 1, chyron->l + 1);
    if (f_xwgetch) {
        curs_set(1);
        in_key = xwgetch(error_win, chyron, -1);
        curs_set(0);
    } else {
        in_key = KEY_F(10);
    }
    destroy_chyron(chyron);
    win_del();
    return (in_key);
}
/** wait_mk_chyron
    @brief Create a Chyron struct for the waiting message
    @ingroup error_handling
    @return Pointer to the chyron struct */
Chyron *
wait_mk_chyron() {
    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 9, "F9 Cancel", KEY_F(9));
    compile_chyron(chyron);
    return chyron;
}
/** wait_mk_win
    @brief Display a popup waiting message
    @ingroup error_handling
    @param chyron Pointer to Chyron struct for displaying key options
    @param title window title
    @return WINDOW * struct */
WINDOW *
wait_mk_win(Chyron *chyron, char *title) {
    char wm1[] = "Seconds remaining:";
    int len;
    int line, col;
    WINDOW *wait_win;

    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", title);
        fprintf(stderr, "%s\n", wm1);
        return NULL;
    }
    len = max(strlen(title), strlen(wm1));
    len = max(len, chyron->l);
    len = max(len, 40);
    col = (COLS - len - 4) / 2;
    line = (LINES - 4) / 2;
    if (box_new(2, len + 2, line, col, title)) {
        ssnprintf(title, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s) failed", 4,
                  line, line, col, title);
        abend(-1, title);
    }
    wait_win = win_win[win_ptr];
    mvwaddstr(wait_win, 0, 1, wm1);
    display_chyron(wait_win, chyron, 1, 0);
    wmove(wait_win, 1, chyron->l);
    return wait_win;
}
/** wait_destroy
    @brief Destroy the waiting message window and chyron
    @ingroup error_handling
    @param chyron Pointer to Chyron struct for displaying key options
    @return true if successful */
bool wait_destroy(Chyron *chyron) {
    win_del();
    destroy_chyron(chyron);
    update_panels();
    doupdate();
    return true;
}
/** wait_continue
    @brief Update the waiting message with remaining time and check for user
   input
    @ingroup error_handling
    @param chyron Pointer to Chyron struct for displaying key options
    @param wait_win Pointer to the waiting message window
    @param remaining Time remaining for the wait in seconds
    @return true if the wait should continue, false if it should be cancelled */
int wait_continue(WINDOW *wait_win, Chyron *chyron, int remaining) {
    char time_str[10];
    ssnprintf(time_str, 9, "%-4d", remaining);
    mvwaddstr(wait_win, 0, 21, time_str);
    display_chyron(wait_win, chyron, 1, 0);
    wmove(wait_win, 1, chyron->l);
    cmd_key = xwgetch(wait_win, chyron, 1);
    return cmd_key;
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
    WINDOW *action_disposition_win;

    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", title);
        fprintf(stderr, "%s\n", action_str);
        return true;
    }
    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 10, "F10 Continue", KEY_F(10));
    compile_chyron(chyron);
    len = max(strlen(title), strlen(action_str));
    col = (COLS - len - 4) / 2;
    line = (LINES - 4) / 2;
    if (box_new(2, len + 2, line, col, title)) {
        ssnprintf(em0, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s) failed", 4,
                  line, line, col, title);
        Perror(em0);
    }
    action_disposition_win = win_win[win_ptr];
    mvwaddstr(action_disposition_win, 0, 1, action_str);
    display_chyron(action_disposition_win, chyron, 1, 0);
    wmove(action_disposition_win, 1, chyron->l);
    cmd_key = xwgetch(action_disposition_win, chyron, 1);
    win_del();
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
    int end_pos = 0;
    int k = 0;
    int pos = 0;
    int cp = cp_nt_rev;
    cchar_t *cx;
    char tmp_str[MAXLEN];
    while (k < CHYRON_KEYS) {
        if (chyron->key[k]->text[0] == '\0' || !chyron->key[k]->active) {
            k++;
            continue;
        }
        if (end_pos == 0) {
            cx = chyron->cmplx_buf;
            mb_to_cc(cx, " ", WA_NORMAL, cp_nt_rev, &pos, MAXLEN - 1);
        } else {
            mb_to_cc(chyron->cmplx_buf, "|", WA_NORMAL, cp_nt_rev, &pos,
                     MAXLEN - 1);
        }
        cx = chyron->cmplx_buf;
        if (chyron->key[k]->cp)
            cp = chyron->key[k]->cp;
        mb_to_cc(cx, chyron->key[k]->text, WA_NORMAL, cp, &pos, MAXLEN - 1);
        end_pos = pos;
        chyron->l = end_pos;
        chyron->key[k]->end_pos = end_pos;
        ssnprintf(tmp_str, MAXLEN - 1, "k=%d, text=%s, end_pos=%d", k,
                  chyron->key[k]->text, chyron->key[k]->end_pos);
        k++;
    }
    mb_to_cc(chyron->cmplx_buf, " ", WA_NORMAL, cp, &pos, MAXLEN - 1);
    chyron->l = end_pos;
}
/** display_chyron
   @brief Display chyron on window
    @ingroup Chyron
    @param win NCurses window to display chyron on
    @param chyron Chyron structure containing the compiled chyron string
    @param line Line number to display the chyron on
    @param col Column number to start displaying the chyron from
    @details This function clears the line where the chyron will be displayed,
   then uses wadd_wchstr to add the compiled chyron string (cmplx_buf) to the
   window. Finally, it moves the cursor to the specified column position.
*/
void display_chyron(WINDOW *win, Chyron *chyron, int line, int col) {
    wmove(win, line, 0);
    wclrtoeol(win);
    wmove(win, line, 0);
    wadd_wchstr(win, chyron->cmplx_buf);
    wmove(win, line, col);
    update_panels();
    doupdate();
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
int get_chyron_key(Chyron *chyron, int x) {
    int i = 0;
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

/** mvwaddstr_fill
    @brief For lines shorter than their display area, fill the rest with spaces
    @ingroup window_support
    @param w Pointer to window
    @param y Y coordinate
    @param x X coordinate
    @param s String to display
    @param l Length of display area */
void mvwaddstr_fill(WINDOW *w, int y, int x, char *s, int l) {
    char *d, *e;
    int maxy, maxx;
    char tmp_str[MAXLEN];
    getmaxyx(w, maxy, maxx);
    y = min(y, maxy);
    l = min(l, maxx);
    l = min(l, MAXLEN - 1);
    e = d = tmp_str;
    e += l;
    while (d < e) {
        if (*s == '\0' || *s == '\n')
            *d++ = ' ';
        else
            *d++ = *s++;
    }
    *d = '\0';
    l = strlen(tmp_str);
    mvwaddstr(w, y, x, tmp_str);
}
/** clr_name_to_idx
    @brief Get color index from color name
    @ingroup color_management
    @param s Color name
    @return Color index or -1 if not found */
int clr_name_to_idx(char *s) {
    int i = 0;
    int n = 16;

    str_to_lower(s);
    while (i < n) {
        if (!strcmp(colors_text[i], s))
            break;
        i++;
    }
    if (i >= n)
        return (-1);
    return (i);
}
/** list_colors
    @brief list colors to stderr
    @ingroup color_management
    @details only lists the first 16, since that's how many we let the
    user redefine */
void list_colors() {
    int i, col;

    for (i = 0, col = 0; i < 16; i++, col++) {
        if (i < 8) {
            fprintf(stderr, " ");
        }
        if (i == 8) {
            col = 0;
            fprintf(stderr, "\n");
        } else if (col > 0)
            fprintf(stderr, " ");
        fprintf(stderr, "%s", colors_text[i]);
    }
    fprintf(stderr, "\n");
}
/** nf_error
    @brief Display error message and wait for key press
    @ingroup error_handling
    @param ec Error code
    @param s Error message */
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
/** waitpid_with_timeout
    @brief Wait for a process to finish with a timeout and optional user
   cancellation
    @ingroup error_handling
    @param pid Process ID to wait for
    @param timeout Time in seconds to wait before timing out
    @return true if the process finished, false if it timed out or was cancelled
 */
bool waitpid_with_timeout(pid_t pid, int timeout) {
    int status;
    Chyron *wait_chyron;
    WINDOW *wait_win;
    int remaining = timeout;
    bool rc = false;

    waitpid(pid, &status, WNOHANG);
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        return true;
    }
    usleep(100000); // Sleep for 200ms */
    wait_chyron = wait_mk_chyron();
    ssnprintf(em0, MAXLEN - 1, "Waiting for process %d to finish...", pid);
    wait_win = wait_mk_win(wait_chyron, em0);
    cmd_key = 0;
    while (remaining > 0 && cmd_key != KEY_F(9)) {
        cmd_key = wait_continue(wait_win, wait_chyron, remaining);
        if (cmd_key == KEY_F(9))
            break;
        if (cmd_key == KEY_F(10)) {
            remaining = timeout;
            continue;
        }
        remaining--;
        waitpid(pid, &status, WNOHANG);
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            rc = true;
            break;
        }
    }
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    win_del();
    destroy_chyron(wait_chyron);
    update_panels();
    doupdate();
    return rc;
}
/** xwgetch
    @brief Wrapper for wgetch that handles signals, mouse events, checks for
   clicks on the chyron line, and accepts a sinigle character answer
    @ingroup window_support
    @param win Pointer to window
    @param chyron Pointer to chyron struct
    @param n Number of seconds to wait before timing out
    @verbatim

        0: Wait indefinitely for user input (raw mode)
            accept a single character answer, and don't wait for Enter key
        1: Wait for 1 decisecond
        n > 1: Wait for n/10 seconds

    @endverbatim
    @return Key code or ERR if interrupted by signal
    @details Get mouse event and check if it's a left click or double click. If
   the click is outside the window, ignore it. If it's on the chyron line, get
   the corresponding key command. Otherwise, store the click coordinates as
   click_y and click_x for later use. */
int xwgetch(WINDOW *win, Chyron *chyron, int n) {
    int c;
    MEVENT event;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED,
              nullptr);
    click_y = event.y = -1;
    click_x = event.x = -1;

    if (n == -1) {
        struct termios raw_tioctl;
        raw_tioctl = curses_tioctl;
        mk_raw_tioctl(&raw_tioctl);
    } else if (n == 0)
        halfdelay(1);
    else
        halfdelay(min(255, max(0, n * 10)));
    tcflush(2, TCIFLUSH);
    do {
        curs_set(1);
        c = wgetch(win);
        curs_set(0);
        if (sig_received != 0) {
            if (handle_signal(sig_received))
                c = display_error(em0, em1, em2, nullptr);
            if (c == 'q' || c == 'Q' || c == KEY_F(9))
                exit(EXIT_FAILURE);
        }
        if (n > 0 && c == ERR) {
            c = 0;
            break;
        }
        if (c == ERR)
            continue;
        if (c == KEY_MOUSE) {
            if (getmouse(&event) != OK) {
                c = 0;
                continue;
            }
            if (event.bstate & BUTTON4_PRESSED) {
                return KEY_UP;
            } else if (event.bstate & BUTTON5_PRESSED) {
                return KEY_DOWN;
            }
            if (event.bstate & BUTTON1_CLICKED || event.bstate & BUTTON1_DOUBLE_CLICKED) {
                if (wenclose(win, event.y, event.x)) {
                    wmouse_trafo(win, &event.y, &event.x, false);
                    click_y = event.y;
                    click_x = event.x;
                    if (chyron && event.y == getmaxy(win) - 1) {
                        c = get_chyron_key(chyron, event.x);
                        break;
                    } else
                        break;
                }
                c = ERR;
                continue;
            }
        }
    } while (c == ERR);
    restore_curses_tioctl();
    return c;
}
/** dxwgetch
    @brief Wrapper for wgetch that handles signals, mouse events, checks for
   clicks on the chyron line, and accepts a sinigle character answer
    @ingroup window_support
    @param win_0 Pointer to window 0
    @param win_1 Pointer to window 1
    @param win_2 Pointer to window 2
    @param win_3 Pointer to window 3
    @param win_c Pointer to chyron window
    @param chyron Pointer to chyron struct
    @param n Number of seconds to wait before timing out
    @verbatim

        0: Wait indefinitely for user input (raw mode)
            accept a single character answer, and don't wait for Enter key
        1: Wait for 1 decisecond
        n > 1: Wait for n/10 seconds

    @endverbatim
    @return Key code or ERR if interrupted by signal
    @details Get mouse event and check if it's a left click or double click. If
   the click is outside the windows, ignore it. If it's on the chyron line, get
   the corresponding key command. Otherwise, store the click coordinates as
   click_y and click_x for later use. */
int dxwgetch(WINDOW *win_0, WINDOW *win_1, WINDOW *win_2, WINDOW *win_3, WINDOW *win_c, Chyron *chyron, int n) {
    int c;
    MEVENT event;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED,
              nullptr);
    click_y = event.y = -1;
    click_x = event.x = -1;

    if (n == -1) {
        struct termios raw_tioctl;
        raw_tioctl = curses_tioctl;
        mk_raw_tioctl(&raw_tioctl);
    } else if (n == 0)
        halfdelay(1);
    else
        halfdelay(min(255, max(0, n * 10)));
    tcflush(2, TCIFLUSH);
    do {
        curs_set(1);
        c = wgetch(win_0);
        curs_set(0);
        if (sig_received != 0) {
            if (handle_signal(sig_received))
                c = display_error(em0, em1, em2, nullptr);
            if (c == 'q' || c == 'Q' || c == KEY_F(9))
                exit(EXIT_FAILURE);
        }
        if (n > 0 && c == ERR) {
            c = 0;
            break;
        }
        if (c == ERR)
            continue;
        if (c == KEY_MOUSE) {
            if (getmouse(&event) != OK) {
                c = 0;
                continue;
            }
            if (event.bstate & BUTTON4_PRESSED) {
                return KEY_UP;
            } else if (event.bstate & BUTTON5_PRESSED) {
                return KEY_DOWN;
            }
            if (event.bstate & BUTTON1_CLICKED || event.bstate & BUTTON1_DOUBLE_CLICKED) {
                // Check if the click is in win_0, win_1, or win_2, and set
                // mouse_win
                // accordingly
                // don't free mouse_win, since it is borrowed
                mouse_win = nullptr;
                if (win_1 != nullptr && wenclose(win_1, event.y, event.x) && wmouse_trafo(win_1, &event.y, &event.x, false))
                    mouse_win = win_1;
                else if (win_2 != nullptr && wenclose(win_2, event.y, event.x) && wmouse_trafo(win_2, &event.y, &event.x, false))
                    mouse_win = win_2;
                else if (win_3 != nullptr && wenclose(win_3, event.y, event.x) && wmouse_trafo(win_3, &event.y, &event.x, false))
                    mouse_win = win_3;
                click_y = event.y;
                click_x = event.x;
                if (mouse_win == nullptr) {
                    c = 0;
                    break;
                }
                if (mouse_win == win_c && chyron && (event.y == getmaxy(mouse_win) - 1))
                    c = get_chyron_key(chyron, event.x);
                break;
            }
        }
    } while (c == ERR);
    restore_curses_tioctl();
    return c;
}
/** vgetch
    @brief Wrapper for wgetch that handles signals and mouse events, and accepts a single character answer
    @ingroup window_support
    @param win Pointer to window
    @param n Number of seconds to wait before timing out
    @return Key code or ERR if interrupted by signal
    @details This function is similar to xwgetch, but it does not handle chyron clicks. It sets the terminal to raw mode if n is -1, or halfdelay mode if n is 0 or greater. It waits for user input and returns the key code. If a signal is received, it handles the signal and may display an error message. If the user presses 'q', 'Q', or F9, the program exits.
 */
int vgetch(WINDOW *win, int n) {
    int c;
    mousemask(0, nullptr);

    tcflush(2, TCIFLUSH);
    curs_set(1);
    if (n == -1) {
        struct termios raw_tioctl;
        raw_tioctl = curses_tioctl;
        mk_raw_tioctl(&raw_tioctl);
    } else if (n == 0)
        halfdelay(1);
    else
        halfdelay(min(255, max(0, n * 10)));
    do {
        c = wgetch(win);
        if (n > 0 && c == ERR) {
            c = 0;
            break;
        }
    } while (c == ERR);
    curs_set(0);
    // restore_curses_tioctl();
    return c;
}
