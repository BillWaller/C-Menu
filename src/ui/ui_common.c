#include "cm.h"
#include "ui_backend.h"
#include <string.h>

/** BOX WIDE UNICODE CODEPOINTS */

BorderWide bw;

wchar_t *border_single = L"─│├┤┬┴┼┌┐└┘";
wchar_t *border_double = L"═║╠╣╦╩╬╔╗╚╝";
wchar_t *border_rounded = L"─│├┤┬┴┼╭╮╰╯";
wchar_t *border_heavy = L"━┃┣┫┳┻╋┏┓┗┛";
wchar_t *border_none = L"           ";

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

// -----------------------------------------------------------------------
// Standard VGA 16-Color Palette
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

// There is also a 256 color palette which is not included here.
// You don't really need any palette. It's mostly here for nostalgia. You can
// show it to your friends and say "look at this." I tell you, when I was a kid,
// we only had 16 colors. And I used to plow four acres of corn with a two-horse team.
// And we liked it!

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
// High Level Surface Instantiations
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
        Perror("Maximum number of surfaces (%d) exceeded");
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
        Perror("Maximum number of surfaces (%d) exceeded");
        exit(EXIT_FAILURE);
    }
    if (sfc_ptr >= SFC_MAX) {
        Perror("Maximum number of surfaces (%d) exceeded");
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
// border_draw
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
   to the newly created chyron. If memory allocation fails, it calls abend() to
   terminate the program with an error message.
 */

UiChyron *ui_new_chyron() {
    UiChyron *chyron = (UiChyron *)calloc(1, sizeof(UiChyron));
    if (!chyron) {
        abend(-1, "calloc chyron failed");
        return nullptr;
    }
    for (int i = 0; i < CHYRON_KEYS; i++) {
        chyron->key[i] = (UiChyronKey *)calloc(1, sizeof(UiChyronKey));
        if (!chyron->key[i]) {
            abend(-1, "calloc chyron->key[i] failed");
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
int ui_assign_chyron_win(UiChyron *chyron, UiSurface *sfc, uint win, char *y) {
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
void ui_display_chyron(UiSurface *sfc, uint w, UiChyron *chyron, uint line, uint col) {
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
