// panels.c - A demonstration of using panels in ncurses
// This program creates a simple UI with a box, a main window, a line number panel
// and a command line panel.
//
//
//
//
// Compile with: gcc -o panels panels.c -lcurses
//
//
#include "panels.h"
#include "curs.h"

#define PAD_COLS 1024
typedef struct {
    Panel box;
    Panel win;
    Panel lnno;
    Panel cmdln;
    Panel pad_container;
    WINDOW *pad;
    Panel pad_view;
} View;

int main() {
    setlocale(LC_ALL, "");

    open_curses();

    // window geometry
    int height = 20, width = 60;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;

    View *view = malloc(sizeof(View));

    // 1. Create the main box window
    view->box.win = newwin(height + 2, width + 2, start_y, start_x);
    view->box.pan = new_panel(view->box.win);
    wbkgrnd(view->box.win, &CC_BOX);
    wborder_set(view->box.win, &ls, &rs, &ts, &bs, &tl, &tr, &bl, &br);
    wbkgrndset(view->box.win, &CC_TITLE);
    box_title(view->box.win, "Unicode Box");

    // 2. Create a container window inside the box
    view->win.win = derwin(view->box.win, height, width, 1, 1);
    view->win.pan = new_panel(view->win.win);
    wbkgrnd(view->win.win, &CC_NT);

    // 3. Create the line number panel
    view->lnno.win = derwin(view->win.win, height - 1, 9, 0, 0);
    view->lnno.pan = new_panel(view->lnno.win);
    wbkgrnd(view->lnno.win, &CC_LNNO);

    // 4. Create the command line panel
    view->cmdln.win = derwin(view->win.win, 1, width, height - 1, 0);
    view->cmdln.pan = new_panel(view->cmdln.win);
    wbkgrnd(view->cmdln.win, &CC_CMD);

    // 5. Create the pad container, the pad, and the view panel
    view->pad_container.win = derwin(view->win.win, height - 1, width - 9, 0, 9);
    view->pad = newpad(height - 1, PAD_COLS - 1);
    view->pad_view.win = subpad(view->pad, height - 1, width - 9, 0, 0);
    view->pad_view.pan = new_panel(view->pad_view.win);
    wbkgrnd(view->pad, &CC_PAD);

    // Display some text in the windows
    mvwprintw(view->lnno.win, height / 2, 0, "Line No.");
    mvwprintw(view->cmdln.win, 0, 0, "Command Line");
    mvwprintw(view->pad, height / 2, (width - 14) / 2, "Pad Window");

    update_panels();
    doupdate();
    prefresh(view->pad, 0, 0, start_y + 1, start_x + 10, start_y + height - 1, start_x + width);
    curs_set(1);
    wgetch(view->cmdln.win);

    del_panel(view->lnno.pan);
    del_panel(view->cmdln.pan);
    del_panel(view->pad_container.pan);
    del_panel(view->win.pan);
    del_panel(view->box.pan);
    delwin(view->lnno.win);
    delwin(view->cmdln.win);
    delwin(view->pad);
    delwin(view->pad_container.win);
    delwin(view->win.win);
    delwin(view->box.win);
    endwin();
    return 0;
}
