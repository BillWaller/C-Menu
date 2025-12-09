#define _XOPEN_SOURCE_EXTENDED 1
#define NCURSES_WIDECHAR 1

#include <locale.h>
#include <ncursesw/ncurses.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

void print_cchar_string(int y, int x, const wchar_t *wstr) {
    cchar_t ch;
    attr_t attrs = A_NORMAL;
    short color_pair = 0;

    while (*wstr) {
        if (setcchar(&ch, wstr, attrs, color_pair, NULL) == ERR) {
            break;
        }
        mvadd_wch(y, x++, &ch);
        wstr++;
    }
}

int main() {
    setlocale(LC_ALL, "");               // Enable Unicode
    const char *pattern = "[A-Z][a-z]+"; // Example: Capitalized word
    const char *input = "Hello World from Ncurses";

    // Compile regex
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return 1;
    }

    // Execute regex
    regmatch_t match;
    if (regexec(&regex, input, 1, &match, 0) == 0) {
        // Extract matched substring
        size_t len = match.rm_eo - match.rm_so;
        char *matched_str = malloc(len + 1);
        if (!matched_str) {
            perror("malloc");
            regfree(&regex);
            return 1;
        }
        strncpy(matched_str, input + match.rm_so, len);
        matched_str[len] = '\0';

        // Convert to wide string
        wchar_t wmatched[256];
        mbstowcs(wmatched, matched_str, 256);

        // Initialize ncurses
        initscr();
        noecho();
        cbreak();

        // Print using cchar_t
        print_cchar_string(1, 1, wmatched);

        refresh();
        getch();
        endwin();

        free(matched_str);
    } else {
        printf("No match found.\n");
    }

    regfree(&regex);
    return 0;
}
