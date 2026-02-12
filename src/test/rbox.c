/** @file rbox.c
    @brief Display box with rounded corners
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
*/

#include <wchar.h>
#define BW_HO L'\x2500'  /**< Horizontal line */
#define BW_VE L'\x2502'  /**< Vertical line */
#define BW_TL L'\x250C'  /**< op left corner */
#define BW_TR L'\x2510'  /**< Top right corner */
#define BW_BL L'\x2514'  /**< Bottom left corner1 */
#define BW_BR L'\x2518'  /**< Bottom right corner */
#define BW_RTL L'\x256d' /**< Rounded top left */
#define BW_RTR L'\x256e' /**< Rounded top right */
#define BW_RBL L'\x2570' /**< Rounded bottom left */
#define BW_RBR L'\x256f' /**< Rounded bottom right */
#define BW_LT L'\x251C'  /**< Left tee */
#define BW_TT L'\x252C'  /**< Top tee */
#define BW_RT L'\x2524'  /**< Right tee */
#define BW_CR L'\x253C'  /**< Cross */
#define BW_BT L'\x2534'  /**< Bottom tee */

#include <locale.h>
#include <wchar.h>

/** @brief Uses Unicode wide box drawing characters to to draw a rectangle.
    @note The use case for this program is simply to determine if a specific
   terminal or terminal emulator can display this type of character.
    */

int main(int argc, char **argv) {
    setlocale(LC_CTYPE, "en_US.UTF-8");
    wchar_t box_top[] = {BW_RTL, BW_HO, BW_HO, BW_HO,  BW_TT,
                         BW_HO,  BW_HO, BW_HO, BW_RTR, L'\0'};
    wchar_t box_line[] = {BW_VE, L' ', L' ', L' ',  BW_CR,
                          L' ',  L' ', L' ', BW_VE, L'\0'};
    wchar_t box_middle[] = {BW_LT, BW_HO, BW_HO, BW_HO, BW_HO,
                            BW_HO, BW_HO, BW_HO, BW_RT, L'\0'};
    wchar_t box_middle_cr[] = {BW_LT, BW_HO, BW_HO, BW_HO, BW_CR,
                               BW_HO, BW_HO, BW_HO, BW_RT, L'\0'};
    wchar_t box_bottom[] = {BW_RBL, BW_HO, BW_HO, BW_HO,  BW_BT,
                            BW_HO,  BW_HO, BW_HO, BW_RBR, L'\0'};
    wprintf(L"%ls\n", box_top);
    wprintf(L"%ls\n", box_line);
    wprintf(L"%ls\n", box_middle_cr);
    wprintf(L"%ls\n", box_bottom);
    return 0;
}
