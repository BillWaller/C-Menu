/** @file dwin.c
    @brief Window support for C-Menu Menu, Form, Pick, and View
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "cm.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

bool open_curses(SIO *);
void destroy_curses();
int win_new(int, int, int, int, char *, int);
void win_redraw(WINDOW *);
void win_resize(int, int, char *);
WINDOW *win_del();
void restore_wins();
void cbox(WINDOW *);
void win_init_attrs(int, int, int);
int Perror(char *);
void mvwaddstr_fill(WINDOW *, int, int, char *, int);
void display_argv_error_msg(char *, char **);
void abend(int, char *);
void user_end();
int nf_error(int, char *);

void set_fkey(int, char *);
bool is_set_fkey(int);
void unset_fkey(int);
int chyron_mk(key_cmd_tbl *, char *);
int get_chyron_key(key_cmd_tbl *, int);

void list_colors();
int clr_name_to_idx(char *);
void init_hex_clr(int, char *);
int rgb_clr_to_cube(int);
RGB hex_clr_str_to_rgb(char *);
RGB xterm256_idx_to_rgb(int);
int rgb_to_curses_clr(RGB *);
int rgb_to_xterm256_idx(RGB *);
void apply_gamma(RGB *);

bool init_clr_palette(SIO *);
cchar_t mkccc(int cp);

SIO *sio; /**< Global pointer to SIO struct for terminal and color settings */

/** @enum colors_enum
    @note Used for xterm256 color conversions
    @note These colors can be overridden in ".minitrc" */
enum colors_enum {
    CLR_BLACK,
    CLR_RED,
    CLR_GREEN,
    CLR_YELLOW,
    CLR_BLUE,
    CLR_MAGENTA,
    CLR_CYAN,
    CLR_WHITE,
    CLR_BBLACK,
    CLR_BRED,
    CLR_BGREEN,
    CLR_BYELLOW,
    CLR_BBLUE,
    CLR_BMAGENTA,
    CLR_BCYAN,
    CLR_BWHITE,
    CLR_BORANGE,
    CLR_NCOLORS
};
/** StdColors
    @note Standard 16 colors for xterm256 color conversions
    @note These colors can be overridden in ".minitrc" */
RGB StdColors[16] = {
    {0, 0, 0},       {128, 0, 0},   {0, 128, 0},   {128, 128, 0},
    {0, 0, 128},     {128, 0, 128}, {0, 128, 128}, {192, 192, 192},
    {128, 128, 128}, {255, 0, 0},   {0, 255, 0},   {255, 255, 0},
    {0, 0, 255},     {255, 0, 255}, {0, 255, 255}, {255, 255, 255}};
/** colors_text
    @note Color names for .minitrc overrides
    @note These names are used in .minitrc to specify color overrides
    @note The order of these names corresponds to the colors_enum values */
char const colors_text[][10] = {
    "black",   "red",    "green", "yellow",   "blue",   "magenta", "cyan",
    "white",   "orange", "bg",    "abg",      "bblack", "bred",    "bgreen",
    "byellow", "bblue",  "bcyan", "bmagenta", "bwhite", "borange", ""};
const wchar_t bw_ho = BW_HO;  /**< horizontal line */
const wchar_t bw_ve = BW_VE;  /**< vertical line */
const wchar_t bw_tl = BW_RTL; /**< top left corner */
const wchar_t bw_tr = BW_RTR; /**< top right corner */
const wchar_t bw_bl = BW_RBL; /**< bottom left corner */
const wchar_t bw_br = BW_RBR; /**< bottom right corner */
const wchar_t bw_lt = BW_LT;  /**< left tee */
const wchar_t bw_rt = BW_RT;  /**< right tee */
const wchar_t bw_sp = BW_SP;  /**< tee space */
double GRAY_GAMMA =
    1.2; /**< Gamma correction value for gray colors. Set in .minitrc */
double RED_GAMMA =
    1.2; /**< Gamma correction value for red colors. Set in .minitrc */
double GREEN_GAMMA =
    1.2; /**< Gamma correction value for green colors. Set in .minitrc */
double BLUE_GAMMA =
    1.2; /**< Gamma correction value for blue colors. Set in .minitrc */

/// key_cmd
/// Function key command table
/// @note This table will be used to create the chyron
/// @note The end_pos values are set in chyron_mk
/// @note The keycode values are used in get_chyron_key
/// @note If text is "", the key is not processed
/// @note The keycode values use NCurses key definitions
/// @note The table can be modified on the fly using set_fkey and unset_fkey
/// @note The table can be extended to 20 function keys if needed
/// @see set_fkey
key_cmd_tbl key_cmd[20] = {
    {"", KEY_F(0), 0},
    {"F1 Help", KEY_F(1), 0},
    {"", KEY_F(2), 0},
    {"", KEY_F(3), 0},
    {"F4 Query", KEY_F(4), 0},
    {"F5 Calculate", KEY_F(5), 0},
    {"F6 Edit", KEY_F(6), 0},
    {"", KEY_F(7), 0},
    {"", KEY_F(8), 0},
    {"F9 Cancel", KEY_F(9), 0},
    {"F10 Accept", KEY_F(10), 0},
    {"PgUp", KEY_PPAGE, 0},
    {"PgDn", KEY_NPAGE, 0},
    {"", 0x0, -1},
};

enum key_idx { F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, PgUp, PgDn, END };
WINDOW *win;
WINDOW *win_win[MAXWIN];
WINDOW *win_box[MAXWIN];
char tmp_str[MAXLEN];
char *tmp_ptr;
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
int cp_norm;
int cp_box;
int cp_reverse;
int clr_cnt = 0;
int clr_idx = 0;
int clr_pair_idx = 1;
int clr_pair_cnt = 1;
cchar_t CCC_NORM;
cchar_t CCC_BOX;
cchar_t CCC_REVERSE;
/// Global file/pipe numbers
int tty_fd, pipe_in, pipe_out;

/** @brief Initialize window attributes
    @param fg_color Foreground color index
    @param bg_color Background color index
    @param bo_color Box color index
    note This function initializes color pairs for the window
    note cp_norm and cp_box are global variables
    see get_clr_pair */
void win_init_attrs(int fg_color, int bg_color, int bo_color) {
    init_extended_pair(cp_norm, fg_color, bg_color);
    init_extended_pair(cp_box, bo_color, bg_color);
    return;
}

/** @brief Check if function key label is set
    @param k Function key index (0-19)
    @return true if set, false if not set */
bool is_set_fkey(int k) {
    if (key_cmd[k].text[0] != '\0')
        return true;
    else
        return false;
}

/** @brief Set function key label
    @param k Function key index (0-19)
    @param s Function key label
    @note This table will be used to change function keys on the fly */
void set_fkey(int k, char *s) {
    if (*s != '\0')
        ssnprintf(key_cmd[k].text, MAXLEN - 1, "F%d %s", k, s);
    else
        key_cmd[k].text[0] = '\0';
}

/** @brief Unset function key label in key_cmd table
 @param k key_cmd index
 @note function keys F0 through F10 idiomatically occupy key_cmd[0-12] */
void unset_fkey(int k) { key_cmd[k].text[0] = '\0'; }
/** @brief construct the chyron string from the key_cmd table
    @param fc Pointer to key_cmd_tbl
    @param s Pointer to chyron string
    @return Length of chyron string
    note This function also sets the end_pos values in the key_cmd_tbl
    which are used to determine which key was clicked in get_chyron_key */
int chyron_mk(key_cmd_tbl *fc, char *s) {
    int end_pos = 0;
    int i = 0;
    *s = '\0';
    while (fc[i].end_pos != -1) {
        if (fc[i].text[0] == '\0') {
            i++;
            continue;
        }
        if (end_pos == 0)
            strnz__cat(s, " ", MAXLEN - strlen(s) - 1);
        else
            strnz__cat(s, " | ", MAXLEN - strlen(s) - 1);
        strnz__cat(s, fc[i].text, MAXLEN - strlen(s) - 1);
        end_pos = strlen(s) + 1;
        fc[i].end_pos = end_pos;
        i++;
    }
    if (end_pos > 0)
        strnz__cat(s, " ", MAXLEN - strlen(s) - 1);
    return strlen(s);
}

/** @brief Get keycode from chyron mouse position
    @param fc Pointer to key_cmd_tbl
    @param x Mouse X position
    @return Keycode
    note This function uses the end_pos values set in chyron_mk
    to determine which key was clicked */
int get_chyron_key(key_cmd_tbl *fc, int x) {
    int i;
    for (i = 0; fc[i].end_pos != -1; i++)
        if (x < fc[i].end_pos)
            break;
    return fc[i].keycode;
}

/** @brief Initialize NCurses and color settings
    @param sio Pointer to SIO struct with terminal and color settings
    @return true if successful, false if error
    note This function initializes NCurses and sets up color pairs based on the
    settings in the SIO struct. It also applies gamma correction to colors.
    Use this function to initialize NCurses if you don't want NCurses to receive
   data from the stdin pipe
    1. saves stdin and stdout file descriptors in SIO
    2. opens a terminal device for NCurses screen IO
    3. replaces STDERR_FILENO with terminal file descriptor */
bool open_curses(SIO *sio) {
    char tmp_str[MAXLEN];
    char emsg0[MAXLEN];
    int rc;
    RGB frgb, brgb;

    if (ttyname_r(STDERR_FILENO, sio->tty_name, sizeof(sio->tty_name)) != 0) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strnz__cpy(emsg0, "ttyname_r failed ", MAXLEN - 1);
        strnz__cat(emsg0, tmp_str, MAXLEN - 1);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    /// save stdin and stdout file descriptors
    sio->stdin_fd = dup(STDIN_FILENO);
    sio->stdout_fd = dup(STDOUT_FILENO);
    /// open the terminal device for reading and writing
    sio->tty_fp = fopen(sio->tty_name, "r+");
    if (sio->tty_fp == NULL) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strnz__cpy(emsg0, "fopen(sio->tty_name) failed ", MAXLEN - 1);
        strnz__cat(emsg0, tmp_str, MAXLEN - 1);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    /// Attach the terminal descriptor to the STDERR_FILENO
    dup2(fileno(sio->tty_fp), STDERR_FILENO);
    /// We use SCREEN and newterm because this allows us to
    /// specify the terminal FILE
    SCREEN *screen = newterm(NULL, sio->tty_fp, sio->tty_fp);
    if (screen == NULL) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strnz__cpy(emsg0, "newterm failed ", MAXLEN - 1);
        strnz__cat(emsg0, tmp_str, MAXLEN - 1);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    set_term(screen);
    f_curses_open = true;
    if (!has_colors()) {
        destroy_curses();
        abend(-1, "Terminal color support required");
    }
    start_color();
    if (!can_change_color()) {
        destroy_curses();
        fprintf(stderr, "Terminal cannot change colors\n");
        fprintf(stderr, "Check TERM environment variable\n");
        fprintf(stderr, "Check terminfo for missing \"ccc\"\n");
        abend(-1, "fatal error");
    }
    init_clr_palette(sio);
    /// Set gamma correction values
    /// These are read from ~/.minitrc
    /// We need these values when initializing colors
    RED_GAMMA = sio->red_gamma;
    GREEN_GAMMA = sio->green_gamma;
    BLUE_GAMMA = sio->blue_gamma;
    GRAY_GAMMA = sio->gray_gamma;
    cp_norm = get_clr_pair(sio->fg_color, sio->bg_color);
    cp_reverse = get_clr_pair(sio->bg_color, sio->fg_color);
    cp_box = get_clr_pair(sio->bo_color, sio->bg_color);
    CCC_NORM = mkccc(cp_norm);
    CCC_BOX = mkccc(cp_box);
    CCC_REVERSE = mkccc(cp_reverse);
    noecho();
    cbreak();
    keypad(stdscr, true);
    idlok(stdscr, false);
    idcok(stdscr, false);
    wbkgrndset(stdscr, &CCC_NORM);
    extended_pair_content(cp_norm, &sio->fg_color, &sio->bg_color);
    extended_color_content(sio->fg_color, &frgb.r, &frgb.g, &frgb.b);
    rc = extended_color_content(sio->bg_color, &brgb.r, &brgb.g, &brgb.b);
    if (rc == ERR) {
        destroy_curses();
        abend(-1, "extended_color_content failed");
    }
    return sio;
}

/** @brief Get color pair index for foreground and background colors
    @param fg Foreground color index
    @param bg Background color index
    @return Color pair index */
int get_clr_pair(int fg, int bg) {
    int rc, i, pfg, pbg;
    for (i = 0; i < clr_pair_cnt; i++) {
        extended_pair_content(i, &pfg, &pbg);
        if (pfg == fg && pbg == bg)
            return i;
    }
    if (i >= COLOR_PAIRS) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "NCurses COLOR_PAIRS (%d) exceeded (%d)",
                  COLOR_PAIRS, i);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, NULL);
        return (EXIT_FAILURE);
    }
    if (i < COLOR_PAIRS) {
        rc = init_extended_pair(i, fg, bg);
        if (rc == ERR)
            return ERR;
    }
    if (i < COLOR_PAIRS)
        clr_pair_cnt++;
    return i;
}

/** @brief Get color index for RGB color
    @param rgb RGB color
    @return NCurses color index
    @note Curses uses 0-1000 for RGB values
    @note If the color does not exist, it is created along with a new color
   index */
int rgb_to_curses_clr(RGB *rgb) {
    int i;
    int r, g, b;
    apply_gamma(rgb);
    rgb->r = (rgb->r * 1000) / 255;
    rgb->g = (rgb->g * 1000) / 255;
    rgb->b = (rgb->b * 1000) / 255;
    for (i = 0; i < clr_cnt; i++) {
        extended_color_content(i, &r, &g, &b);
        if (rgb->r == r && rgb->g == g && rgb->b == b) {
            return i;
        }
    }
    if (i < COLORS) {
        init_extended_color(i, rgb->r, rgb->g, rgb->b);
        clr_cnt++;
        return clr_cnt - 1;
    }
    return ERR;
}

/** @brief Convert RGB color to XTerm 256 color index
    @param rgb RGB color
    @return XTerm 256 color index
    note This function converts an RGB color to the nearest XTerm 256 color
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

/** @brief Convert XTerm 256 color index to RGB color
    @param idx XTerm 256 color index
    @return RGB color
    note This function converts an XTerm 256 color index to an RGB color. It
   first checks if the index is in the standard 16 colors, then checks if it's
   in the 6x6x6 color cube, and finally checks if it's in the gray ramp. */
RGB xterm256_idx_to_rgb(int idx) {
    /// Convert XTerm 256 color index to RGB
    /// @param idx - XTerm 256 color index
    /// @return RGB struct
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

/** @brief Apply gamma correction to RGB color
    @param rgb Pointer to RGB color
    note This function modifies the RGB color in place. It applies gamma
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

/** @brief Initialize color palette based on SIO settings
    @param sio Pointer to SIO struct with color settings
    @return true if successful, false if error
    @note This function initializes the xterm256 color cube and applies any
   color overrides specified in the SIO struct. The color strings in the SIO
   struct are expected to be six-digit HTML style hex color codes (e.g.,
   "#RRGGBB"). If a color override is specified for any of the standard colors,
   it is applied using the init_hex_clr function. After processing all colors,
   the clr_cnt variable is set to 16 to indicate that the standard colors have
   been initialized. */
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
    clr_cnt = 16;
    return true;
}

/** @brief Initialize extended ncurses color from HTML style hex string
    @param idx Color index
    @param s Hex color string
    @note NCurses uses 0-1000 for RGB values, so the RGB values from the hex
   string are converted to this range before initializing the color. If the
   color index is less than 16, the RGB values are also stored in the StdColors
   array for reference.
    */
void init_hex_clr(int idx, char *s) {
    /// Create extended ncurses color from HTML style hex string
    /// @param idx Color index
    /// @param s Hex color string
    /// @note NCursesw uses 0-1000 for RGB values
    RGB rgb;
    rgb = hex_clr_str_to_rgb(s);
    apply_gamma(&rgb);
    if (idx < 16) {
        StdColors[idx].r = rgb.r;
        StdColors[idx].g = rgb.g;
        StdColors[idx].g = rgb.g;
    }
    rgb.r = (rgb.r * 1000) / 255;
    rgb.g = (rgb.g * 1000) / 255;
    rgb.b = (rgb.b * 1000) / 255;
    init_extended_color(idx, rgb.r, rgb.g, rgb.b);
}

/** @brief Convert six-digit HTML style hex color code to RGB struct
    @param s six-digit HTML style hex color code */
RGB hex_clr_str_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}

/** @brief Gracefully shut down NCurses and restore terminal settings
    @note This function should be called before exiting the program to ensure
   that the terminal is left in a usable state. It checks if NCurses was
   initialized and, if so, it clears the screen, refreshes it, and ends the
   NCurses session. It also restores the original terminal settings using
   restore_shell_tioctl and resets signal handlers to their default state with
   sig_dfl_mode. */
void destroy_curses() {
    if (f_curses_open) {
        wclear(stdscr);
        wrefresh(stdscr);
        endwin();
        f_curses_open = false;
    }
    restore_shell_tioctl();
    sig_dfl_mode();
    return;
}

/** @brief Create a cchar_t with the specified color pair index
    @param cp Color pair index
    @return cchar_t with the specified color pair index and a space character
    as the wide character */
cchar_t mkccc(int cp) {
    cchar_t cc;
    wchar_t wc = L' ';
    setcchar(&cc, &wc, WA_NORMAL, cp, NULL);
    return cc;
}

/** @brief Create a new window with optional box and title
    @param wlines Number of lines
    @param wcols Number of columns
    @param wbegy Beginning Y position
    @param wbegx Beginning X position
    @param wtitle Window title
    @param flag Window flags
    @note if flag set to W_BOX, Only create win_box. This is for View which
   uses the box window for display and doesn't need a separate win_win
    @return 0 if successful, 1 if error */
int win_new(int wlines, int wcols, int wbegy, int wbegx, char *wtitle,
            int flag) {
    int maxx;
    if (win_ptr < MAXWIN) {
        if (win_ptr > 0)
            wrefresh(win_win[win_ptr]);
        else
            wrefresh(stdscr);
        win_ptr++;
        if (wbegy != 0 || wbegx != 0 || wlines < LINES - 2 ||
            wcols < COLS - 2) {
            win_box[win_ptr] = newwin(wlines + 2, wcols + 2, wbegy, wbegx);
            if (win_box[win_ptr] == NULL) {
                win_ptr--;
                return (1);
            }
            wbkgrnd(win_box[win_ptr], &CCC_BOX);
            wbkgrndset(win_box[win_ptr], &CCC_BOX);
            if (wtitle != NULL && *wtitle != '\0') {
                cbox(win_box[win_ptr]);
                mvwaddnwstr(win_box[win_ptr], 0, 1, &bw_rt, 1);
                mvwaddnwstr(win_box[win_ptr], 0, 2, &bw_sp, 1);
                mvwaddstr(win_box[win_ptr], 0, 3, wtitle);
                maxx = getmaxx(win_box[win_ptr]);
                int s = strlen(wtitle);
                if ((s + 3) < maxx)
                    mvwaddch(win_box[win_ptr], 0, (s + 3), ' ');
                if ((s + 4) < maxx)
                    mvwaddnwstr(win_box[win_ptr], 0, (s + 4), &bw_lt, 1);
            }
            wnoutrefresh(win_box[win_ptr]);
            wbegy += 1;
            wbegx += 1;
        } else {
            win_box[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            if (win_box[win_ptr] == NULL) {
                win_ptr--;
                return (1);
            }
            wbkgrnd(win_box[win_ptr], &CCC_BOX);
            wbkgrndset(win_box[win_ptr], &CCC_BOX);
        }
        if (!(flag & W_BOX)) {
            win_win[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
            if (win_win[win_ptr] == NULL) {
                delwin(win_box[win_ptr]);
                win_ptr--;
                return (1);
            }
            wbkgrnd(win_win[win_ptr], &CCC_NORM);
            wbkgrndset(win_win[win_ptr], &CCC_NORM);
            keypad(win_win[win_ptr], TRUE);
            idlok(win_win[win_ptr], false);
            idcok(win_win[win_ptr], false);
        }
    }
    return (0);
}

/** @brief Resize the current window and its box, and update the title
    @param wlines Number of lines
    @param wcols Number of columns
    @param title Window title
    @note This function resizes the current window and its associated box window
   to the specified number of lines and columns. It also updates the title of
   the box window if a title is provided. After resizing, it refreshes the
   windows to apply the changes. */
void win_resize(int wlines, int wcols, char *title) {
    int maxx;
    wrefresh(stdscr);
    wresize(win_box[win_ptr], wlines + 2, wcols + 2);
    wbkgrnd(win_box[win_ptr], &CCC_BOX);
    wbkgrndset(win_box[win_ptr], &CCC_BOX);
    cbox(win_box[win_ptr]);
    if (title != NULL && *title != '\0') {
        wmove(win_box[win_ptr], 0, 1);
        waddnstr(win_box[win_ptr], (const char *)&bw_rt, 1);
        wmove(win_box[win_ptr], 0, 2);
        waddnstr(win_box[win_ptr], (const char *)&bw_sp, 1);
        mvwaddnwstr(win_box[win_ptr], 0, 1, &bw_rt, 1);
        mvwaddnwstr(win_box[win_ptr], 0, 2, &bw_sp, 1);
        mvwaddstr(win_box[win_ptr], 0, 3, title);
        maxx = getmaxx(win_box[win_ptr]);
        int s = strlen(title);
        if ((s + 3) < maxx)
            mvwaddch(win_box[win_ptr], 0, (s + 3), ' ');
        if ((s + 4) < maxx)
            mvwaddnwstr(win_box[win_ptr], 0, (s + 4), &bw_lt, 1);
    }
    wnoutrefresh(win_box[win_ptr]);
    wresize(win_win[win_ptr], wlines, wcols);
    init_extended_pair(cp_norm, sio->fg_color, sio->bg_color);
    wbkgrnd(win_win[win_ptr], &CCC_NORM);
    wbkgrndset(win_win[win_ptr], &CCC_NORM);
    wsetscrreg(win_win[win_ptr], 0, wlines - 1);
    keypad(win_win[win_ptr], TRUE);
    idlok(win_win[win_ptr], false);
    idcok(win_win[win_ptr], false);
}
/** @brief Redraw the specified window
    @param win Pointer to the window to redraw
    note This function erases the contents of the specified window and then
   refreshes it to update the display. Use this function when you need to clear
   and redraw a window, such as after resizing or when updating its contents. */
void win_redraw(WINDOW *win) {
    werase(win);
    wnoutrefresh(win);
}

/** @brief Delete the current window and its associated box window
    @return NULL
    note This function deletes the current window and its associated box window,
   if they exist. It also refreshes the remaining windows to ensure the display
   is updated correctly. After calling this function, the global win_ptr
   variable is decremented to point to the previous window in the stack. */
WINDOW *win_del() {
    int i;

    if (win_ptr > 0) {
        wclear(win_win[win_ptr]);
        touchwin(win_win[win_ptr]);
        wnoutrefresh(win_win[win_ptr]);
        delwin(win_win[win_ptr]);

        wclear(win_box[win_ptr]);
        touchwin(win_box[win_ptr]);
        wnoutrefresh(win_box[win_ptr]);
        delwin(win_box[win_ptr]);

        touchwin(stdscr);
        wnoutrefresh(stdscr);
        for (i = 0; i < win_ptr; i++) {
            touchwin(win_box[i]);
            wnoutrefresh(win_box[i]);
            touchwin(win_win[i]);
            wnoutrefresh(win_win[i]);
        }
        win_ptr--;
    }
    return (0);
}

/** @brief Restore all windows after a screen resize
    note This function is used to restore the display of all windows after a
   screen resize event. It clears the standard screen and then iterates through
   all existing windows, touching and refreshing them to ensure they are
   redrawn correctly on the resized screen. Use this function in response to a
   SIGWINCH signal to handle terminal resizing gracefully. */
void restore_wins() {
    int i;

    wclear(stdscr);
    touchwin(stdscr);
    wnoutrefresh(stdscr);
    for (i = 0; i <= win_ptr; i++) {
        touchwin(win_box[i]);
        wnoutrefresh(win_box[i]);
        touchwin(win_win[i]);
        wnoutrefresh(win_win[i]);
    }
}

/** @brief Draw a box around the specified window
    @param box Pointer to the window to draw the box around
    note This function uses NCurses functions to draw a box around the specified
   window. It adds the appropriate characters for the corners and edges of the
   box based on the current character set. Use this function when you want to
   visually separate a window from the rest of the screen with a border. */
void cbox(WINDOW *box) {
    int x, y;
    int maxx;
    int maxy;

    maxx = getmaxx(box);
    maxx--;
    mvwaddnwstr(box, 0, 0, &bw_tl, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(box, &bw_ho, 1);
    waddnwstr(box, &bw_tr, 1);
    maxy = getmaxy(box);
    maxy--;
    for (y = 1; y < maxy; y++) {
        mvwaddnwstr(box, y, 0, &bw_ve, 1);
        mvwaddnwstr(box, y, maxx, &bw_ve, 1);
    }
    mvwaddnwstr(box, maxy, 0, &bw_bl, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(box, &bw_ho, 1);
    waddnwstr(box, &bw_br, 1);
}
/** @brief Display an error message window or print to stderr
    @param em0 First error message line
    @param em1 Second error message line
    @param em2 Third error message line
    @param em3 Fourth error message line
    @return Key code of user command */
int display_error(char *em0, char *em1, char *em2, char *em3) {
    char title[MAXLEN];
    WINDOW *error_win;
    int line, pos, em_l, em0_l, em1_l, em2_l, em3_l, cmd_l;
    char cmd[] = " F1 Help | F9 Cancel | F10 Continue ";

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n%s\n%s\n%s\n\n", em0, em1, em2, em3);
        return (1);
    }
    em_l = 0;
    cmd_l = strnz(cmd, COLS - 4);
    em0_l = strnz(em0, COLS - 4);
    em1_l = strnz(em1, COLS - 4);
    em2_l = strnz(em2, COLS - 4);
    em3_l = strnz(em1, COLS - 4);
    if (em0_l > em_l)
        em_l = em0_l;
    if (em1_l > em_l)
        em_l = em1_l;
    if (em2_l > em_l)
        em_l = em2_l;
    if (em3_l > em_l)
        em_l = em3_l;
    if (em_l < cmd_l)
        em_l = cmd_l;
    if (em_l > (COLS - 4))
        em_l = COLS - 4;

    pos = ((COLS - em_l) - 4) / 2;
    line = (LINES - 6) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (win_new(5, em_l + 2, line, pos, title, 0)) {
        ssnprintf(title, MAXLEN - 1, "win_new(%d, %d, %d, %d, %s, %b) failed",
                  5, em_l + 2, line, pos, title, 0);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, em0);
    mvwaddstr(error_win, 1, 1, em1);
    mvwaddstr(error_win, 2, 1, em2);
    mvwaddstr(error_win, 3, 1, em3);
    wattron(error_win, WA_REVERSE);
    mvwaddstr(error_win, 4, 1, cmd);
    wattroff(error_win, WA_REVERSE);
    wmove(error_win, 4, cmd_l + 1);
    wrefresh(error_win);
    while (1) {
        cmd_key = xwgetch(error_win);
        switch (cmd_key) {
        case 'n':
        case 'N':
        case 'q':
        case 'Q':
        case 'x':
        case 'X':
        case 'y':
        case 'Y':
        case KEY_F(1):
        case KEY_F(9):
        case KEY_F(10):
            win_del();
            return (cmd_key);
        default:
            continue;
        }
    }
    win_del();
    return (cmd_key);
}

/** @brief Display a simple error message window or print to stderr
    @param emsg_str Error message string
    @return Key code of user command */
int Perror(char *emsg_str) {
    char emsg[80];
    int emsg_max_len = 80;
    unsigned cmd_key;
    WINDOW *error_win;
    int len, line, pos;
    char title[MAXLEN];

    len = strnz__cpy(emsg, emsg_str, emsg_max_len - 1);
    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", emsg);
        return (1);
    }
    pos = (COLS - len - 4) / 2;
    line = (LINES - 4) / 2;
    if (len < 39)
        len = 39;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (win_new(2, len + 2, line, pos, title, 0)) {
        ssnprintf(title, MAXLEN - 1, "win_new(%d, %d, %d, %d, %s, %b) failed",
                  4, line, line, pos, title, 0);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, emsg);
    wattron(error_win, WA_REVERSE);
    mvwaddstr(error_win, 1, 0, " F9 Cancel | Any other key to continue ");
    wattroff(error_win, WA_REVERSE);
    wmove(error_win, 1, 27);
    wrefresh(error_win);
    cmd_key = xwgetch(error_win);
    win_del();
    return (cmd_key);
}

/** @brief For lines shorter than their display area, fill the rest with spaces
    @param w Pointer to window
    @param y Y coordinate
    @param x X coordinate
    @param s String to display
    @param l Length of display area */
void mvwaddstr_fill(WINDOW *w, int y, int x, char *s, int l) {
    char *d, *e;
    char tmp_str[MAXLEN];

    d = tmp_str;
    if (l > MAXLEN - 1)
        l = MAXLEN - 1;
    e = tmp_str + l;
    while (d < e)
        if (*s == '\0' || *s == '\n')
            *d++ = ' ';
        else
            *d++ = *s++;
    *d++ = '\0';
    mvwaddstr(w, y, x, tmp_str);
}

/** @brief Get color index from color name
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

/** @brief list colors to stderr
    @note only lists the first 16, since that's how many we let the
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

/** @brief Display argument vectors and error message
    @param emsg Error message
    @param argv Argument vector */
void display_argv_error_msg(char *emsg, char **argv) {
    int argc = 0;
    fprintf(stderr, "\r\n");
    while (*argv != NULL && **argv != '\0')
        fprintf(stderr, "argv[%d] - %s\r\n", argc++, *argv++);
    fprintf(stderr, "%s\r\n", emsg);
    fprintf(stderr, "%s", "Press any key to continue");
    wrefresh(stdscr);
    xwgetch(stdscr);
}

/** @brief Display error message and wait for key press
    @param ec Error code
    @param s Error message */
int nf_error(int ec, char *s) {
    fprintf(stderr, "ERROR: %s code: %d\n", s, ec);
    fprintf(stderr, "Press a key to continue");
    di_getch();
    fprintf(stderr, "\n");
    return ec;
}

/** @brief Program terminated by user */
void user_end() {
    destroy_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    fprintf(stderr, "Normal program exit");
    fprintf(stderr, "\n");
    exit(EXIT_SUCCESS);
}

/** @brief Abnormal program termination
    @param ec Exit code
    @param s Error message */
void abend(int ec, char *s) {
    destroy_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    fprintf(stderr, "\n\nABEND: %s (code: %d)\n", s, ec);
    exit(EXIT_FAILURE);
}

/** @brief Wrapper for wgetch that handles signals
    @param win Pointer to window
    @return Key code or ERR if interrupted by signal */
int xwgetch(WINDOW *win) {
    int c;
    halfdelay(1);
    do {
        c = wgetch(win);
        if (sig_received != 0) {
            if (handle_signal(sig_received))
                c = display_error(em0, em1, em2, NULL);
            if (c == 'q' || c == KEY_F(9))
                exit(EXIT_FAILURE);
            continue;
        }
    } while (c == ERR);
    return c;
}
