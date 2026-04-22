/** @file dwin.c
    @brief Window support for C-Menu - EXPERIMENTAL
    @note This file contains functions for managing NCurses windows and color
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

#include <cm.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>
#include <wchar.h>
#define NC true

bool open_curses(SIO *);
void destroy_curses();
int box_new(int, int, int, int, char *, bool);
int box2_new(int, int, int, int, char *, bool);
int win_new(int, int, int, int);
void win_redraw(WINDOW *);
void win_resize(int, int, char *);
WINDOW *win_del();
void restore_wins();
void cbox(WINDOW *);
void cbox2(WINDOW *);
void win_init_attrs();
int Perror(char *);
void mvwaddstr_fill(WINDOW *, int, int, char *, int);
void abend(int, char *);
int nf_error(int, char *);
int click_y;
int click_x;
WINDOW *mouse_win;
void list_colors();
int clr_name_to_idx(char *);
void init_hex_clr(int, char *);
int rgb_clr_to_cube(int);
RGB hex_clr_str_to_rgb(char *);
RGB xterm256_idx_to_rgb(int);
int rgb_to_curses_clr(RGB *);
int rgb_to_xterm256_idx(RGB *);
void apply_gamma(RGB *);

Chyron *new_chyron();
void set_chyron_key(Chyron *, int, char *, int);
void set_chyron_key_cp(Chyron *, int, char *, int, int);
bool is_set_chyron_key(Chyron *, int);
void unset_chyron_key(Chyron *, int);
void compile_chyron(Chyron *);
int get_chyron_key(Chyron *, int);
Chyron *destroy_chyron(Chyron *chyron);
int mb_to_cc(cchar_t *, char *, attr_t, int, int *, int);

Chyron *wait_mk_chyron();
WINDOW *wait_mk_win(Chyron *, char *);
int wait_continue(WINDOW *, Chyron *, int);
bool wait_destroy(Chyron *);

int xwgetch(WINDOW *, Chyron *, int);

bool init_clr_palette(SIO *);
cchar_t mkccc(int, attr_t, char *);
SCREEN *screen;

SIO *sio; /**< Global pointer to SIO struct for terminal and color settings */

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
const wchar_t bw_la = BW_LA;   /**< right arrow */
const wchar_t bw_ua = BW_UA;   /**< right arrow */
const wchar_t bw_da = BW_DA;   /**< right arrow */
const wchar_t bw_ran = BW_RAN; /**< right angle */
const wchar_t bw_lan = BW_LAN; /**< right angle */

double GRAY_GAMMA =
    1.2; /**< Gamma correction value for gray colors. Set in .minitrc */
double RED_GAMMA =
    1.2; /**< Gamma correction value for red colors. Set in .minitrc */
double GREEN_GAMMA =
    1.2; /**< Gamma correction value for green colors. Set in .minitrc */
double BLUE_GAMMA =
    1.2; /**< Gamma correction value for blue colors. Set in .minitrc */

WINDOW *win;
WINDOW *win_win[MAXWIN];
WINDOW *win_win2[MAXWIN];
WINDOW *win_box[MAXWIN];
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
int cp_win;
int cp_box;
int cp_reverse;
int cp_reverse_highlight;
int cp_ln;
int clr_cnt = 0;
int clr_pair_idx = 1;
int clr_pair_cnt = 1;
cchar_t CCC_NORM;
cchar_t CCC_WIN;
cchar_t CCC_BG;
cchar_t CCC_BOX;
cchar_t CCC_REVERSE;
cchar_t CCC_REVERSE_HIGHLIGHT;
cchar_t CCC_LN;
/** Global file/pipe numbers */
int tty_fd, pipe_in, pipe_out;
FILE *ncurses_fp;

/** @brief Initialize window attributes
    @ingroup window_support
    @note This function initializes color pairs for the window
    @note cp_norm, cp_win, and cp_box are global variables
    @see get_clr_pair
 */
void win_init_attrs() { return; }

/** @defgroup Chyron Chyron Management
    @brief Create and manage the Chyron
 */

/** @brief Create and initialize Chyron structure
    @ingroup Chyron
    @return pointer to new Chyron structure
    @details This function allocates memory for a new Chyron structure and
   initializes the key pointers. Each key pointer is allocated memory for a
   ChyronKey structure. The Chyron structure is used to manage function key
   labels and their associated keycodes for mouse click handling in the chyron
   area of the interface.
    @note The use of calloc ensures that the allocated memory is initialized to
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
    for (int i = 0; i < CHYRON_KEYS; i++)
        chyron->key[i] = (ChyronKey *)calloc(1, sizeof(ChyronKey));
    return chyron;
}
/** @brief Destroy Chyron structure
    @ingroup Chyron
    @param chyron pointer to Chyron structure
    @return nullptr
 */
Chyron *destroy_chyron(Chyron *chyron) {
    int i;

    if (!chyron)
        return nullptr;
    for (i = 0; i < CHYRON_KEYS; i++) {
        if (chyron->key[i])
            free(chyron->key[i]);
        chyron->key[i] = nullptr;
    }
    free(chyron);
    chyron = nullptr;
    return chyron;
}
/** @brief Check if function key label is set
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
/** @brief Set chyron key
    @ingroup Chyron
    @param chyron structure
    @param k chyron key index (0-19)
    @param s chyron key label
    @param kc chyron key code
    @param cp color pair index for the key label
    @details This function sets the label and keycode for a chyron key. The
   label is copied into the chyron structure, and the keycode is stored for
   later retrieval when theire is a mouse click. The compile_chyron function
   uses the keycode values to determine which key was clicked based on the mouse
   X position. If the label string is empty, the key is unset by setting the
   first character of the text to '\0'.
*/
void set_chyron_key_cp(Chyron *chyron, int k, char *s, int kc, int cp) {
    if (*s != '\0')
        ssnprintf(chyron->key[k]->text, CHYRON_KEY_MAXLEN - 1, "%s", s);
    else
        chyron->key[k]->text[0] = '\0';
    chyron->key[k]->keycode = kc;
    chyron->key[k]->cp = cp;
}
void set_chyron_key(Chyron *chyron, int k, char *s, int kc) {
    if (*s != '\0')
        ssnprintf(chyron->key[k]->text, CHYRON_KEY_MAXLEN - 1, "%s", s);
    else
        chyron->key[k]->text[0] = '\0';
    chyron->key[k]->keycode = kc;
    chyron->key[k]->cp = cp_reverse;
}
/** @brief Unset chyron key
    @ingroup Chyron
    @param chyron structure
    @param k chyron_key index
*/
void unset_chyron_key(Chyron *chyron, int k) { chyron->key[k]->text[0] = '\0'; }
/** @brief construct the chyron string from the chyron structure
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
    int cp = cp_reverse;
    cchar_t *cx;
    while (k < CHYRON_KEYS) {
        if (chyron->key[k]->text[0] == '\0') {
            k++;
            continue;
        }
        if (end_pos == 0) {
            cx = chyron->cmplx_buf;
            mb_to_cc(cx, " ", WA_NORMAL, cp_reverse, &pos, MAXLEN - 1);
        } else {
            mb_to_cc(chyron->cmplx_buf, "|", WA_NORMAL, cp_reverse, &pos,
                     MAXLEN - 1);
        }
        cx = chyron->cmplx_buf;
        cp = chyron->key[k]->cp;
        mb_to_cc(cx, chyron->key[k]->text, WA_NORMAL, cp, &pos, MAXLEN - 1);
        end_pos = pos;
        chyron->l = end_pos;
        chyron->key[k]->end_pos = end_pos;
        k++;
    }
    mb_to_cc(chyron->cmplx_buf, " ", WA_NORMAL, cp, &pos, MAXLEN - 1);
    chyron->l = end_pos;
}
void display_chyron(WINDOW *win, Chyron *chyron, int line, int col) {
    wmove(win, line, 0);
    wclrtoeol(win);
    wmove(win, line, 0);
    wadd_wchstr(win, chyron->cmplx_buf);
    wmove(win, line, col);
    return;
}
/** @brief Convert multibyte string to complex character array
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
int mb_to_cc(cchar_t *cmplx_buf, char *str, attr_t attr, int cpx, int *pos,
             int maxlen) {
    int i = 0, len = 0;
    const char *s;
    cchar_t cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    attr = WA_NORMAL;
    if (*pos >= maxlen - 1)
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
        if (*pos >= maxlen - 1)
            break;
        if (setcchar(&cc, wstr, attr, cpx, nullptr) != ERR) {
            if (len > 0 && (*pos + len) < MAXLEN - 1)
                cmplx_buf[(*pos)++] = cc;
        }
        i += len;
    }
    wstr[0] = L'\0';
    wstr[1] = L'\0';
    setcchar(&cc, wstr, attr, cpx, nullptr);
    cmplx_buf[*pos] = cc;
    return *pos;
}
/** @brief Get keycode from chyron
    @ingroup Chyron
    @param chyron structure
    @param x Mouse X position
    @return Keycode
    @note This function uses the end_pos values set in compile_chyron
    to determine which key was clicked
    @note The chyron functions provide xwgetch() with a mechanism to translate
    mouse click positions into key codes based on the labels set in the chyron
   structure. When a mouse click occurs, xwgetch() can call get_chyron_key()
   with the X position of the click to determine which function key was clicked,
   allowing for dynamic and customizable function key behavior in the chyron
   area of the interface.
*/
int get_chyron_key(Chyron *chyron, int x) {
    int i;
    for (i = 0; chyron->key[i]->end_pos != -1; i++)
        if (x < chyron->key[i]->end_pos)
            break;
    return chyron->key[i]->keycode;
}
/** @brief Initialize NCurses and color settings
    @ingroup window_support
    @param sio Pointer to SIO struct with terminal and color settings
    @return true if successful, false if error
    note This function initializes NCurses and sets up color pairs based on the
    settings in the SIO struct. It also applies gamma correction to colors.
    Use this function to initialize NCurses if you don't want NCurses to receive
   data from the stdin pipe
    @code
    1. saves stdin and stdout file descriptors in SIO
    2. opens a terminal device for NCurses screen IO
    3. replaces STDERR_FILENO with terminal file descriptor
    @endcode */

bool open_curses(SIO *sio) {
    char tmp_str[MAXLEN];
    char emsg0[MAXLEN];

    if (ttyname_r(STDERR_FILENO, sio->tty_name, sizeof(sio->tty_name)) != 0) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strnz__cpy(emsg0, "ttyname_r failed ", MAXLEN - 1);
        strnz__cat(emsg0, tmp_str, MAXLEN - 1);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    /** open the terminal device for reading and writing */
    ncurses_fp = fopen(sio->tty_name, "r+");
    if (ncurses_fp == nullptr) {
        strerror_r(errno, tmp_str, MAXLEN - 1);
        strnz__cpy(emsg0, "fopen(sio->tty_name) failed ", MAXLEN - 1);
        strnz__cat(emsg0, tmp_str, MAXLEN - 1);
        fprintf(stderr, "%s\n", tmp_str);
        exit(0);
    }
    /** We use SCREEN and newterm because this allows us to */
    /** specify the terminal FILE */
    screen = newterm(nullptr, ncurses_fp, ncurses_fp);
    if (screen == nullptr) {
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
    /** Set gamma correction values */
    /** These are read from ~/.minitrc */
    /** We need these values when initializing colors */
    RED_GAMMA = sio->red_gamma;
    GREEN_GAMMA = sio->green_gamma;
    BLUE_GAMMA = sio->blue_gamma;
    GRAY_GAMMA = sio->gray_gamma;

    cp_win = get_clr_pair(CLR_FG, CLR_BG);
    cp_norm = get_clr_pair(CLR_WHITE, CLR_BLACK);
    cp_reverse = get_clr_pair(CLR_BLACK, CLR_WHITE);
    cp_reverse_highlight = get_clr_pair(CLR_BLACK, CLR_YELLOW);
    cp_box = get_clr_pair(CLR_BO, CLR_BG);
    cp_ln = get_clr_pair(CLR_LN, CLR_LN_BG);

    CCC_NORM = mkccc(cp_norm, WA_NORMAL, " ");
    CCC_WIN = mkccc(cp_win, WA_NORMAL, " ");
    CCC_REVERSE = mkccc(cp_reverse, WA_NORMAL, " ");
    CCC_REVERSE_HIGHLIGHT = mkccc(cp_reverse_highlight, WA_NORMAL, " ");
    CCC_BOX = mkccc(cp_box, WA_NORMAL, " ");
    CCC_LN = mkccc(cp_ln, WA_NORMAL, " ");
    noecho();
    keypad(stdscr, true);
    idlok(stdscr, false);
    idcok(stdscr, false);
    wbkgrnd(stdscr, &CCC_NORM);
    wbkgrndset(stdscr, &CCC_NORM);
#ifdef DEBUG_IMMEDOK
    immedok(stdscr, true);
#endif
    win_ptr = -1;
    return sio;
}
/** @defgroup color_management Color Management
    @brief Conversion of Color Data Types and Management of Colors and Color
   Pairs
 */
/** @brief Get color pair index for foreground and background colors
    @ingroup color_management
    @param fg Foreground color index
    @param bg Background color index
    @return Color pair index */
int get_clr_pair(int fg, int bg) {
    int rc, i, pfg, pbg;
    for (i = 1; i < clr_pair_cnt; i++) {
        extended_pair_content(i, &pfg, &pbg);
        if (pfg == fg && pbg == bg)
            return i;
    }
    if (i >= COLOR_PAIRS) {
        ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__, __LINE__ - 1);
        ssnprintf(em1, MAXLEN - 1, "NCurses COLOR_PAIRS (%d) exceeded (%d)",
                  COLOR_PAIRS, i);
        strerror_r(errno, em2, MAXLEN);
        display_error(em0, em1, em2, nullptr);
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
    @ingroup color_management
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
    @ingroup color_management
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
    @ingroup color_management
    @param idx XTerm 256 color index
    @return RGB color
    note This function converts an XTerm 256 color index to an RGB color. It
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

/** @brief Apply gamma correction to RGB color
    @ingroup color_management
    @param rgb Pointer to RGB color
    @note This function modifies the RGB color in place. It applies gamma
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
    @ingroup color_management
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
    if (sio->borange[0])
        init_hex_clr(CLR_BORANGE, sio->borange);
    if (sio->fg_clr_x[0])
        init_hex_clr(CLR_FG, sio->fg_clr_x);
    if (sio->bg_clr_x[0])
        init_hex_clr(CLR_BG, sio->bg_clr_x);
    if (sio->bo_clr_x[0])
        init_hex_clr(CLR_BO, sio->bo_clr_x);
    if (sio->ln_clr_x[0])
        init_hex_clr(CLR_LN, sio->ln_clr_x);
    if (sio->ln_bg_clr_x[0])
        init_hex_clr(CLR_LN_BG, sio->ln_bg_clr_x);
    clr_cnt = CLR_NCOLORS;
    return true;
}
/** @brief Initialize extended ncurses color from HTML style hex string
    @ingroup color_management
    @param idx Color index
    @param s Hex color string
    @note NCurses uses 0-1000 for RGB values, so the RGB values from the hex
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
/** @brief Convert six-digit HTML style hex color code to RGB struct
    @ingroup color_management
    @param s six-digit HTML style hex color code */
RGB hex_clr_str_to_rgb(char *s) {
    RGB rgb;
    sscanf(s, "#%02x%02x%02x", &rgb.r, &rgb.g, &rgb.b);
    return rgb;
}
/** @brief Gracefully shut down NCurses and restore terminal settings
    @ingroup window_support
    @note This function should be called before exiting the program to ensure
   that the terminal is left in a usable state. It checks if NCurses was
   initialized and, if so, it erases the screen, refreshes it, and ends the
   NCurses session. It also restores the original terminal settings using
   restore_shell_tioctl and resets signal handlers to their default state with
   sig_dfl_mode. */
void destroy_curses() {
    if (!f_curses_open)
        return;
    while (win_ptr > 0) {
        if (win_win[win_ptr])
            delwin(win_win[win_ptr]);
        if (win_win2[win_ptr])
            delwin(win_win2[win_ptr]);
        if (win_box[win_ptr])
            delwin(win_box[win_ptr]);
        win_win[win_ptr] = nullptr;
        win_win2[win_ptr] = nullptr;
        win_box[win_ptr] = nullptr;
        win_ptr--;
    }
    werase(stdscr);
    wrefresh(stdscr);
    endwin();
    delscreen(screen);
    // screen = nullptr;
    fclose(ncurses_fp);
    f_curses_open = false;
    restore_shell_tioctl();
    sig_dfl_mode();
    return;
}
/** @brief Create a cchar_t with the specified color pair index
    @ingroup color_management
    @param cp Color pair index
    @return cchar_t with the specified color pair index and a space character
    as the wide character */
cchar_t mkccc(int cp, attr_t attr, char *s) {
    cchar_t cc = {0};
    wchar_t wstr[2] = {L'\0', L'\0'};
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    mbrtowc(wstr, s, MB_CUR_MAX, &mbstate);
    setcchar(&cc, wstr, attr, cp, nullptr);
    return cc;
}

/** @brief Create a new window with optional box and title
    @ingroup window_support
    @param wlines Number of lines
    @param wcols Number of columns
    @param wbegy Beginning Y position
    @param wbegx Beginning X position
    @param wtitle Window title
    @param win_pair If true, creates a pair of windows (box and inner window)
    @return 0 if successful, 1 if error */
int box2_new(int wlines, int wcols, int wbegy, int wbegx, char *wtitle,
             bool win_pair) {
    int maxx;
    if (win_ptr >= MAXWIN) {
        ssnprintf(em0, MAXLEN - 1, "Maximum number of windows (%d) exceeded");
        abend(-1, em0);
    }
    win_ptr++;
    wlines = min(wlines, LINES - 2);
    wcols = min(wcols, COLS - 2);
    win_box[win_ptr] = newwin(wlines + 4, wcols + 2, wbegy, wbegx);
    if (win_box[win_ptr] == nullptr) {
        win_ptr--;
        return 1;
    }
#ifdef DEBUG_IMMEDOK
    immedok(win_box[win_ptr], true);
#endif
    wbkgrnd(win_box[win_ptr], &CCC_BOX);
    wbkgrndset(win_box[win_ptr], &CCC_BOX);
    cbox2(win_box[win_ptr]);
    mvwaddnwstr(win_box[win_ptr], 0, 1, &bw_rt, 1);
    mvwaddnwstr(win_box[win_ptr], 0, 2, &bw_sp, 1);
    mvwaddstr(win_box[win_ptr], 0, 3, wtitle);
    maxx = getmaxx(win_box[win_ptr]);
    int s = strlen(wtitle);
    if ((s + 3) < maxx)
        mvwaddch(win_box[win_ptr], 0, (s + 3), ' ');
    if ((s + 4) < maxx)
        mvwaddnwstr(win_box[win_ptr], 0, (s + 4), &bw_lt, 1);
    wnoutrefresh(win_box[win_ptr]);
    win_win[win_ptr] = nullptr;
    win_win2[win_ptr] = nullptr;
    if (win_pair) {
        win_new(wlines - 1, wcols, wbegy, wbegx);
        win2_new(2, wcols, wbegy + wlines, wbegx);
    }
    return 0;
}
/** @brief Create a new window with optional box and title
    @ingroup window_support
    @param wlines Number of lines
    @param wcols Number of columns
    @param wbegy Beginning Y position
    @param wbegx Beginning X position
    @param wtitle Window title
    @param win_pair If true, creates a pair of windows (box and inner window)
    @return 0 if successful, 1 if error */
int box_new(int wlines, int wcols, int wbegy, int wbegx, char *wtitle,
            bool win_pair) {
    int maxx;
    if (win_ptr >= MAXWIN) {
        ssnprintf(em0, MAXLEN - 1, "Maximum number of windows (%d) exceeded");
        abend(-1, em0);
    }
    win_ptr++;
    wlines = min(wlines, LINES - 2);
    wcols = min(wcols, COLS - 2);
    win_box[win_ptr] = newwin(wlines + 2, wcols + 2, wbegy, wbegx);
    if (win_box[win_ptr] == nullptr) {
        win_ptr--;
        return 1;
    }
#ifdef DEBUG_IMMEDOK
    immedok(win_box[win_ptr], true);
#endif
    wbkgrnd(win_box[win_ptr], &CCC_BOX);
    wbkgrndset(win_box[win_ptr], &CCC_BOX);
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
    wnoutrefresh(win_box[win_ptr]);
    win_win[win_ptr] = nullptr;
    if (win_pair)
        win_new(wlines, wcols, wbegy, wbegx);
    return 0;
}
/** @brief Create a new window with specified dimensions and position
    @ingroup window_support
    @param wlines Number of lines
    @param wcols Number of columns
    @param wbegy Beginning Y position
    @param wbegx Beginning X position
    @return 0 if successful, 1 if error */
int win_new(int wlines, int wcols, int wbegy, int wbegx) {
    wbegy += 1;
    wbegx += 1;
    win_win[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
    if (win_win[win_ptr] == nullptr) {
        delwin(win_box[win_ptr]);
        return 1;
    }
#ifdef DEBUG_IMMEDOK
    immedok(win_win[win_ptr], true);
#endif
    wbkgrnd(win_win[win_ptr], &CCC_WIN);
    wbkgrndset(win_win[win_ptr], &CCC_WIN);
    keypad(win_win[win_ptr], true);
    idlok(win_win[win_ptr], false);
    idcok(win_win[win_ptr], false);
    scrollok(win_win[win_ptr], true);
    return 0;
}
/** @brief Create a new window with specified dimensions and position
    @ingroup window_support
    @param wlines Number of lines
    @param wcols Number of columns
    @param wbegy Beginning Y position
    @param wbegx Beginning X position
    @return 0 if successful, 1 if error */
int win2_new(int wlines, int wcols, int wbegy, int wbegx) {
    wbegy += 1;
    wbegx += 1;
    win_win2[win_ptr] = newwin(wlines, wcols, wbegy, wbegx);
    if (win_win2[win_ptr] == nullptr) {
        delwin(win_box[win_ptr]);
        return 1;
    }
#ifdef DEBUG_IMMEDOK
    immedok(win_win2[win_ptr], true);
#endif
    wbkgrnd(win_win2[win_ptr], &CCC_WIN);
    wbkgrndset(win_win2[win_ptr], &CCC_WIN);
    keypad(win_win2[win_ptr], true);
    idlok(win_win2[win_ptr], false);
    idcok(win_win2[win_ptr], false);
    return 0;
}
/** @brief Resize the current window and its box, and update the title
    @ingroup window_support
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
    if (title != nullptr && *title != '\0') {
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
    wbkgrnd(win_win[win_ptr], &CCC_WIN);
    wbkgrndset(win_win[win_ptr], &CCC_WIN);
    wsetscrreg(win_win[win_ptr], 0, wlines - 1);
    keypad(win_win[win_ptr], TRUE);
    idlok(win_win[win_ptr], false);
    idcok(win_win[win_ptr], false);
    scrollok(win_win[win_ptr], true);
#ifdef DEBUG_IMMEDOK
    immedok(win_win[win_ptr], true);
#endif
}
/** @brief Redraw the specified window
    @ingroup window_support
    @param win Pointer to the window to redraw
    @note This function erases the contents of the specified window and then
   refreshes it to update the display. Use this function when you need to clear
   and redraw a window, such as after resizing or when updating its contents. */
void win_redraw(WINDOW *win) {
    werase(win);
    wnoutrefresh(win);
}
/** @brief Delete the current window and its associated box window
    @ingroup window_support
    @return nullptr
    @note This function deletes the current window and its associated box
   window, if they exist. It also refreshes the remaining windows to ensure the
   display is updated correctly. After calling this function, the global win_ptr
   variable is decremented to point to the previous window in the stack. */
WINDOW *win_del() {
    int i;
    curs_set(0);
    if (win_ptr >= 0) {
        if (win_win[win_ptr] != nullptr) {
            touchwin(win_win[win_ptr]);
            wbkgrnd(win_win[win_ptr], &CCC_NORM);
            wbkgrndset(win_win[win_ptr], &CCC_NORM);
            werase(win_win[win_ptr]);
            wnoutrefresh(win_win[win_ptr]);
            delwin(win_win[win_ptr]);
        }
        if (win_box[win_ptr] != nullptr) {
            touchwin(win_box[win_ptr]);
            wbkgrnd(win_box[win_ptr], &CCC_NORM);
            wbkgrndset(win_box[win_ptr], &CCC_NORM);
            werase(win_box[win_ptr]);
            wnoutrefresh(win_box[win_ptr]);
            delwin(win_box[win_ptr]);
        }
        for (i = 0; i < win_ptr; i++) {
            if (win_box[i] != nullptr) {
                touchwin(win_box[i]);
                wnoutrefresh(win_box[i]);
            }
            if (win_win[i] == nullptr)
                continue;
            touchwin(win_win[i]);
            wnoutrefresh(win_win[i]);
        }
        win_ptr--;
    }
    return 0;
}
/** @brief Restore all windows after a screen resize
    @ingroup window_support
    @note This function is used to restore the display of all windows after a
   screen resize event. It clears the standard screen and then iterates through
   all existing windows, touching and refreshing them to ensure they are redrawn
   correctly on the resized screen. Use this function in response to a SIGWINCH
   signal to handle terminal resizing gracefully. */
void restore_wins() {
    int i;
    // touchwin(stdscr);
    // wnoutrefresh(stdscr);
    // wrefresh(stdscr);
    for (i = 0; i <= win_ptr; i++) {
        if (win_box[i] != nullptr) {
            touchwin(win_box[i]);
            wnoutrefresh(win_box[i]);
            wrefresh(win_box[i]);
        }
        if (win_win[i] == nullptr)
            continue;
        touchwin(win_win[i]);
        wnoutrefresh(win_win[i]);
        wrefresh(win_win[i]);
    }
}
/** @brief Draw a box around the specified window
    @ingroup window_support
    @param box Pointer to the window to draw the box around
    @note This function uses NCurses functions to draw a box around the
   specified window. It adds the appropriate characters for the corners and
   edges of the box based on the current character set. Use this function when
   you want to visually separate a window from the rest of the screen with a
   border. */
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

void cbox2(WINDOW *box) {
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
    // Verticals
    for (y = 1; y < maxy - 4; y++) {
        mvwaddnwstr(box, y, 0, &bw_ve, 1);
        mvwaddnwstr(box, y, maxx, &bw_ve, 1);
    }
    // Separator line
    mvwaddnwstr(box, y, 0, &bw_lt, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(box, &bw_ho, 1);
    waddnwstr(box, &bw_rt, 1);
    // Window 2
    y++;
    mvwaddnwstr(box, y, 0, &bw_ve, 1);
    mvwaddnwstr(box, y, maxx, &bw_ve, 1);
    y++;
    mvwaddnwstr(box, y, 0, &bw_ve, 1);
    mvwaddnwstr(box, y, maxx, &bw_ve, 1);
    // Bottom
    y++;
    mvwaddnwstr(box, y, 0, &bw_bl, 1);
    for (x = 1; x < maxx; x++)
        waddnwstr(box, &bw_ho, 1);
    waddnwstr(box, &bw_br, 1);
}

/** @defgroup error_handling Error Handling
    @brief Display Error messages
 */

/** @brief Accept a single letter answer
    @ingroup error_handling
    @param em0 First error message line
    @param em1 Second error message line
    @param em2 Third error message line
    @param em3 Fourth error message line
    @return Key code of user command */
int answer_yn(char *em0, char *em1, char *em2, char *em3) {
    char title[MAXLEN];
    int line, pos, em_l, em0_l, em1_l, em2_l, em3_l;
    WINDOW *error_win;

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n%s\n%s\n%s\n\n", em0, em1, em2, em3);
        return 1;
    }

    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(chyron, 2, "N - No", 'n');
    set_chyron_key(chyron, 3, "Y - Yes", 'y');
    compile_chyron(chyron);

    em0_l = strnz(em0, COLS - 4);
    em1_l = strnz(em1, COLS - 4);
    em2_l = strnz(em2, COLS - 4);
    em3_l = strnz(em1, COLS - 4);
    em_l = max(em0_l, em1_l);
    em_l = max(em_l, em2_l);
    em_l = max(em_l, em3_l);
    em_l = max(em_l, chyron->l);
    em_l = min(em_l, COLS - 4);

    pos = ((COLS - em_l) - 4) / 2;
    line = (LINES - 6) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (box_new(5, em_l + 2, line, pos, title, true)) {
        ssnprintf(title, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s) failed", 5,
                  em_l + 2, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, em0);
    mvwaddstr(error_win, 1, 1, em1);
    mvwaddstr(error_win, 2, 1, em2);
    mvwaddstr(error_win, 3, 1, em3);
    display_chyron(error_win, chyron, 4, chyron->l + 1);
    do {
        curs_set(1);
        cmd_key = xwgetch(error_win, chyron, -1);
        curs_set(0);
        if (cmd_key == KEY_F(1) || cmd_key == 'N' || cmd_key == 'n' ||
            cmd_key == 'Y' || cmd_key == 'y')
            break;
    } while (1);
    win_del();
    destroy_chyron(chyron);
    return (cmd_key);
}
/** @brief Display an error message window or print to stderr
    @ingroup error_handling
    @param em0 First error message line
    @param em1 Second error message line
    @param em2 Third error message line
    @param em3 Fourth error message line
    @return Key code of user command */
int display_error(char *em0, char *em1, char *em2, char *em3) {
    char title[MAXLEN];
    int line, pos, em_l, em0_l, em1_l, em2_l, em3_l;
    WINDOW *error_win;

    if (!f_curses_open) {
        fprintf(stderr, "\n\n%s\n%s\n%s\n%s\n\n", em0, em1, em2, em3);
        return 1;
    }

    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(chyron, 9, "F9 Cancel", KEY_F(9));
    set_chyron_key(chyron, 10, "F10 Continue", KEY_F(10));
    compile_chyron(chyron);

    em0_l = strnz(em0, COLS - 4);
    em1_l = strnz(em1, COLS - 4);
    em2_l = strnz(em2, COLS - 4);
    em3_l = strnz(em1, COLS - 4);
    em_l = max(em0_l, em1_l);
    em_l = max(em_l, em2_l);
    em_l = max(em_l, em3_l);
    em_l = max(em_l, chyron->l);
    em_l = min(em_l, COLS - 4);

    pos = ((COLS - em_l) - 4) / 2;
    line = (LINES - 6) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (box_new(5, em_l + 2, line, pos, title, true)) {
        ssnprintf(title, MAXLEN - 1, "box_new(%d, %d, %d, %d, %s) failed", 5,
                  em_l + 2, line, pos, title);
        destroy_chyron(chyron);
        abend(-1, title);
    }
    error_win = win_win[win_ptr];
    mvwaddstr(error_win, 0, 1, em0);
    mvwaddstr(error_win, 1, 1, em1);
    mvwaddstr(error_win, 2, 1, em2);
    mvwaddstr(error_win, 3, 1, em3);
    display_chyron(error_win, chyron, 4, chyron->l + 1);
    do {
        cmd_key = xwgetch(error_win, chyron, -1);
        if (cmd_key == KEY_F(9) || cmd_key == KEY_F(10) || cmd_key == 'q' ||
            cmd_key == 'Q')
            break;
    } while (1);
    win_del();
    destroy_chyron(chyron);
    return (cmd_key);
}

/** @brief Display a simple error message window or print to stderr
    @ingroup error_handling
    @param emsg_str Error message string
    @return Key code of user command */
int Perror(char *emsg_str) {
    char emsg[80];
    int emsg_max_len = 80;
    unsigned cmd_key;
    WINDOW *error_win;
    int len, line, pos;
    char title[MAXLEN];
    bool f_xwgetch = true;
    if (emsg_str[0] == '' && emsg_str[1] == 'w') {
        emsg_str += 2;
        f_xwgetch = false;
    }
    strnz__cpy(emsg, emsg_str, emsg_max_len - 1);
    if (!f_curses_open) {
        fprintf(stderr, "\n%s\n", emsg);
        return 1;
    }
    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 1, "F1 Help", KEY_F(1));
    set_chyron_key(chyron, 9, "F9 Cancel", KEY_F(9));
    set_chyron_key(chyron, 10, "F10 Continue", KEY_F(10));
    compile_chyron(chyron);
    len = max(strlen(title), strlen(emsg));
    len = max(len, chyron->l);
    len = max(len, 40);
    pos = (COLS - len - 4) / 2;
    line = (LINES - 4) / 2;
    strnz__cpy(title, "Notification", MAXLEN - 1);
    if (box_new(2, len + 2, line, pos, title, true)) {
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
        cmd_key = xwgetch(error_win, chyron, -1);
        curs_set(0);
        win_del();
    } else {
        cmd_key = KEY_F(10);
    }
    destroy_chyron(chyron);
    return (cmd_key);
}
/** @brief Create a Chyron struct for the waiting message
    @ingroup error_handling
    @return Pointer to the chyron struct */
Chyron *wait_mk_chyron() {
    Chyron *chyron = new_chyron();
    set_chyron_key(chyron, 9, "F9 Cancel", KEY_F(9));
    compile_chyron(chyron);
    return chyron;
}
/** @brief Display a popup waiting message
    @ingroup error_handling
    @param chyron Pointer to Chyron struct for displaying key options
    @param title window title
    @return WINDOW * struct */
WINDOW *wait_mk_win(Chyron *chyron, char *title) {
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
    if (box_new(2, len + 2, line, col, title, true)) {
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
/** @brief Destroy the waiting message window and chyron
    @ingroup error_handling
    @param chyron Pointer to Chyron struct for displaying key options
    @return true if successful */
bool wait_destroy(Chyron *chyron) {
    win_del();
    destroy_chyron(chyron);
    return true;
}
/** @brief Update the waiting message with remaining time and check for user
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
    if (box_new(2, len + 2, line, col, title, true)) {
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

/** @brief For lines shorter than their display area, fill the rest with spaces
    @ingroup window_support
    @param w Pointer to window
    @param y Y coordinate
    @param x X coordinate
    @param s String to display
    @param l Length of display area */
void mvwaddstr_fill(WINDOW *w, int y, int x, char *s, int l) {
    char *d, *e;
    char tmp_str[MAXLEN];

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
    mvwaddstr(w, y, x, tmp_str);
}
/** @brief Get color index from color name
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
/** @brief list colors to stderr
    @ingroup color_management
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
/** @brief Display error message and wait for key press
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
/** @brief Abnormal program termination
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
/** @brief Wait for a process to finish with a timeout and optional user
   cancellation
    @ingroup error_handling
    @param pid Process ID to wait for
    @param timeout Time in seconds to wait before timing out
    @return true if the process finished, false if it timed out or was cancelled
 */
bool waitpid_with_timeout(pid_t pid, int timeout) {
    int status;
    int rc;
    Chyron *wait_chyron;
    WINDOW *wait_win;
    int remaining = timeout;

    usleep(100000); // Sleep for 200ms */
    rc = waitpid(pid, &status, WNOHANG);
    if (rc == pid)
        return true;
    if (rc == -1) {
        ssnprintf(em0, MAXLEN - 1, "Error waiting for process %d", pid);
        Perror(em0);
        return false;
    }
    wait_chyron = wait_mk_chyron();
    ssnprintf(em0, MAXLEN - 1, "Waiting for process %d to finish...", pid);
    wait_win = wait_mk_win(wait_chyron, em0);
    cmd_key = 0;
    while (rc == 0 && remaining > 0 && cmd_key != KEY_F(9)) {
        cmd_key = wait_continue(wait_win, wait_chyron, remaining);
        if (cmd_key == KEY_F(9))
            break;
        remaining--;
    }
    wait_destroy(wait_chyron);
    if (rc == pid)
        return true;
    return false;
}
/** @brief Wrapper for wgetch that handles signals, mouse events, checks for
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
    @note This, of course, will be expanded into an event loop for message
   queuing
    @details Get mouse event and check if it's a left click or double click. If
   the click is outside the window, ignore it. If it's on the chyron line, get
   the corresponding key command. Otherwise, store the click coordinates as
   click_y and click_x for later use. */
int xwgetch(WINDOW *win, Chyron *chyron, int n) {
    int c;
    MEVENT event;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | BUTTON4_PRESSED |
                  BUTTON5_PRESSED,
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
    curs_set(1);
    do {
        c = wgetch(win);
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
                curs_set(0);
                return KEY_UP;
            } else if (event.bstate & BUTTON5_PRESSED) {
                curs_set(0);
                return KEY_DOWN;
            }
            if (event.bstate & BUTTON1_CLICKED ||
                event.bstate & BUTTON1_DOUBLE_CLICKED) {
                if (wenclose(win, event.y, event.x)) {
                    wmouse_trafo(win, &event.y, &event.x, false);
                } else {
                    c = 0;
                    continue;
                }
                click_y = event.y;
                click_x = event.x;
                if (chyron && event.y == getmaxy(win) - 1)
                    c = get_chyron_key(chyron, event.x);
                break;
            }
        }
    } while (c == ERR);
    curs_set(0);
    restore_curses_tioctl();
    return c;
}
/** This is a version of xwgetch that checks for mouse clicks in two windows.
    Sets mouse_win to the window that was clicked and click_y and click_x to the
   coordinates of the click. If the click is in the chyron line of the second
   window, it gets the corresponding key command. Otherwise, it stores the click
   coordinates and returns 0.
 */
int dxwgetch(WINDOW *win, WINDOW *win2, Chyron *chyron, int n) {
    int c;
    MEVENT event;
    mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | BUTTON4_PRESSED |
                  BUTTON5_PRESSED,
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
    curs_set(1);
    do {
        c = wgetch(win);
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
                curs_set(0);
                return KEY_UP;
            } else if (event.bstate & BUTTON5_PRESSED) {
                curs_set(0);
                return KEY_DOWN;
            }
            if (event.bstate & BUTTON1_CLICKED ||
                event.bstate & BUTTON1_DOUBLE_CLICKED) {
                // Check if the click is in win or win2, and set mouse_win
                // accordingly
                // don't free mouse_win, since it is borrowed
                mouse_win = nullptr;
                if (wenclose(win, event.y, event.x)) {
                    if (wmouse_trafo(win, &event.y, &event.x, false))
                        mouse_win = win;
                } else if (win2 != nullptr &&
                           wenclose(win2, event.y, event.x) &&
                           wmouse_trafo(win2, &event.y, &event.x, false))
                    mouse_win = win2;
                click_y = event.y;
                click_x = event.x;
                if (mouse_win == nullptr) {
                    c = 0;
                    break;
                }
                if (chyron && event.y == getmaxy(mouse_win) - 1)
                    c = get_chyron_key(chyron, event.x);
                break;
            }
        }
    } while (c == ERR);
    curs_set(0);
    restore_curses_tioctl();
    return c;
}
