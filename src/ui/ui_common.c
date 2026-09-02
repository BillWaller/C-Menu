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

BorderWide bw;

int click_y;
int click_x;
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
double GRAY_GAMMA = 1.2; /**< Gamma correction. Set in .minitrc */
double RED_GAMMA = 1.2;
double GREEN_GAMMA = 1.2;
double BLUE_GAMMA = 1.2;

/** BOX WIDE UNICODE CODEPOINTS */
const wchar_t *border_single = L"─│├┤┬┴┼┌┐└┘";
const wchar_t *border_double = L"═║╠╣╦╩╬╔╗╚╝";
const wchar_t *border_rounded = L"─│├┤┬┴┼╭╮╰╯";
const wchar_t *border_heavy = L"━┃┣┫┳┻╋┏┓┗┛";
const wchar_t *border_none = L"           ";

/** MISCELANEOUS UNICODE CODEPOINTS */
const wchar_t *bw_rtl = L"\x256d"; /**< rounded top left */
const wchar_t *bw_rtr = L"\x256e"; /**< rounded top right */
const wchar_t *bw_rbl = L"\x2570"; /**< rounded bottom left */
const wchar_t *bw_rbr = L"\x256f"; /**< rounded bottom right */
const wchar_t *bw_sp = L"\x20";    /**< space */
const wchar_t *bw_ra = L"\x2192";  /**< large right arrow */
const wchar_t *bw_la = L"\x2190";  /**< large left arrow */
const wchar_t *bw_ua = L"\x2191";  /**< large up arrow */
const wchar_t *bw_da = L"\x2193";  /**< large down arrow */
const wchar_t *bw_ran = L"\x276F"; /**< right_angle */
const wchar_t *bw_lan = L"\x276E"; /**< left_angle */
const wchar_t *bw_chk = L"\x2611"; /**< left_angle */
const wchar_t *bw_h09 = L"\x23BD"; /**< horizontal line 9 */

/** colors_text
    @brief Color names for .minitrc overrides
    @details These names are used in .minitrc to specify color overrides The
   order of these names corresponds to the ColorsEnum values */
char const colors_text[][10] = {
    "black", "red", "green", "yellow", "blue", "magenta", "cyan",
    "white", "orange", "bg", "abg", "bblack", "bred", "bgreen",
    "byellow", "bblue", "bcyan", "bmagenta", "bwhite", "borange", ""};

// -----------------------------------------------------------------------
// Standard UiCells
// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------
// Standard Color Pairs
// -----------------------------------------------------------------------

ushort cp_box;       /**< border */
ushort cp_ind;       /**< indicator character */
ushort cp_cmdln;     /**< command line */
ushort cp_title;     /**< border title */
ushort cp_nt;        /**< normal text */
ushort cp_nt_rev;    /**< normal text reverse */
ushort cp_nt_hl;     /**< normal text highlight */
ushort cp_nt_hl_rev; /**< normal text highlight reverse */
ushort cp_ln;        /**< line numbers */
ushort cp_default;   /**< default color pair */
ushort cp_fill_char; /**< field fill characters */
ushort cp_brackets;  /**< field enclosure brackets */
ushort cp_red;       /**< red background - for testing */
ushort cp_green;     /**< green background - for testing */
ushort cp_yellow;    /**< yellow background - for testing */
ushort cp_blue;      /**< blue background - for testing */

// -----------------------------------------------------------------------
// Standard VGA Colors
// -----------------------------------------------------------------------

STDRGB std_color[] = {
    {0, 0, 0},
    {128, 0, 0},
    {0, 128, 0},
    {128, 128, 0},
    {0, 0, 128},
    {128, 0, 128},
    {0, 128, 128},
    {192, 192, 192},
    {128, 128, 128},
    {255, 0, 0},
    {0, 255, 0},
    {255, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {0, 255, 255},
    {255, 255, 255}};

//-------------------------------------------------------------------------
// Initialization
// -------------------------------------------------------------------------

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
    // Initialize Standardized color pairs
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
    // Initialize Standardized UiCells
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
// -----------------------------------------------------------------------------
// Multibyte, Wide Character (wchar_t), and Complex Character (cchar_t) Support
// -----------------------------------------------------------------------------

/** mbc_to_wc
    @brief Convert a multibyte character to a wide character
    @ingroup window_support
    @param wc Output array for wide characters (size 2)
    @param mbc Input multibyte character
    @details This function converts a single multibyte character to a wide
   character. It uses the mbrtowc function to perform the conversion and handles
   any errors by setting the output to a question mark ('?') if the conversion
   fails. The output array is null-terminated. */
void ui_mbc_to_wc(wchar_t wc[2], const char mbc) {
    wc[0] = wc[1] = L'\0';
    mbstate_t state = {0};
    size_t len = mbrtowc(wc, &mbc, MB_CUR_MAX, &state);
    if (len <= 0) {
        wc[0] = L'?';
        wc[1] = L'\0';
        len = 1;
    }
}
/** ui_mbstr_to_wcstr
    @brief Convert a multibyte string to a wide character string
    @ingroup window_support
    @param mb_str Input multibyte string
    @return Pointer to newly allocated wide character string, or nullptr on error
    @details This function converts a null-terminated multibyte string to a wide
   character string. It first calculates the required length for the wide
   character string, allocates memory for it, and then performs the conversion.
   The caller is responsible for freeing the allocated memory. If the conversion
   fails, it returns nullptr. */
wchar_t *ui_mbstr_to_wcstr(const char *mb_str) {
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
/** ui_mbstr_to_cellstr
    @brief Convert multibyte string to complex character array
    @ingroup UiChyron
    @param cmplx_buf Output buffer for complex characters
    @param str Input multibyte string
    @param cell_base Base cell for attributes and color pair
    @param p Pointer to current position in the output buffer, updated as
   characters are added
    @param atmost Maximum length of the output buffer
    @return Number of bytes processed from the input string
    @details This function converts a multibyte string to an array of complex
   characters (cchar_t) that can be used with NCurses functions. It handles
   multibyte characters and applies the attributes and color pair from the base
   cell to each character. The p parameter is updated to reflect the current
   position in the output buffer, and the function ensures that it does not exceed
   the maximum length specified by atmost.
*/
#ifdef NCURSES_UI
uint ui_mbstr_to_cellstr(UiCell *cmplx_buf, const char *str, const UiCell *cell_base, uint *p, const uint atmost) {
    attr_t attrs;
    short cp;
    uint p1 = 0;
    uint *pos = &p1;
    if (p)
        pos = p;
    else
        pos = &p1;
    uint i = 0, len = 0;
    const char *s;
    UiCell cc = {0};
    wchar_t wstr[5];
    getcchar(cell_base, &wstr[0], &attrs, &cp, nullptr);
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    if (pos && *pos >= atmost - 1)
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
        if (*pos > atmost)
            break;
        if (setcchar(&cc, wstr, attrs, cp, nullptr) != ERR) {
            if (len > 0 && (*pos + len) < atmost)
                cmplx_buf[(*pos)++] = cc;
        }
        i += len;
    }
    wstr[0] = L'\0';
    wstr[1] = L'\0';
    setcchar(&cc, wstr, attrs, cp, nullptr);
    cmplx_buf[*pos] = cc;
    return *pos;
}
#else
uint ui_mbstr_to_cellstr(UiCell *cmplx_buf, const char *str, const UiCell *cell_base, uint *p, const uint atmost) {
    ushort cp;
    uint p1 = 0;
    uint *pos = &p1;
    if (p)
        pos = p;
    uint i = 0, len = 0;
    const char *s;
    attr_t style;
    UiCell cc;
    wchar_t wstr[5];
    ui_getcchar(cell_base, &wstr[0], &style, &cp, nullptr);
    mbstate_t mbstate;
    memset(&mbstate, 0, sizeof(mbstate));
    if (pos && *pos >= atmost - 1)
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
        if (*pos > atmost)
            break;
        if (ui_setcchar(&cc, wstr, style, cp, nullptr) != ERR) {
            if (len > 0 && (*pos + len) < atmost)
                cmplx_buf[(*pos)++] = cc;
        }
        i += len;
    }
    wstr[0] = L'\0';
    wstr[1] = L'\0';
    ui_setcchar(&cc, wstr, style, cp, nullptr);
    cmplx_buf[*pos] = cc;
    return *pos;
}
#endif
// -----------------------------------------------------------------------------
// High Level Surface Instantiation
// -----------------------------------------------------------------------------
/** surface_box_win_new
    @brief Create a new surface with a box window
    @ingroup window_support
    @param wlines Number of lines for the window
    @param wcols Number of columns for the window
    @param wbegy Starting Y position for the window
    @param wbegx Starting X position for the window
    @param wtitle Title for the window
    @return 0 on success, -1 on error
    @details This function creates a new surface and adds a box window to it.
   It checks if the maximum number of surfaces has been exceeded and adjusts the
   window size based on the screen size. The new surface is stored in the global
   ui_surface array, and the surface pointer (sfc_ptr) is incremented. The window
   is then rendered on the screen.
 */
int ui_surface_box_win_new(uint wlines, uint wcols, uint wbegy, uint wbegx, char *wtitle) {
    if (sfc_ptr >= SFC_MAX) {
        ui_log(ERROR, "Maximum number of surfaces (%d) exceeded", SFC_MAX);
        exit(EXIT_FAILURE);
    }
    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    wlines = min(wlines, maxy - 2);
    wcols = min(wcols, maxx - 2);
    sfc_ptr++;
    // ------------------->    UAL_win_box    <-------------------
    ui_surface[sfc_ptr] = ui_box_surface_new(nullptr, 0, wlines, wcols, wbegy, wbegx, wtitle);
    UiSurface *sfc = ui_surface[sfc_ptr];
    ui_surface_addwin(sfc, WIN, BOX, wlines, wcols, 1, 1);
    ui_render();
    return 0;
}
// -----------------------------------------------------------------------------
// box_split_new
// -----------------------------------------------------------------------------
/** surface_split_box_win_new
    @brief Create a new surface with a split box window
    @ingroup window_support
    @param wlines Number of lines for the main window
    @param wcols Number of columns for the main window
    @param split_y Y position for the split line
    @param split_x X position for the split line (not implemented yet)
    @param wbegy Starting Y position for the window
    @param wbegx Starting X position for the window
    @param wtitle Title for the window
    @return 0 on success, -1 on error
    @details This function creates a new surface and adds a split box window to it.
   It checks if the maximum number of surfaces has been exceeded and adjusts the
   window size based on the screen size. The new surface is stored in the global
   ui_surface array, and the surface pointer (sfc_ptr) is incremented. The main
   window and the split window are then rendered on the screen, with a separator
   line drawn at the specified Y position.
 */
int ui_surface_split_box_win_new(uint wlines, uint wcols, uint split_y, uint split_x, uint wbegy, uint wbegx, char *wtitle) {
    if (sfc_ptr >= SFC_MAX) {
        ui_log(ERROR, "Maximum number of surfaces (%d) exceeded", SFC_MAX);
        exit(EXIT_FAILURE);
    }
    uint maxy, maxx;
    ui_get_screen_size(&maxy, &maxx);
    wlines = min(wlines, maxy - 2);
    wcols = min(wcols, maxx - 2);
    split_x = min(split_x, maxx - 2); // not implemented yet
    uint split_wlines = min(wlines + split_y + 1, maxy - 2);
    wcols = min(wcols, maxx - 2);
    sfc_ptr++;
    // ------------------->    surface_new    <-------------------
    ui_surface[sfc_ptr] = ui_box_surface_new(nullptr, 0, split_wlines, wcols, wbegy, wbegx, wtitle);
    UiSurface *sfc = ui_surface[sfc_ptr];

    ui_surface_addwin(sfc, WIN, BOX, wlines, wcols, 1, 1);
    ui_render();

    ui_border_ysplit(sfc, wlines + 1);
    ui_render();
    ui_surface_addwin(sfc, WIN2, BOX, 2, wcols, wlines + 2, 1);
    ui_curs_set(0);
    return 0;
}
// -----------------------------------------------------------------------------
// ui_cm_surface_destroy
// -----------------------------------------------------------------------------
/** ui_cm_destroy_surface
    @brief Destroy the most recently created surface
    @ingroup window_support
    @return 0 on success, -1 if no surfaces exist
    @details This function destroys the most recently created surface and decrements the surface pointer. It should be called when a surface is no longer needed to free up resources. If there are no surfaces to destroy, it returns -1.
    @note The difference between this function and ui_surface_destroy() is that this function destroys the surface pointed to by the surface pointer (sfc_ptr) and decrements the surface pointer after destroying the surface.
 */

int ui_cm_surface_destroy(UiSurface *sfc) {
    ui_surface_destroy(sfc);
    sfc_ptr--;
    return 0;
}
// -----------------------------------------------------------------------------
// ui_border_draw
// -----------------------------------------------------------------------------
int ui_border_draw(UiSurface *sfc) {
    uint maxy = ui_getmaxy(sfc, BOX);
    uint maxx = ui_getmaxx(sfc, BOX);
    uint y = 0;
    uint x = 0;
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_tl, 1);
    ui_render();
    for (x = 1; x < maxx - 1; x++)
        ui_mvwadd_wchnstr(sfc, BOX, y, x, &cell_ho, 1);
    ui_render();
    ui_mvwadd_wchnstr(sfc, BOX, y, maxx - 1, &cell_tr, 1);
    ui_render();
    for (y = 1; y < maxy - 1; y++) {
        ui_mvwadd_wchnstr(sfc, BOX, y, 0, &cell_ve, 1);
        ui_render();
        ui_mvwadd_wchnstr(sfc, BOX, y, maxx - 1, &cell_ve, 1);
        ui_render();
    }
    ui_mvwadd_wchnstr(sfc, BOX, y, 0, &cell_bl, 1);
    for (x = 1; x < maxx - 1; x++)
        ui_mvwadd_wchnstr(sfc, BOX, y, x, &cell_ho, 1);
    ui_mvwadd_wchnstr(sfc, BOX, y, maxx - 1, &cell_br, 1);
    ui_render();
    return 0;
}
/** ui_border-ysplit
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
int ui_border_ysplit(UiSurface *sfc, uint y) {
    uint maxx = ui_getmaxx(sfc, BOX);
    ui_mvwadd_wchnstr(sfc, BOX, y, 0, &cell_lt, 1);
    for (uint x = 1; x < maxx - 1; x++)
        ui_mvwadd_wchnstr(sfc, BOX, y, x, &cell_ho, 1);
    ui_mvwadd_wchnstr(sfc, BOX, y, maxx - 1, &cell_rt, 1);
    return 0;
}
/** ui_border_ysplit_text
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
int ui_border_ysplit_text(UiSurface *sfc, char *text, uint separator_line) {
    uint maxx = ui_getmaxx(sfc, BOX);
    uint l;
    uint y = separator_line;
    uint x = 0;
    // Draw the horizontal line with text in the middle, so we start by drawing
    // the left edge, then the text, then the right edge, and finally fill in the
    // horizontal line on either side of the text.
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_lt, 1);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_ho, 1);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_rt, 1);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_sp, 1);
    strnz(text, maxx - 7);
    wchar_t *text_wc;
    text_wc = ui_mbstr_to_wcstr(text);
    l = wcswidth(text_wc, wcslen(text_wc));
    ui_bkgdset(sfc, BOX, &cell_nt_hl);
    ui_mvwaddnwstr(sfc, BOX, y, x, text_wc, l);
    x += l;
    l = min(l, maxx - 7);
    free(text_wc);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_sp, 1);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_lt, 1);

    while (x < maxx - 1)
        ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_ho, 1);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_rt, 1);
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
int ui_border_title(UiSurface *sfc, char *title) {
    if (!title || !*title)
        return 0;
    uint y = 0;
    uint x = 0;
    uint l;
    uint maxx = ui_getmaxx(sfc, BOX);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_tl, 1);
    ui_render();
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_rt, 1);
    ui_render();
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_sp, 1);
    ui_render();
    wchar_t *title_wc;
    title_wc = ui_mbstr_to_wcstr(title);
    l = wcswidth(title_wc, wcslen(title_wc));
    l = min(l, maxx - 7);
    ui_bkgdset(sfc, BOX, &cell_title);
    ui_mvwaddnwstr(sfc, BOX, y, x, title_wc, l);
    ui_render();
    ui_bkgdset(sfc, BOX, &cell_box);
    x += l;
    free(title_wc);
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_sp, 1);
    ui_render();
    ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_lt, 1);
    ui_render();
    while (x < maxx - 1) {
        ui_mvwadd_wchnstr(sfc, BOX, y, x++, &cell_ho, 1);
        ui_render();
    }
    return 0;
}
// -----------------------------------------------------------------------
// Chyron API
// -----------------------------------------------------------------------
/** new_chyron
    @brief Create new chyron structure
    @ingroup UiChyron
    @return pointer to new chyron structure
    @details This function allocates memory for a new chyron structure and its
   associated keys. It initializes the structure to zero and returns a pointer
   to the newly created chyron. If memory allocation fails, it calls ui_abend() to
   terminate the program with an error message.
 */
UiChyron *ui_new_chyron() {
    UiChyron *chyron = (UiChyron *)calloc(1, sizeof(UiChyron));
    if (!chyron) {
        ui_abend(-1, "calloc chyron failed");
        return nullptr;
    }
    for (int i = 0; i < CHYRON_KEYS; i++) {
        chyron->key[i] = (UiChyronKey *)calloc(1, sizeof(UiChyronKey));
        if (!chyron->key[i]) {
            ui_abend(-1, "calloc chyron->key[i] failed");
            return nullptr;
        }
    }
    return chyron;
}
/** assign_chyron_win
    @brief Assign surface and window to chyron structure
    @ingroup UiChyron
    @param chyron pointer to UiChyron structure
    @param sfc pointer to UiSurface structure
    @param win window index (WIN or BOX)
    @param y line number for chyron display
    @return 0 on success, -1 on error
    @details This function assigns the specified surface and window to the chyron
   structure. It also sets the line number for displaying the chyron. If the
   input string y is "-", it sets the line number to the last line of the
   surface. Otherwise, it converts the string y to an integer and sets it as the
   line number, ensuring it does not exceed the maximum line number of the
   surface.
    @note Drop-down menus from the chyron will be added in the future
 */
int ui_assign_chyron_win(UiChyron *chyron, UiSurface *sfc, ss_t w, char *y) {
    bool a_toi_error = false;
    if (!sfc)
        return -1;
    chyron->sfc = sfc;
    chyron->win = w;
    if (*y == '-')
        chyron->y = ui_getmaxy(sfc, w) - 1;
    else {
        chyron->y = a_toi(y, &a_toi_error);
        if (a_toi_error)
            return -1;
        chyron->y = min(chyron->y, ui_getmaxy(sfc, w) - 1);
    }
    return 0;
}
/** destroy_chyron
    @brief Destroy UiChyron structure
    @ingroup UiChyron
    @param chyron pointer to UiChyron structure
    @return nullptr
 */
UiChyron *ui_destroy_chyron(UiChyron *chyron) {
    uint i;

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
    @ingroup UiChyron
    @param chyron structure
    @param k Function key index (0-19)
    @return true if set, false if not set */
bool ui_is_set_chyron_key(UiChyron *chyron, uint k) {
    if (chyron->key[k]->text[0] != '\0')
        return true;
    else
        return false;
}
/** set_chyron_key
    @brief Set chyron key with color pair (cp)
    @ingroup UiChyron
    @param chyron structure
    @param k chyron key index (0-19)
    @param s chyron key label
    @param kc chyron key code
    @param cell_base color pair for chyron key label
    @details This function is like set_chyron_key, except it includes a color
   pair numbers */
void ui_set_chyron_key_cb(UiChyron *chyron, uint k, char *s, uint kc, UiCell cell_base) {
    if (*s != '\0')
        ssnprintf(chyron->key[k]->text, CHYRON_KEY_MAXLEN - 1, "%s", s);
    else
        chyron->key[k]->text[0] = '\0';
    chyron->key[k]->keycode = kc;
    chyron->key[k]->active = true;
    chyron->key[k]->cell_base = cell_base;
}
/** set_chyron_key
    @brief Set chyron key with default color pair (cp_nt_rev)
    @ingroup UiChyron
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
void ui_set_chyron_key(UiChyron *chyron, uint k, char *s, uint kc) {
    if (*s != '\0')
        ssnprintf(chyron->key[k]->text, CHYRON_KEY_MAXLEN - 1, "%s", s);
    else
        chyron->key[k]->text[0] = '\0';
    chyron->key[k]->keycode = kc;
    chyron->key[k]->active = true;
    chyron->key[k]->cell_base = cell_nt_rev;
}
/** unset_chyron_key
    @brief Unset chyron key
    @ingroup UiChyron
    @param chyron structure
    @param k chyron_key index
*/
void ui_unset_chyron_key(UiChyron *chyron, uint k) {
    chyron->key[k]->text[0] = '\0';
}
/** activate_chyron_key
    @brief Activate chyron key
    @ingroup UiChyron
    @param chyron structure
    @param k chyron_key index
*/
void ui_activate_chyron_key(UiChyron *chyron, uint k) {
    chyron->key[k]->active = true;
}
/** deactivate_chyron_key
    @brief Deactivate chyron key
    @ingroup UiChyron
    @param chyron structure
    @param k chyron_key index
*/
void ui_activate_all_chyron_keys(UiChyron *chyron) {
    for (int k = 0; k < CHYRON_KEYS; k++)
        chyron->key[k]->active = true;
}
/** deactivate_chyron_key
    @brief Deactivate chyron key
    @ingroup UiChyron
    @param chyron structure
    @param k chyron_key index
*/
void ui_deactivate_chyron_key(UiChyron *chyron, uint k) {
    chyron->key[k]->active = false;
}
/** deactivate_all_chyron_keys
    @brief Deactivate all chyron keys
    @ingroup UiChyron
    @param chyron structure
*/
void ui_deactivate_all_chyron_keys(UiChyron *chyron) {
    for (int k = 0; k < CHYRON_KEYS; k++)
        chyron->key[k]->active = false;
}
/**  compile_chyron
   @brief construct the chyron string from the chyron structure
    @ingroup UiChyron
    @param chyron
    @details The chyron string is constructed by concatenating the labels of the
   set keys, separated by " | ". The end_pos values for each key are set to
   determine the zones for mouse clicks. When a mouse click occurs, the
   get_chyron_key function uses the end_pos values to determine which key was
   clicked based on the X position of the click.
*/
void ui_compile_chyron(UiChyron *chyron) {
    uint end_pos = 0;
    uint k = 0;
    uint pos = 0;
    UiCell cell_base = cell_nt_rev;
    UiCell *cx;
    char tmp_str[MAXLEN];
    while (k < CHYRON_KEYS) {
        if (chyron->key[k]->text[0] == '\0' || !chyron->key[k]->active) {
            k++;
            continue;
        }
        cell_base = chyron->key[k]->cell_base;
        if (end_pos == 0) {
            cx = chyron->cmplx_buf;
            ui_mbstr_to_cellstr(cx,
                                " ",
                                &cell_base,
                                &pos,
                                MAXLEN - 1);
        } else {
            ui_mbstr_to_cellstr(chyron->cmplx_buf,
                                "|",
                                &cell_base,
                                &pos,
                                MAXLEN - 1);
        }
        cx = chyron->cmplx_buf;
        ui_mbstr_to_cellstr(cx, chyron->key[k]->text, &cell_base, &pos, MAXLEN - 1);
        end_pos = pos;
        chyron->l = end_pos;
        chyron->key[k]->end_pos = end_pos;
        ssnprintf(tmp_str, MAXLEN - 1, "k=%d, text=%s, end_pos=%d", k,
                  chyron->key[k]->text, chyron->key[k]->end_pos);
        k++;
    }
    ui_mbstr_to_cellstr(chyron->cmplx_buf, " ", &cell_base, &pos, MAXLEN - 1);
    chyron->l = end_pos;
}
/** display_chyron
 *   @brief Display chyron on the specified line and column of the surface
    @ingroup UiChyron
    @param sfc Pointer to the UiSurface structure
    @param w Window index (WIN or BOX)
    @param chyron Pointer to the UiChyron structure
    @param line Line number where the chyron should be displayed
    @param col Column number where the chyron should start
    @details This function displays the compiled chyron string on the specified
   line and column of the given surface. It first moves the cursor to the
   specified position, clears to the end of the line, and then writes the chyron
   string. Finally, it moves the cursor back to the specified column for user
   input.
 */
void ui_display_chyron(UiSurface *sfc, ss_t w, UiChyron *chyron, uint line, uint col) {
    ui_wmove(sfc, w, line, 0);
    ui_wclrtoeol(sfc, w);
    ui_wmove(sfc, w, line, 0);
    ui_mvwadd_wchstr(sfc, w, line, 0, chyron->cmplx_buf);
    ui_wmove(sfc, w, line, col);
    return;
}
/** get_chyron_key
    @brief Get keycode from chyron
    @ingroup UiChyron
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
int ui_get_chyron_key(UiChyron *chyron, uint x) {
    uint i = 0;
    uint k = -1;
    while (i < CHYRON_KEYS - 1) {
        if (chyron->key[i]->text[0] != '\0' && chyron->key[i]->active)
            if (chyron->key[i]->end_pos >= x) {
                k = i;
                break;
            }
        i++;
    }
    return chyron->key[k]->keycode;
}
// -----------------------------------------------------------------------
// Color format translations
// -----------------------------------------------------------------------
/** ui_rgb_to_xterm256_idx
    @brief Convert RGB color to XTerm 256 color index
    @ingroup color_management
    @param rgb RGB color
    @return XTerm 256 color index
    @details This function converts an RGB color to the nearest XTerm 256 color
   index. It first checks if the color is a shade of gray, and if so, it uses
   the gray ramp. Otherwise, it calculates the nearest color in the 6x6x6 color
   cube. */
uint ui_rgb_to_xterm256_idx(RGB *rgb) {
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
/** ui_xterm256_idx_to_rgb
    @brief Convert XTerm 256 color index to RGB
    @ingroup color_management
    @param idx XTerm 256 color index
    @return RGB color
    @details This function converts an XTerm 256 color index to an RGB color. It
   first checks if the index is in the standard 16 colors, then checks if it's
   in the 6x6x6 color cube, and finally checks if it's in the gray ramp. */
RGB ui_xterm256_idx_to_rgb(uint idx) {
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

/** ui_apply_gamma
    @brief Apply gamma correction to RGB color
    @ingroup color_management
    @param rgb Pointer to RGB color
    @details This function modifies the RGB color in place. It applies gamma
   correction to the RGB color based on the gamma values set in the SIO struct.
   If the color is a shade of gray, it applies the gray gamma correction.
   Otherwise, it applies the individual red, green, and blue gamma corrections.
 */
void ui_apply_gamma(RGB *rgb) {
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
// -----------------------------------------------------------------------
// Error Handling
// -----------------------------------------------------------------------
/**
 * @defgroup error_handling Error Handling
    @brief Display Error messages
 */
/** ui_answer_yn
    @brief Accept a single letter answer
    @ingroup error_handling
    @param msg0 First error message line
    @param msg1 Second error message line
    @param msg2 Third error message line
    @param msg3 Fourth error message line
    @return Key code of user command */
int ui_answer_yn(char *msg0, char *msg1, char *msg2, char *msg3) {
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
        ui_abend(-1, title);
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
/** ui_display_error
    @brief Display an error message window or print to stderr
    @ingroup error_handling
    @param msg0 First error message line
    @param msg1 Second error message line
    @param msg2 Third error message line
    @param msg3 Fourth error message line
    @return Key code of user command */
int ui_display_error(char *msg0, char *msg1, char *msg2, char *msg3) {
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
        ui_abend(-1, title);
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

/** ui_perror
    @brief Display a simple error message window or print to stderr
    @ingroup error_handling
    @param emsg_str Error message string
    @return Key code of user command */
int ui_perror(char *emsg_str) {
    char emsg[MAXLEN];
    unsigned in_key;
    uint line, pos, cols;
    char title[MAXLEN];
    char tmp_str[MAXLEN];
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
        ssnprintf(tmp_str, MAXLEN - 1, "ui_surface_box_win_new(%d, %d, %d, %d, %s, %b) failed",
                  4, line, line, pos, title);
        ui_log(ERROR, "%s", tmp_str);
        ui_destroy_chyron(chyron);
        exit(EXIT_FAILURE);
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
/** ui_action_disposition
    @brief Display a simple action disposition message window or print to stderr
    @ingroup error_handling
    @param title Window title
    @param action_str Action description string
    @return true if successful */
bool ui_action_disposition(char *title, char *action_str) {
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
        ui_perror(em0);
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
/** ui_abend
    @brief Abnormal program termination
    @ingroup error_handling
    @param ec Exit code
    @param s Error message */
void ui_abend(int ec, char *s) {
    destroy_curses();
    restore_shell_tioctl();
    sig_dfl_mode();
    fprintf(stderr, "\n\nABEND: %s (code: %d)\n", s, ec);
    exit(EXIT_FAILURE);
}
// -----------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------
char *ui_iso8601_timestamp(char *buf, size_t n, bool local) {
    if (buf == NULL)
        return NULL;
    time_t t = time(NULL);
    struct tm *tp = local ? localtime(&t) : gmtime(&t);
    if (local) {
        strftime(buf, n, "%Y-%m-%dT%H:%M:%S%z", tp);
    } else {
        strftime(buf, n, "%Y-%m-%dT%H:%M:%SZ", tp);
    }
    return buf;
}
#define AS_STRING(NAME) #NAME,
const char *subsfc_s[] = {
    SUB_SURFACE_LIST(AS_STRING)};

const char *ui_sub_surface_str(ss_t w) {
    if (w < BOX || w >= SUB_SFC_MAX)
        return "unknown";
    return subsfc_s[w];
}

#define AS_STRING(NAME) #NAME,
const char *const ui_log_level_s[] = {
    LOG_LEVEL_LIST(AS_STRING)};

// ANSI Color Strings for UiLog
const char *const ui_logcolor[] = {
    [FATAL] = "\033[1;31m",   // Bold Red
    [ERROR] = "\033[0;31m",   // Regular Red
    [WARN] = "\033[0;33m",    // Yellow
    [INFO] = "\033[0;32m",    // Green
    [VERBOSE] = "\033[0;36m", // Cyan
    [DEBUG] = "\033[0;34m",   // Blue
    [SILENT] = "SILENT",
};

FILE *ui_log_fp = NULL;
char ui_log_file_name[MAXLEN] = "/tmp/ui.log"; // default log file
const char *const ui_logcolor[];
LogLevel ui_min_log_level = INFO;
bool ui_timestamp_local = true; // default to local time for timestamps
char ui_timestamp[32];

FILE *ui_open_log() {
    if (!ui_log_fp) {
        if (strlen(ui_log_file_name) == 0)
            strnz__cpy(ui_log_file_name, "/tmp/ui_default.log", MAXLEN - 1);
        ui_log_fp = fopen(ui_log_file_name, "w");
        if (!ui_log_fp) {
            fprintf(stderr, "Failed to open log file: %s\n", ui_log_file_name);
            exit(EXIT_FAILURE);
        }
        setvbuf(ui_log_fp, NULL, _IOLBF, BUFSIZ);
    }
    char ttyname[MAXLEN];
    char cmenu_user[MAXLEN];
    char *p = getenv("USER");
    strnz__cpy(cmenu_user, p, MAXLEN - 1);
    if (ttyname_r(STDERR_FILENO, ttyname, sizeof(ttyname)) == 0)
        strnz__cpy(em0, ttyname, MAXLEN - 1);
    ssnprintf(em0, MAXLEN - 1, "Ui_Log started by user '%s' on terminal '%s'", cmenu_user, ttyname);
    ui_log(INFO, "%s:", em0);
    return ui_log_fp;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
