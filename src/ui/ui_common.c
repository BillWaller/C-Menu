#include "cm.h"
#include "ui_backend.h"

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
            mbstr_to_cellstr(cx,
                             " ",
                             &cell_base,
                             &pos,
                             MAXLEN - 1);
        } else {
            mbstr_to_cellstr(chyron->cmplx_buf,
                             "|",
                             &cell_base,
                             &pos,
                             MAXLEN - 1);
        }
        cx = chyron->cmplx_buf;
        mbstr_to_cellstr(cx, chyron->key[k]->text, &cell_base, &pos, MAXLEN - 1);
        end_pos = pos;
        chyron->l = end_pos;
        chyron->key[k]->end_pos = end_pos;
        ssnprintf(tmp_str, MAXLEN - 1, "k=%d, text=%s, end_pos=%d", k,
                  chyron->key[k]->text, chyron->key[k]->end_pos);
        k++;
    }
    mbstr_to_cellstr(chyron->cmplx_buf, " ", &cell_base, &pos, MAXLEN - 1);
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
