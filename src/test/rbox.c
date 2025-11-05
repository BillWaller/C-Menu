#include <wchar.h>
// rbox.c
// This program prints a rectangular box using box drawing characters.
// It uses wide characters to ensure proper rendering of the box drawing
// symbols. Compile with: gcc -o rbox rbox.c -lwchar Run with: ./rbox Expected
// output:
//         ╭───────╮
//         │       │
//         ╰───────╯
// Box drawing characters
#define BW_HO L'\x2500'  // Horizontal line
#define BW_VE L'\x2502'  // Vertical line
#define BW_TL L'\x250C'  // Top left corner
#define BW_TR L'\x2510'  // Top right corner
#define BW_BL L'\x2514'  // Bottom left corner1
#define BW_BR L'\x2518'  // Bottom right corner
#define BW_RTL L'\x256d' // Rounded top left
#define BW_RTR L'\x256e' // Rounded top right
#define BW_RBL L'\x2570' // Rounded bottom left
#define BW_RBR L'\x256f' // Rounded bottom right
#define BW_LT L'\x251C'  // Left tee
#define BW_TT L'\x252C'  // Top tee
#define BW_RT L'\x2524'  // Right tee
#define BW_CR L'\x253C'  // Cross
#define BW_BT L'\x2534'  // Bottom tee

#include <locale.h>
#include <wchar.h>

int main() {
    setlocale(LC_CTYPE, "");

    wchar_t box_top[] = {BW_RTL, BW_HO, BW_HO, BW_HO,  BW_HO,
                         BW_HO,  BW_HO, BW_HO, BW_RTR, L'\0'};
    wchar_t box_middle[] = {BW_VE, L' ', L' ', L' ',  L' ',
                            L' ',  L' ', L' ', BW_VE, L'\0'};
    wchar_t box_bottom[] = {BW_RBL, BW_HO, BW_HO, BW_HO,  BW_HO,
                            BW_HO,  BW_HO, BW_HO, BW_RBR, L'\0'};
    wprintf(L"%ls\n", box_top);
    wprintf(L"%ls\n", box_middle);
    wprintf(L"%ls\n", box_middle);
    wprintf(L"%ls\n", box_middle);
    wprintf(L"%ls\n", box_middle);
    wprintf(L"%ls\n", box_bottom);
    return 0;
}
