#define NCURSES_WIDECHAR 1

#include <locale.h>
#include <ncursesw/curses.h>
#include <stdio.h>
#include <wchar.h>

int main() {
    // Enable wide-character support
    setlocale(LC_ALL, "");

    initscr();     // Start ncurses mode
    start_color(); // Enable colors
    init_pair(1, COLOR_RED, COLOR_BLACK);

    cchar_t ch;            // Complex character variable
    wchar_t wch[2] = L"★"; // Wide character (Unicode star)
    attr_t attrs = A_BOLD; // Bold attribute
    short color_pair = 1;  // Color pair index

    // Set the complex character with attributes and color
    if (setcchar(&ch, wch, attrs, color_pair, NULL) == ERR) {
        endwin();
        fprintf(stderr, "Error setting complex character.\n");
        return 1;
    }

    // Display the complex character at position (5,5)
    mvadd_wch(5, 5, &ch);
    refresh();

    // Retrieve the character's data back
    wchar_t retrieved_wch[2];
    attr_t retrieved_attrs;
    short retrieved_color;

    if (getcchar(&ch, retrieved_wch, &retrieved_attrs, &retrieved_color,
                 NULL) == ERR) {
        endwin();
        fprintf(stderr, "Error getting complex character.\n");
        return 1;
    }

    // Move cursor and print retrieved info
    mvprintw(7, 5, "Retrieved char: %lc", retrieved_wch[0]);
    mvprintw(8, 5, "Attributes: %lu, Color pair: %d",
             (unsigned long)retrieved_attrs, retrieved_color);
    refresh();

    getch();  // Wait for key press
    endwin(); // End ncurses mode
    return 0;
}
