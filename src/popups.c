/** @file popups.c
    @brief functions to create popup windows
    @author Bill Waller
    @date 2026-03
    @details This file contains functions to create popup windows for menu,
   form, pick, and view. The functions in this file are called by the main
   function in main.c when the user selects a menu option that requires a popup
   window. The functions in this file also handle command line arguments that
   may be passed to the popup windows, such as the number of lines and columns
   for the view window, and the y and x coordinates for the popup windows. The
   functions in this file also save the current state of the popup windows
   before creating a new one, and restore the state after the popup window is
   closed. The functions in this file also call the appropriate functions to
   initialize and destroy the popup windows, and to handle the user input for
   the popup windows. The functions in this file also return the appropriate
   return codes to the main function in main.c, which will determine how to
   proceed based on the return codes. The functions in this file also handle any
   errors that may occur during the creation and handling of the popup windows,
   and will return appropriate error codes to the main function in main.c, which
   will determine how to proceed based on the error codes. The functions in this
   file also handle any cleanup that may be necessary after the popup windows
   are closed, such as freeing memory and resetting variables. The functions in
   this file also handle any necessary updates to the main window after the
   popup windows are closed, such as refreshing the main window and updating any
   relevant data.
*/

#include "common.h"

int popup_menu(Init *, int, char **, int, int);
int popup_form(Init *, int, char **, int, int);
int popup_pick(Init *, int, char **, int, int);
int popup_view(Init *, int, char **, int, int, int, int);

/** @brief instantiate a menu popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param by the y coordinate for the menu window
    @param bx the x coordinate for the menu window
    @details by and bx may be set as command line option arguments, in which
   case, they will take precedence over arguments passed in the function
   arguments.
    @verbatim
    by, and bx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
int popup_menu(Init *init, int argc, char **argv, int by, int bx) {
    int rc;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Menu *sav_menu = init->menu;
    init->menu = nullptr;
    init->menu = new_menu(init, init->argc, init->argv, by, bx);
    rc = menu_engine(init);
    destroy_menu(init);
    init->menu = sav_menu;
    return rc;
}
/** @brief instantiate a pick popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param by the y coordinate for the pick window
    @param bx the x coordinate for the pick window
    @details by and bx may be set as command line option arguments, in which
   case, they will take precedence over arguments passed in the function
   arguments.
    @verbatim
    by, and bx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
int popup_pick(Init *init, int argc, char **argv, int by, int bx) {
    int rc;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Pick *sav_pick = init->pick;
    init->pick = nullptr;
    rc = init_pick(init, init->argc, init->argv, by, bx);
    destroy_pick(init);
    init->pick = sav_pick;
    return rc;
}
/** @brief instantiate a form popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param by the y coordinate for the form window
    @param bx the x coordinate for the form window
    @details by and bx may be set as command line option arguments, in which
   case, they will take precedence over arguments passed in the function
   arguments.
    @verbatim
    by, and bx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
int popup_form(Init *init, int argc, char **argv, int by, int bx) {
    int rc;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    Form *sav_form = init->form;
    init->form = nullptr;
    rc = init_form(init, init->argc, init->argv, by, bx);
    destroy_form(init);
    init->form = sav_form;
    return rc;
}

/** @brief instantiate a view popup window
    @param init the Init struct pointer
    @param argc the number of command line arguments
    @param argv the command line arguments
    @param ilines the number of lines for the view window
    @param icols the number of columns for the view window
    @param by the y coordinate for the view window
    @param bx the x coordinate for the view window
    @details ilines, cols, by, and bx may also be set as command line option
   arguments, in which case, they will take precedence over arguments passed in
   the function arguments.
    @verbatim
    ilines, cols, by, and bx may be set by
       1. the calling function
          or
       2. command line arguments
    Non-zero command line arguments will override the calling function's
   arguments.
    @endverbatim
*/
int popup_view(Init *init, int argc, char **argv, int ilines, int icols, int by,
               int bx) {
    int rc = 0;
    zero_opt_args(init);
    parse_opt_args(init, argc, argv);
    // view_stack_push(&view_stack, *init->view);
    View *view_sav = init->view;
    init->view = nullptr;
    View *view = nullptr;
    view = nullptr;
    view = new_view(init);

    if (init->lines > 0 && init->cols > 0) {
        ilines = init->lines;
        icols = init->cols;
    }
    if (init->begy > 0 || init->begx > 0) {
        by = init->begy;
        bx = init->begx;
    }
    view->begy = by;
    view->begx = bx;
    view->lines = ilines;
    view->cols = icols;
    view->f_full_screen = false;
    if (!init_view_boxwin(init))
        rc = view_file(init);
    destroy_view(init);
    init->view = view_sav;
    // view_stack_pop(&view_stack, init->view);
    return rc;
}
