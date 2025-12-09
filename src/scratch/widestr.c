#define _XOPEN_SOURCE_EXTENDED
#include <locale.h>
#include <ncursesw/curses.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

int main(void) {
    int i;
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();

    cchar_t ch;            // Complex character variable
    wchar_t wch[2] = L"★"; // Wide character (Unicode star)
    attr_t attrs = A_BOLD; // Bold attribute
    short color_pair = 1;  // Color pair index

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
    }

    wchar_t source_wcs[] = L"Hello, wide world! \x2603";
    wchar_t destination_wcs[50];
    wcscpy(destination_wcs, source_wcs);
    mvaddwstr(2, 5, destination_wcs);

    // copy a portion of a wide string
    wchar_t source_short[] = L"Short";
    wchar_t destination_fixed[10];
    wcsncpy(destination_fixed, source_short, 9);
    mvaddwstr(4, 5, destination_fixed);
    refresh();

    // getcchar and setcchar example
    const wchar_t *wide_string =
        L"Now is the time for all good men to come to the aid of their nation.";
    const wchar_t *wide_string2 = L"Hello World! \u2665\u2605";

    getcchar(&ch, wch, &attrs, &color_pair, NULL);
    setcchar(&ch, wch, attrs, color_pair, NULL);

    int length = i;

    mvaddwstr(5, 5, wide_string);
    refresh();

    // Highlight on-screen text (attron and atroff aren't necessary here)
    move(5, 18);
    attron(A_REVERSE | COLOR_PAIR(1));
    chgat(4, A_REVERSE | COLOR_PAIR(1), 1, NULL);
    attroff(A_REVERSE | COLOR_PAIR(1));
    refresh();

    // multibyte to wide character conversion example
    char *multibyte_str = "Hello, world! \xE2\x98\x83";
    // UTF-8 for "Hello, world! ☃"
    wchar_t wide_str_buffer[100]; // Destination buffer

    // Calculate buffer size: The number of wide characters is usually less than
    // the number of bytes
    size_t buffer_size = sizeof(wide_str_buffer) / sizeof(wchar_t);

    // 3. Convert the multibyte string to a wide character string
    size_t converted_count =
        mbstowcs(wide_str_buffer, multibyte_str, buffer_size);

    if (converted_count == (size_t)-1) {
        mvprintw(0, 0, "Conversion error occurred!");
    } else {
        if (converted_count < buffer_size)
            wide_str_buffer[converted_count] = L'\0';
        mvaddwstr(2, 5, wide_str_buffer);
    }
    refresh();
    getch();
    endwin();
    return 0;
}
