#define _XOPEN_SOURCE_EXTENDED
#define NCURSES_WIDECHAR 1
#include <locale.h> // For setlocale
#include <ncursesw/ncurses.h>
// #include <wchar.h>  // For wide character types and functions

int main() {
    // Set the locale to support wide characters (e.g., UTF-8)
    setlocale(LC_ALL, "");

    // Initialize ncurses
    initscr();
    cbreak(); // Line buffering disabled, pass characters immediately
    noecho(); // Don't echo input characters

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Your terminal does not support colors.\n");
        return 1;
    }
    if (!can_change_color()) {
        endwin();
        fprintf(stderr, "Your terminal cannot change colors.\n");
        return 1;
    }
    if (!use_default_colors()) {
        mvaddwstr(0, 0, L"Your terminal cannot use default colors.");
    }

    // Print a wide character string
    mvaddwstr(2, 0, L"Hello, Unicode! ♥ ♠ ♣ ♦");

    // Print individual wide characters
    mvadd_wch(4, 0, WACS_BLOCK);
    mvadd_wch(4, 0, (cchar_t *)'A');
    mvadd_wch(4, 1, (const cchar_t *)L'é');  // Example accented character
    mvadd_wch(4, 2, (const cchar_t *)L'漢'); // Example Chinese character

    // Refresh the screen to show the output
    refresh();

    // Wait for user input before exiting
    getch();

    // End ncurses mode
    endwin();

    return 0;
}
