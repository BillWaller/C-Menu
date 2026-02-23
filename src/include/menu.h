/** @file menu.h
 *  @brief Menu data structures, enums, types, end external declarations
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#ifndef _MENU_H
#define _MENU_H 1

#include <cm.h>

#ifndef _COMMON_H
typedef struct Init Init;
#endif

#define MAX_MENU_LINES 256 /** Maximum number of menu lines in a menu */

/**< Menu line types */
enum { MT_NULL = 0x320, MT_TEXT, MT_CHOICE };

/**< Menu Action types */
enum {
    MA_INIT = 350,
    MA_RETURN,
    MA_RETURN_MAIN,
    MA_DISPLAY_MENU,
    MA_ENTER_OPTION
};

/**< Menu command types */
enum {
    CT_NULL = 0x396,
    CT_RETURNMAIN,
    CT_EXEC,
    CT_HELP,
    CT_ABOUT,
    CT_FORM,
    CT_FORM_EXEC,
    CT_FORM_WRITE,
    CT_MENU,
    CT_PICK,
    CT_VIEW,
    CT_CKEYS,
    CT_RETURN,
    CT_TOGGLE,
    CT_WRITE_CONFIG,
    CT_UNDEFINED
};

/** @struct Line
    @brief The Line strutures are attached to the Menu main structure */
typedef struct {
    unsigned int type; /**< The type of menu line, e.g. MT_TEXT for a text line,
                          MT_CHOICE for a choice line */
    char *raw_text;    /**< The raw text of the menu line, as read from the menu
                          description file, before processing any variables or
                          formatting */
    char *choice_text; /**< The formatted text of the menu line, after
                          processing any variables and formatting, used for
                          display purposes */
    char choice_letter; /**< The letter that the user can press to select this
                           menu line, if it is a choice line, used for selection
                           purposes */
    int letter_pos;     /**< The position of the choice letter in the text, for
                           highlighting purposes, used to determine where to
                           highlight the choice letter in the menu display */
    unsigned int
        command_type;  /**< The type of command associated with this menu line,
                          e.g. CT_RETURNMAIN for a command that returns to the
                          main menu, CT_EXEC for a command that executes a
                          program, CT_HELP for a command that displays help
                          information, etc., used to determine what action to
                          take when this menu line is selected */
    char *command_str; /**< The command string associated with this menu line,
                          which may be a command to execute, a menu to display,
                          a form to show, etc., depending on the command type,
                          used to determine the specific action to take when
                          this menu line is selected */
} Line;

/** @struct Menu
    @brief The Menu structure is the main data structure for the menu
   application, containing all the information about the menu, its lines, and
   its display properties */
typedef struct {
    int fg_color; /**< The foreground color for the menu display, used to
                     determine the color of the text and other elements in the
                     menu display */
    int bg_color; /** <The background color for the menu display, used to
                     determine the color of the background in the menu display
                   */
    int bo_color; /**< The color for the box around the menu, used to determine
                     the color of the box in the menu display */
    int lines;    /**< The number of lines for the menu window size, used to
                     determine the height of the menu display */
    int cols;     /**< The number of columns for the menu window size, used to
                     determine the width of the menu display */
    int begy; /**< The screen line for the upper left corner of the menu window,
                 used to determine the vertical position of the menu display on
                 the screen */
    int begx; /**< The screen column for the upper left corner of the menu
                 window, used to determine the horizontal position of the menu
                 display on the screen */
    WINDOW *win; /**< The ncurses window structure for the menu display, used to
                    manage the display of the menu on the screen and handle user
                    input */
    WINDOW *box; /**< The ncurses window structure for the box around the menu,
                    used to manage the display of the box around the menu on the
                    screen */
    char title[MAXLEN]; /**< The title to display on the top line of the box
                           window, used to provide a title or header for the
                           menu display */
    int argc;    /**< The number of arguments on the command line, used to
                    determine how many arguments were passed to the menu
                    application when it was executed */
    char **argv; /**< The argument vector from the command line, used to access
                    the specific arguments passed to the menu application when
                    it was executed */
    char mapp_spec[MAXLEN]; /**< The menu application description file spec,
                               which may be a file name or a fully qualified
                               path to a file, used to specify the location of
                               the menu description file that contains the
                               information about the menu lines and their
                               properties */
    char help_spec[MAXLEN]; /**< The menu application help file spec, which may
                               be a file name or a fully qualified path to a
                               file, used to specify the location of the help
                               file that contains the information about the menu
                               application and how to use it */
    char provider_cmd[MAXLEN]; /**< The provider command, which can be executed
                                  in the background to provide dynamic content
                                  for a program called by the menu application,
                                  used to specify a command that can be run in
                                  the background to generate dynamic content for
                                  the menu display or for a program that is
                                  called by the menu application */
    char receiver_cmd
        [MAXLEN];     /**< The receiver command, which can be executed in the
                         background to process the output of a program called by the
                         menu application, used to specify a command that can be run
                         in the background to process the output of a program that
                         is called by the menu application, such as parsing the
                         output and updating the menu display or performing some
                         other action based on the output */
    char cmd[MAXLEN]; /**< A command that can be executed in the foreground,
                         possibly taking control of the screen, by the menu
                         application, such as an editor or other program that is
                         called by the menu application, used to specify a
                         command that can be run in the foreground when selected
                         from the menu, which may take control of the screen and
                         require user interaction */
    bool f_mapp_spec; /**< A flag to indicate whether the menu application
                         description file has been verified, used to indicate
                         whether the menu description file specified by
                         mapp_spec has been successfully verified and can be
                         used to populate the menu lines and their properties */
    bool f_help_spec; /**< A flag to indicate whether the menu application help
                         file has been verified, used to indicate whether the
                         help file specified by help_spec has been successfully
                         verified and can be used to provide help information
                         for the menu application */
    bool f_provider_cmd; /**< A flag to indicate whether the provider command
                            has been verified, used to indicate whether the
                            provider command specified by provider_cmd has been
                            successfully verified and can be executed in the
                            background to provide dynamic content for the menu
                            display or for a program called by the menu
                            application */
    bool f_receiver_cmd; /**< A flag to indicate whether the receiver command
                            has been verified, used to indicate whether the
                            receiver command specified by receiver_cmd has been
                            successfully verified and can be executed in the
                            background to process the output of a program called
                            by the menu application */
    bool f_cmd; /**< A flag to indicate whether the foreground command has been
                   verified, used to indicate whether the command specified by
                   cmd has been successfully verified and can be executed in the
                   foreground when selected from the menu */
    int choice_max_len; /**< The longest choice text string of all menu lines,
                           used to determine the width of the menu window,
                           calculated based on the length of the choice_text for
                           all menu lines and used to ensure that the menu
                           window is wide enough to accommodate the longest
                           choice text without truncation or wrapping */
    int text_max_len;   /**< The longest text string of all menu lines, used to
                           determine the width of the menu window, calculated
                           based on the length of the raw_text for all menu lines
                           and used to ensure that the menu window is wide enough
                           to accommodate the longest text without truncation or
                           wrapping */
    int item_count;     /**< The number of menu lines in this menu, used to
                           determine how many lines are currently defined in the
                           menu and to manage the array of menu lines */
    int line_idx; /**< The index of the currently selected menu line, used for
                     highlighting and selection purposes, updated based on user
                     input to indicate which menu line is currently selected and
                     should be highlighted in the menu display */
    Line *line[MAX_MENU_LINES]; /**< An array of pointers to the menu line
                                   structures in this menu, used to store the
                                   information about each menu line, including
                                   its type, text, choice letter, command type,
                                   and command string, and to manage the menu
                                   lines in the menu display */
    bool f_stop_on_error; /**< A flag to indicate whether the menu application
                             should stop on error, used for error handling
                             purposes to determine whether the menu application
                             should terminate or continue running when an error
                             occurs, such as a failure to verify a file or
                             command, or an error in processing user input */
} Menu;
extern Menu *menu;

extern unsigned int get_command_type(char *);
extern void free_menu_line(Line *);
#endif
