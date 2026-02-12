/** @file view_engine.c
    @brief The working part of View
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include "common.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define Ctrl(c) ((c) & 0x1f)

/** @brief read the next characater from the virtual file */
#define get_next_char()                                                        \
    do {                                                                       \
        if (view->file_pos == view->file_size) {                               \
            view->f_eod = true;                                                \
            break;                                                             \
        } else                                                                 \
            view->f_eod = false;                                               \
        c = view->buf[view->file_pos++];                                       \
    } while (c == 0x0d);

/** @brief read the previous characater from the virtual file */
#define get_prev_char()                                                        \
    do {                                                                       \
        if (view->file_pos == 0) {                                             \
            view->f_bod = true;                                                \
            break;                                                             \
        } else                                                                 \
            view->f_bod = false;                                               \
        c = view->buf[--view->file_pos];                                       \
    } while (c == 0x0d);

char prev_regex_pattern[MAXLEN];
FILE *dbgfp;
int view_file(Init *);
int view_cmd_processor(Init *);
int get_cmd_char(View *, off_t *);
int get_cmd_arg(View *, char *);
void build_prompt(View *, int, char *, double elapsed);
void cat_file(View *);
void lp(char *);
void go_to_mark(View *, int);
void go_to_eof(View *);
int go_to_line(View *, off_t);
void go_to_percent(View *, int);
void go_to_position(View *, off_t);
bool search(View *, int, char *, bool);
void next_page(View *);
void prev_page(View *);
void resize_page(Init *);
void redisplay_page(View *);
void scroll_down_n_lines(View *, int);
void scroll_up_n_lines(View *, int);
off_t get_next_line(View *, off_t);
off_t get_prev_line(View *, off_t);
off_t get_pos_next_line(View *, off_t);
off_t get_pos_prev_line(View *, off_t);
int fmt_line(View *);
void display_line(View *);
bool ansi_to_cmplx();
void parse_ansi_str(char *, attr_t *, int *);
void view_display_help(Init *);
void cmd_line_prompt(View *, char *);
void remove_file(View *);

int a_toi(char *s, bool *a_toi_error);

char err_msg[MAXLEN];
/** @brief Global pointer to the current View structure, used for accessing view
   state and data across functions.
    @param init Pointer to Init structure
    @return 0 on success
    @note This function processes each file specified in the command line
   arguments. If no files are specified, it defaults to reading from standard
   input ("-").
    @note For each file, it initializes the viewing context, displays the first
   page, and enters the command processing loop.
    @note After processing each file, it cleans up resources before moving to
   the next file. */
int view_file(Init *init) {
    view = init->view;
    if (view->argc < 1) {
        view->curr_argc = -1;
        view->argc = 0;
        view->next_file_spec_ptr = "-";
    } else
        view->next_file_spec_ptr = view->argv[0];
    while (view->curr_argc < view->argc) {
        if (view->next_file_spec_ptr == NULL ||
            *view->next_file_spec_ptr == '\0') {
            break;
        }
        view->file_spec_ptr = view->next_file_spec_ptr;
        view->next_file_spec_ptr = NULL;
        if (view_init_input(view, view->file_spec_ptr)) {
            if (view->buf) {
                view->maxcol = 0;
                view->f_forward = true;
                view->page_top_pos = 0;
                view->page_bot_pos = 0;
                view->file_pos = 0;
                next_page(view);
                view_cmd_processor(init);
                munmap(view->buf, view->file_size);
                // close(view->in_fd);
            }
        } else {
            view->curr_argc++;
            if (view->curr_argc < view->argc) {
                view->next_file_spec_ptr = view->argv[view->curr_argc];
            }
        }
    }
    return 0;
}
/** @brief Main Command Processing Loop for View
    @param init Pointer to Init structure
    @return 0 on exit command
    @details This function handles user commands for navigating and manipulating
   the viewed file. It processes commands such as scrolling, searching, jumping
   to specific lines, and executing shell commands.
    @note The function maintains the state of the view and updates the display
   as needed based on user input. */
int view_cmd_processor(Init *init) {
    int tfd;
    char tmp_str[MAXLEN];
    char earg_str[MAXLEN];
    char *eargv[MAX_ARGS];
    int eargc;
    int begy, begx;
    int c;
    int max_n;
    int shift = 0;
    int search_cmd = 0;
    int prev_search_cmd = 0;
    int rc, i;
    int n = 0;
    int l = 0;
    ssize_t bytes_written;
    char *editor_ptr;
    char shell_cmd_spec[MAXLEN];
    struct timespec start, end;
    double elapsed = 0;
    bool f_clock_started = false;
    off_t n_cmd = 0L;
    view = init->view;
    view->f_timer = false;
    view->cmd[0] = '\0';
    while (1) {
        c = view->next_cmd_char;
        view->next_cmd_char = 0;
        if (!c) {
            if (view->f_redisplay_page)
                redisplay_page(view);
            view->f_redisplay_page = false;
            if (view->f_timer && f_clock_started) {
                clock_gettime(CLOCK_MONOTONIC, &end);
                elapsed = (end.tv_sec - start.tv_sec) +
                          (end.tv_nsec - start.tv_nsec) / 1e9;
                f_clock_started = false;
            }
            build_prompt(view, view->prompt_type, view->prompt_str, elapsed);
            if (view->prompt_str[0] == '\0')
                cmd_line_prompt(view, "");
            else if (view->tmp_prompt_str[0] == '\0')
                cmd_line_prompt(view, view->prompt_str);
            else
                cmd_line_prompt(view, view->tmp_prompt_str);
            rc =
                prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                         view->smincol, view->smaxrow, view->smaxcol);
            if (rc == ERR)
                Perror("Error refreshing screen");
            c = get_cmd_char(view, &n_cmd);
            if (view->f_timer) {
                clock_gettime(CLOCK_MONOTONIC, &start);
                f_clock_started = true;
            }
            view->tmp_prompt_str[0] = '\0';
            if (c >= '0' && c <= '9') {
                tmp_str[0] = (char)c;
                tmp_str[1] = '\0';
                c = get_cmd_arg(view, tmp_str);
            }
        }
        switch (c) {
        /** KEY_(ALTHOME, ALTEND) - move to end of line */
        case KEY_ALTHOME:
            view->pmincol = 0;
            break;
            /**  KEY_ALTHOME move to the first column
             @details KEY_ALTHOME and KEY_ALTEND are used for horizontal
             scrolling. KEY_ALTHOME scrolls to the beginning of the line, while
             KEY_ALTEND scrolls to the end of the line. The implementation
             checks if the maximum column exceeds the number of columns in the
             view to determine how to set the pmincol for scrolling to the end
             of the line. */
        case KEY_ALTEND:
            if (view->maxcol > view->cols)
                view->pmincol = view->maxcol - view->cols;
            else
                view->pmincol = 0;
            break;
            /**  Ctrl('R') or KEY_RESIZE - Handle terminal resize events
             @details This case handles terminal resize events, which can
             disrupt the display of the viewed file. When a resize event is
             detected, the function calls resize_page() to adjust the view
             accordingly, ensuring that the content is displayed correctly after
             the terminal size changes. */
        case Ctrl('R'):
        case KEY_RESIZE:
            resize_page(init);
            break;
        /**  'h', Ctrl('H'), KEY_LEFT, KEY_BACKSPACE - Scroll left by two
         thirds of the page width
         @details This case handles horizontal scrolling to the left. The
         commands 'h', Ctrl('H'), KEY_LEFT, and KEY_BACKSPACE are used to scroll
         left. If the numeric argument (n_cmd) is 0 or less, it defaults to
         scrolling left by 2/3rds of the page width. The implementation
         calculates the shift amount based on the view's column width and
         updates the pmincol accordingly, ensuring that it does not scroll past
         the beginning of the line. After updating the view's position, it sets
         f_redisplay_page to true to trigger a refresh of the display. */
        case 'h':
        case Ctrl('H'):
        case KEY_LEFT:
        case KEY_BACKSPACE:
            if (n_cmd <= 0)
                n_cmd = 1;
            shift = (view->cols / 3) * 2;
            max_n = view->pmincol / shift;
            n = (int)n_cmd;
            if (n > max_n)
                n = max_n;
            shift *= n;
            if (view->pmincol - shift < 0)
                view->pmincol = 0;
            else
                view->pmincol -= shift;
            view->f_redisplay_page = true;
            break;
            /**  'l', 'L', KEY_RIGHT - Scroll right by two thirds of the
             page width
             @details This case handles horizontal scrolling to the right. The
             commands 'l', 'L', and KEY_RIGHT are used to scroll right. Similar
             to the left scrolling case, if the numeric argument (n_cmd) is 0 or
             less, it defaults to scrolling right by 2/3rds of the page width.
             The implementation calculates the shift amount and updates pmincol
             to scroll right, ensuring that it does not scroll past the maximum
             column. After updating the view's position, it sets
             f_redisplay_page to true to trigger a refresh of the display. */
        case 'l':
        case 'L':
        case KEY_RIGHT:
            /// 'l' or Right Arrow to scroll right, if n_cmd is 0 or less,
            /// scroll right by 2/3rds width of page
            if (n_cmd <= 0)
                n_cmd = 1;
            shift = (view->cols / 3) * 2;
            max_n = (view->maxcol - view->pmincol) / shift;
            n = (int)n_cmd;
            if (n > max_n)
                n = max_n;
            shift *= n;
            if (view->pmincol + shift <= view->maxcol)
                view->pmincol += shift;
            else
                view->pmincol = (view->maxcol - shift) > 0
                                    ? (view->maxcol - view->cols)
                                    : 0;
            view->f_redisplay_page = true;
            break;
            /**  'k', 'K', KEY_UP, Ctrl('K') - Scroll up by a line
             @details This case handles vertical scrolling upwards. The commands
             'k', 'K', KEY_UP, and Ctrl('K') are used to scroll up. If the
             numeric argument (n_cmd) is 0 or less, it defaults to scrolling up
             by 1 line. The implementation calls the scroll_up_n_lines()
             function with the specified number of lines to scroll up. */
        case 'k':
        case 'K':
        case KEY_UP:
        case Ctrl('K'):
            /// 'k', KEY_UP, Ctrl('K') to scroll up a line, if n_cmd is 0 or
            /// less, scroll up 1 line
            if (n_cmd <= 0)
                n_cmd = 1;
            scroll_up_n_lines(view, n_cmd);
            break;
            /**  'j', 'J', KEY_DOWN, KEY_ENTER, SPACE - Scroll down by a
             line
             @details This case handles vertical scrolling downwards. The
             commands 'j', 'J', KEY_DOWN, KEY_ENTER, and SPACE are used to
             scroll down. Similar to the previous case, if the numeric argument
             (n_cmd) is 0 or less, it defaults to scrolling down by 1 line. The
             implementation calls the scroll_down_n_lines() function with the
             specified number of lines to scroll down. */
        case 'j':
        case 'J':
        case '\n':
        case ' ':
        case KEY_DOWN:
        case KEY_ENTER:
            /// 'j', KEY_ENTER, SPACE, KEY_DOWN,
            /// go down n_cmd lines, if n_cmd is 0 or less, go down 1 line
            if (n_cmd <= 0)
                n_cmd = 1;
            for (i = 0; i < n_cmd; i++) {
                scroll_down_n_lines(view, n_cmd);
            }
            break;
            /**  'b', 'B', Ctrl('B'), KEY_PPAGE - Scroll up by a page
             @details This case handles vertical scrolling upwards by a page.
             The commands 'b', 'B', Ctrl('B'), and KEY_PPAGE are used to scroll
             up by a page. If the numeric argument (n_cmd) is 0 or less, it
             defaults to scrolling up by the number of lines in the view
             (view->scroll_lines). The implementation calls the
             scroll_up_n_lines() function with the specified number of lines to
             scroll up. */
        case KEY_PPAGE:
        case 'b':
        case 'B':
        case Ctrl('B'):
            scroll_up_n_lines(view, view->scroll_lines);
            break;
        /**  'f', 'F', Ctrl('F'), KEY_NPAGE - Scroll down by a page
             @details This case handles vertical scrolling downwards by a page.
           The commands 'f', 'F', Ctrl('F'), and KEY_NPAGE are used to scroll
           down by a page. If the numeric argument (n_cmd) is 0 or less, it
           defaults to scrolling down by the number of lines in the view
           (view->scroll_lines). The implementation calls the
           scroll_down_n_lines() function with the specified number of lines to
           scroll down. */
        case 'f':
        case 'F':
        case KEY_NPAGE:
        case Ctrl('F'):
            next_page(view);
            break;
            /**  'g', KEY_HOME - Go to the beginning of the document
             @details This case handles the command to go to the beginning of
             the document. The commands 'g' and KEY_HOME are used for this
             purpose. When this command is executed, it sets pmincol to 0 to
             ensure that the view is scrolled to the leftmost position and then
             calls go_to_line() with a line number of 0 to move the view to the
             top of the document. */
        case 'g':
        case KEY_HOME:
            view->pmincol = 0;
            go_to_line(view, 0L);
            break;
            /**  KEY_LL - Go to the end of the document
             @details This case handles the command to go to the end of the
             document. The KEY_LL command is used for this purpose. When this
             command is executed, it calls the go_to_eof() function to move the
             view to the end of the document. */
        case KEY_LL:
            go_to_eof(view);
            break;
            /**  '!', Execute Shell Command from within C-Menu View
             @details This case handles the command to execute a shell command
             from within the view. The '!' command allows the user to execute a
             shell command and optionally display its output in the view. If the
             view is currently displaying help, this command is ignored. The
             user is prompted to enter a shell command, which can include a
             placeholder '%' that will be replaced with the current file name.
             The command is then executed using full_screen_shell(). If the view
             is not displaying a pipe, it updates the file position and
             specification to allow returning to the current file after
             executing the shell command. */
        case '!':
            if (view->f_displaying_help)
                break;
            if (get_cmd_arg(view, "!") == 0) {
                if (!view->f_is_pipe) {
                    view->prev_file_pos = view->page_top_pos;
                    view->next_file_spec_ptr = view->file_spec_ptr;
                    str_subc(shell_cmd_spec, view->cmd_arg, '%',
                             view->cur_file_str, MAXLEN - 1);
                    munmap(view->buf, view->file_size);
                } else
                    strnz__cpy(shell_cmd_spec, view->cmd_arg, MAXLEN - 1);
                full_screen_shell(shell_cmd_spec);
                if (!view->f_is_pipe) {
                    view->next_file_spec_ptr = view->cur_file_str;
                    return 0;
                }
            }
            break;
            /**  '+', Set Startup Command
             @details This case handles the command to set a startup command
             that will be executed when the next file is opened. The '+' command
             allows the user to specify a command that will be stored in the
             view's cmd field. When the next file is opened (via an 'N' or 'P'
             command), this startup command will be executed. The user is
             prompted to enter the startup command, and if a valid command is
             entered, it is copied into the view's cmd field for later
             execution. */
        case '+':
            if (get_cmd_arg(view, "Startup Command:") == 0)
                strnz__cpy(view->cmd, view->cmd_arg, MAXLEN - 1);
            break;
            /**  '-', Change View Settings
             @details This case handles the command to change various view
             settings. The '-' command allows the user to modify settings such
             as clearing the screen at the end of the file, ignoring case in
             search, setting the prompt type, squeezing multiple blank lines,
             and setting tab stop columns. The user is prompted to select a
             specific setting to change, and based on the selection, additional
             prompts are provided to configure the chosen setting. This command
             provides a way for users to customize their viewing experience
             within the application. */
        case '-':
            /// - Start Change Settings
            if (view->f_displaying_help)
                break;
            cmd_line_prompt(view, "(c, i, p, s, t, or h)->");
            c = get_cmd_char(view, &n_cmd);
            c = tolower(c);
            if (c >= 'A' && c <= 'Z')
                c += ' ';
            switch (c) {
                /**  'c' - Clear Screen at End of File
                 @details This case handles the setting to clear the screen at
                 the end of the file. When the user selects 'c', they are
                 prompted to choose whether to enable or disable this setting by
                 entering 'Y' or 'N'. If the user chooses 'Y', the view's
                 f_at_end_clear flag is set to true, which will clear the screen
                 when the end of the file is reached. If the user chooses 'N',
                 the flag is set to false, and the screen will not be cleared at
                 the end of the file. This setting allows users to control how
                 they want the view to behave when they reach the end of a file.
               */
            case 'c':
                /// '-c' clear screen at end of file
                cmd_line_prompt(view, "Clear Screen at End (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_at_end_clear = true;
                else if (c == 'n' || c == 'N')
                    view->f_at_end_clear = false;
                break;
                /**  'i' - Ignore Case in Search
                 @details This case handles the setting to ignore case in search
                 operations. When the user selects 'i', they are prompted to
                 choose whether to enable or disable this setting by entering
                 'Y' or 'N'. If the user chooses 'Y', the view's f_ignore_case
                 flag is set to true, which will make search operations
                 case-insensitive. If the user chooses 'N', the flag is set to
                 false, and search operations will be case-sensitive. This
                 setting allows users to customize their search experience based
                 on their preferences for case sensitivity. */
            case 'i':
                cmd_line_prompt(view, "Ignore Case in search (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_ignore_case = true;
                else if (c == 'n' || c == 'N')
                    view->f_ignore_case = false;
                break;
                /**  'p' - Set Prompt Type
                 @details This case handles the setting to configure the prompt
                 type displayed in the view. When the user selects 'p', they are
                 prompted to choose a prompt type by entering 'S' for short
                 prompt, 'L' for long prompt, or 'N' for no prompt. Based on the
                 user's selection, the view's prompt_type field is updated
                 accordingly. The short prompt type (PT_SHORT) provides a
                 concise display of information, while the long prompt type
                 (PT_LONG) offers a more detailed display. The no prompt type
                 (PT_NONE) disables the display of any prompt information. This
                 setting allows users to customize the amount of information
                 shown in the prompt based on their preferences. */
            case 'p':
                cmd_line_prompt(view, "(Short Long or No prompt)->");
                c = tolower(get_cmd_char(view, &n_cmd));
                switch (c) {
                    /**  's' - Short Prompt Type
                     @details This case sets the prompt type to short when the
                     user selects 'S'. The short prompt type (PT_SHORT) provides
                     a concise display of information in the prompt, which may
                     include essential details such as the current file name,
                     line number, and percentage through the file. This option
                     is suitable for users who prefer a minimalistic prompt that
                     still conveys key information about their position in the
                     file. */
                case 's':
                    view->prompt_type = PT_SHORT;
                    break;
                    /**  'l' - Long Prompt Type
                     @details This case sets the prompt type to long when the
                     user selects 'L'. The long prompt type (PT_LONG) provides a
                     more detailed display of information in the prompt, which
                     may include additional details such as the total number of
                     lines in the file, the current line number, the percentage
                     through the file, and possibly other contextual
                     information. This option is suitable for users who prefer a
                     more informative prompt that gives them a clearer
                     understanding of their position and context within the
                     file. */
                case 'l':
                    /**  'l' - Long Prompt Type
                     @details This case sets the prompt type to long when the
                     user selects 'L'. The long prompt type (PT_LONG) provides a
                     more detailed display of information in the prompt, which
                     may include additional details such as the total number of
                     lines in the file, the current line number, the percentage
                     through the file, and possibly other contextual
                     information. This option is suitable for users who prefer a
                     more informative prompt that gives them a clearer
                     understanding of their position and context within the
                     file. */
                    view->prompt_type = PT_LONG;
                    break;
                    /**  'n' - No Prompt Type
                     @details This case sets the prompt type to none when the
                     user selects 'N'. The no prompt type (PT_NONE) disables the
                     display of any prompt information in the view. When this
                     option is selected, users will not see any prompt details
                     such as file name, line number, or percentage through the
                     file. This setting is suitable for users who prefer a clean
                     and uncluttered view without any additional information
                     displayed in the prompt area. */
                case 'n':
                    view->prompt_type = PT_NONE;
                    break;
                default:
                    break;
                }
                break;
                /**  's' - Squeeze Multiple Blank Lines
                 @details This case handles the setting to squeeze multiple
                 blank lines in the view. When the user selects 's', they are
                 prompted to choose whether to enable or disable this setting by
                 entering 'Y' or 'N'. If the user chooses 'Y', the view's
                 f_squeeze flag is set to true, which will cause multiple
                 consecutive blank lines to be displayed as a single blank line
                 in the view. If the user chooses 'N', the flag is set to false,
                 and all blank lines will be displayed as they are in the file.
                 This setting allows users to control how blank lines are
                 displayed, which can help reduce visual clutter when viewing
                 files with many consecutive blank lines. */
            case 's':
                cmd_line_prompt(
                    view, "view->f_squeeze Multiple Blank lines (Y or N)->");
                if ((c = get_cmd_char(view, &n_cmd)) == 'y' || c == 'Y')
                    view->f_squeeze = true;
                else if (c == 'n' || c == 'N')
                    view->f_squeeze = false;
                break;
                /**  't' - Set Tab Stop Columns
                 @details This case handles the setting to configure the number
                 of columns for tab stops in the view. When the user selects
                 't', they are prompted to enter the number of columns for tab
                 stops, with the current setting displayed in the prompt. The
                 user can enter a value between 1 and 12, which will be used to
                 determine how many spaces a tab character will represent in the
                 view. If the user enters a valid number within the specified
                 range, the view's tab_stop field is updated with the new value,
                 and f_redisplay_page is set to true to refresh the display with
                 the new tab stop settings. If the user enters an invalid value,
                 an error message is displayed, and the tab stop settings remain
                 unchanged. This setting allows users to customize how tab
                 characters are displayed based on their preferences for
                 spacing. */
            case 't':
                sprintf(tmp_str,
                        "Tabstop Colums Currently %d:", view->tab_stop);
                i = 0;
                if (get_cmd_arg(view, tmp_str) == 0)
                    i = atoi(view->cmd_arg);
                if (i >= 1 && i <= 12) {
                    view->tab_stop = i;
                    view->f_redisplay_page = true;
                } else
                    Perror("Tab stops not changed");
                break;
                /**  KEY_F(1), 'H' - Display Help Information about Settings
                 * Commands @details This case handles the command to display
                 * help information about the settings commands. When the user
                 * selects 'h' or 'H', it checks if the view is currently
                 * displaying help information. If it is not, it calls the
                 * view_display_help() function to show the help information
                 * related to the settings commands. After displaying the help,
                 * it sets next_cmd_char to '-' to allow the user to easily
                 * return to the settings menu after viewing the help
                 * information. This provides users with a convenient way to
                 * access guidance on how to use the various settings options
                 * available in the application. */
            case 'H':
            case KEY_F(1):
                if (!view->f_displaying_help) {
                    view_display_help(init);
                    view = init->view;
                }
                view->next_cmd_char = '-';
                break;
            default:
                break;
            }
            break;
            /**  ':' - Set a Prompt String
             @details This case handles the command to set a custom prompt
             string in the view. When the user enters ':', they are prompted to
             enter a new prompt string. The input is captured using
             get_cmd_arg(), and if a valid string is entered, it is stored in
             the view's next_cmd_char field. This allows the user to customize
             the prompt that will be displayed in the view, providing a way to
             include specific information or messages in the prompt area based
             on their preferences. */
        case ':':
            view->next_cmd_char = get_cmd_arg(view, ":");
            break;
            /**  '/' or '?' - Search Forward or Backward
             @details This case handles the search functionality within view.
             The '/' command is used to search forward, while the '?' command is
             used to search backward. When either of these commands is entered,
             the user is prompted to enter a search pattern. The input is
             captured using get_cmd_arg(), and if a valid pattern is entered
             (not just a newline), the search() function is called with the
             specified search command, the search pattern, and a flag indicating
             whether to repeat the previous search. The search results are then
             displayed in the view, allowing users to navigate through
             occurrences of the search pattern in the file. Additionally, the
             previous search command and pattern are stored for easy repetition
             with the 'n' command. */
        case '/':
        case '?':
            strnz__cpy(tmp_str, (c == '/') ? "(forward)->" : "(backward)->",
                       MAXLEN - 1);
            search_cmd = c;
            c = get_cmd_arg(view, tmp_str);
            if (c == '\n') {
                view->f_wrap = false;
                search(view, search_cmd, view->cmd_arg, false);
                prev_search_cmd = search_cmd;
                strnz__cpy(prev_regex_pattern, view->cmd_arg, MAXLEN - 1);
                view->srch_beg_pos = view->page_top_pos;
            }
            break;
            /**  'o' or 'O' - Open a File
             @details This case handles the command to open a file while viewing
             another file. The 'o' or 'O' command allows the user to specify a
             new file to open. When this command is entered, the user is
             prompted to enter the name of the file they wish to open. The input
             is captured using get_cmd_arg(), and if a valid file name is
             entered, it is stored in the view's next_file_spec_ptr field. This
             will trigger the view to open the specified file and display its
             contents, allowing users to easily switch between files without
             exiting the application. */
        case 'o':
        case 'O':
        case 'e':
        case 'E':
            if (get_cmd_arg(view, "File name:") == 0) {
                strtok(view->cmd_arg, " ");
                view->next_file_spec_ptr = strdup(view->cmd_arg);
                view->f_redisplay_page = true;
                return 0;
            }
            break;
            /**  'g' or 'G' - Go to the End of the Document or a Specific
             Line
             @details This case handles the command to navigate to a specific
             line or the end of the document. The 'g' command is used to go to a
             specific line, while the 'G' command is used to go to the end of
             the document. When either of these commands is entered, it checks
             the numeric argument (n_cmd) provided by the user. If n_cmd is 0 or
             less, it calls go_to_eof() to move to the end of the document. If
             n_cmd is greater than 0, it calls go_to_line() with n_cmd as the
             line number to navigate to. This allows users to quickly jump to a
             specific line or the end of the file based on their input. */
        case 'G':
        case KEY_END:
            if (n_cmd <= 0)
                go_to_eof(view);
            else
                go_to_line(view, n_cmd);
            break;
            /**  'F' or KEY_F(1) - Display Help Information
             @details This case handles the command to display help information
             about the application. The 'F' command or the F1 key is used to
             trigger the display of help content. When this command is entered,
             it checks if the view is currently displaying help information. If
             it is not, it calls the view_display_help() function to show the
             help information. This provides users with a convenient way to
             access guidance and instructions on how to use the various features
             and commands available in the application. */
        case 'H':
        case KEY_F(1):
            if (!view->f_displaying_help) {
                view_display_help(init);
                view = init->view;
            }
            break;
            /**  'm' - Set a Mark at the Current Position
             @note This feature is currently not implemented and may be removed
             in the future.
             @details This case handles the command to set a mark at the current
             position in the file. The 'm' command allows the user to assign a
             mark label (a-z) to the current position in the file. When this
             command is entered, the user is prompted to enter a mark label,
             which should be a lowercase letter from 'a' to 'z'. If a valid mark
             label is entered, it is stored in the view's mark_tbl array at the
             index corresponding to the letter (e.g., 'a' corresponds to index
             0, 'b' to index 1, etc.) with the value of page_top_pos, which
             represents the current position in the file. This feature allows
             users to set marks at specific locations in the file for easy
             navigation later using the corresponding uppercase letter command.
           */
        case 'm':
            cmd_line_prompt(view, "Mark label (A-Z)->");
            c = get_cmd_char(view, &n_cmd);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                if (c >= 'A' && c <= 'Z')
                    c += ' ';
            if (c < 'a' || c > 'z')
                Perror("Not (a-z)");
            else
                view->mark_tbl[c - 'a'] = view->page_top_pos;
            break;
            /**  'M' - Go to a Mark
             @note This feature is currently not implemented and may be removed
             in the future.
             @details This case handles the command to navigate to a previously
             set mark in the file. The 'M' command allows the user to jump to a
             mark that was set using the 'm' command. When this command is
             entered, the user is prompted to enter a mark label, which should
             be an uppercase letter from 'A' to 'Z'. If a valid mark label is
             entered, it is converted to lowercase (if necessary) and used to
             retrieve the corresponding position from the view's mark_tbl array.
             The view then navigates to that position in the file, allowing
             users to quickly jump back to specific locations they have marked
             for easy reference. */
        case 'M':
            cmd_line_prompt(view, "Goto mark (A-Z)->");
            c = get_cmd_char(view, &n_cmd);
            if (c == '@' || c == KEY_F(9) || c == '\033')
                break;
            // c = tolower(c);
            if (c >= 'A' && c <= 'Z')
                c += ' ';
            if (c < 'a' || c > 'z')
                Perror("Not (A-Z)");
            else
                go_to_mark(view, c);
            break;
            /**  'n' - Repeat Previous Search
             @details This case handles the command to repeat the previous
             search operation. The 'n' command allows the user to repeat the
             last search command that was executed. When this command is
             entered, it checks if there is a previous search command stored in
             prev_search_cmd. If there is no previous search command, it
             displays an error message indicating that there is no previous
             search. If there is a previous search command, it checks if the
             current bottom position of the page (page_bot_pos) is at the end of
             the file (file_size). If it is, it resets the top and bottom
             positions of the page to 0 to start from the beginning of the file.
             Then, it calls the search() function with the previous search
             command, the previous regex pattern, and a flag set to true to
             indicate that this is a repeat search. This allows users to quickly
             navigate through occurrences of the previously searched pattern
             without having to re-enter the search command. */
        case 'n':
            if (prev_search_cmd == 0) {
                Perror("No previous search");
                break;
            }
            if (view->page_bot_pos == view->file_size) {
                view->page_top_pos = 0;
                view->page_bot_pos = 0;
            }
            search(view, prev_search_cmd, prev_regex_pattern, true);
            break;
            /**  'N' - Close Current File and Open Next File
             @details This case handles the command to close the current file
             and open the next file in the list of files being viewed. The 'N'
             command allows the user to quickly move to the next file without
             having to exit and restart the application. When this command is
             entered, it checks if there are more files available in the view's
             argv array based on the current index (curr_argc) and the total
             number of files (argc). If there are no more files, it displays an
             error message indicating that there are no more files and sets
             curr_argc to the last valid index. If there are more files, it
             increments curr_argc to point to the next file and updates
             next_file_spec_ptr to point to the new file specification. This
             will trigger the view to open the next file and display its
             contents. */
        case 'N':
            if (n_cmd <= 0)
                n_cmd = 1;
            if (view->curr_argc + n_cmd >= view->argc) {
                Perror("no more files");
                view->curr_argc = view->argc - 1;
            } else {
                view->curr_argc++;
                if (view->curr_argc < view->argc)
                    view->next_file_spec_ptr = view->argv[view->curr_argc];
                return 0;
            }
            break;
            /**  'P' or '%' - Go to a Percent of the File
             @details This case handles the command to navigate to a specific
             percentage of the file. The 'P' command or the '%' key is used for
             this purpose. When this command is entered, it checks the numeric
             argument (n_cmd) provided by the user. If n_cmd is less than 0, it
             calls go_to_line() with a line number of 1 to move to the beginning
             of the file. If n_cmd is 100 or greater, it calls go_to_eof() to
             move to the end of the file. Otherwise, it calls go_to_percent()
             with n_cmd as the percentage value to navigate to that specific
             percentage of the file. This allows users to quickly jump to a
             position in the file based on a percentage, which can be useful for
             large files where line numbers may not be as meaningful. */
        case 'p':
        case '%':
            /// 'p' or '%' Go to Percent of File
            if (n_cmd < 0)
                go_to_line(view, 1);
            if (n_cmd >= 100)
                go_to_eof(view);
            else
                go_to_percent(view, n_cmd);
            break;
            /**  Ctrl('Z') - Send File to Print Queue with Notation
             @note This feature is currently not supported for lack of testing.
             @details This case handles the command to send the currently viewed
             file to the print queue with an optional notation. The Ctrl('Z')
             command allows the user to specify a notation that can be included
             with the print job. When this command is entered, the user is
             prompted to enter a notation, which is captured using
             get_cmd_arg(). A temporary file is created to store the notation,
             and shell commands are used to send the file to the print queue.
             After the print command is executed, the temporary file is removed.
             This feature provides users with a way to print the current file
             while including additional information in the print job through the
             use of notations. */
        case Ctrl('Z'):
            get_cmd_arg(view, "Enter Notation:");
            strnz__cpy(tmp_str, "/tmp/view-XXXXXX", MAXLEN - 1);
            tfd = mkstemp(tmp_str);
            strnz__cpy(view->tmp_file_name_ptr, tmp_str, MAXLEN - 1);
            if (tfd == -1) {
                Perror("Unable to create temporary file");
                break;
            }
            strnz__cpy(shell_cmd_spec, "echo ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->cmd_arg, MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            strnz__cpy(shell_cmd_spec, "cat ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->cmd_arg, MAXLEN - 5);
            strnz__cat(shell_cmd_spec, ">>", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            lp(view->cur_file_str);
            prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                     view->smincol, view->smaxrow, view->smaxcol);
            shell(shell_cmd_spec);
            ssnprintf(shell_cmd_spec, (size_t)(MAXLEN - 5), "rm %s",
                      view->tmp_file_name_ptr);
            strnz__cpy(shell_cmd_spec, "rm ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->tmp_file_name_ptr, MAXLEN - 5);
            shell(shell_cmd_spec);
            restore_wins();
            view->f_redisplay_page = true;
            unlink(tmp_str);
            break;
            /**  'P' or KEY_CATAB or KEY_PRINT - Print Current File
             @note this feature is currently not supported for lack of testing.
             @details This case handles the command to print the currently
             viewed file. The 'P' command, KEY_CATAB, and KEY_PRINT are used for
             this purpose. When this command is entered, it simply prints the
             name of the current file being viewed (cur_file_str) to the user.
             After printing the file name, it sets f_redisplay_page to true to
             refresh the display. This provides users with a quick way to see
             the name of the file they are currently viewing, which can be
             useful when navigating through multiple files or when working with
             piped input. */
        case Ctrl('P'):
        case KEY_CATAB:
        case KEY_PRINT:
            lp(view->cur_file_str);
            view->f_redisplay_page = true;
            break;
            /**  'P' or KEY_F(9) or ESC - Close Current File and Open
             Previous Pile
             @details This case handles the command to close the current file
             and open the previous file in the list of files being viewed. The
             'P' command, KEY_F(9), and ESC are used for this purpose. When this
             command is entered, it checks if there are previous files available
             in the view's argv array based on the current index (curr_argc). If
             there are no previous files, it displays an error message
             indicating that there is no previous file and sets curr_argc to 0.
             If there are previous files, it decrements curr_argc to point to
             the previous file and updates next_file_spec_ptr to point to the
             new file specification. This will trigger the view to open the
             previous file and display its contents, allowing users to easily
             navigate back through their list of files without exiting the
             application. */
        case 'P':
            if (n_cmd <= 0)
                n_cmd = 1;
            if (view->curr_argc - n_cmd < 0) {
                Perror("No previous file");
                view->curr_argc = 0;
            } else {
                view->curr_argc--;
                if (view->curr_argc >= 0)
                    view->next_file_spec_ptr = view->argv[view->curr_argc];
                return 0;
            }
            break;
            /**  'q' or 'Q' or KEY_F(9) or ESC - Quit the Application
             @details This case handles the command to quit the application. The
             'q', 'Q', KEY_F(9), and ESC commands are used for this purpose.
             When this command is entered, it sets curr_argc to argc to indicate
             that there are no more files to view and sets next_file_spec_ptr to
             NULL. This will trigger the application to exit the view loop and
             terminate, allowing users to quit the application gracefully when
             they are finished viewing files. */
        case 'q':
        case 'Q':
        case KEY_F(9):
        case '\033':
            /// 'q', 'Q', F9, ESC - Quit
            view->curr_argc = view->argc;
            view->next_file_spec_ptr = NULL;
            return 0;
            /**  'v' - Open Current File in Editor
             @details This case handles the command to open the currently viewed
             file in an external editor. The 'v' command is used for this
             purpose. When this command is entered, it first checks if the view
             is currently displaying help information, and if so, it ignores the
             command. If the view is displaying piped input, it displays an
             error message indicating that editing standard input is not
             supported and suggests writing the data to a file and using the 'w'
             command to save it before trying again. If the view is not
             displaying piped input, it retrieves the default editor from the
             environment variable DEFAULTEDITOR or uses a predefined
             DEFAULTEDITOR if the environment variable is not set. It then
             prepares a shell command to open the current file in the editor and
             executes it using full_screen_shell(). After returning from the
             editor, it updates the file position and specification to allow
             returning to the current file when exiting the editor. This feature
             provides users with a convenient way to edit the currently viewed
             file using their preferred text editor directly from within the
             application. */
        case 'v':
            if (view->f_displaying_help)
                break;
            if (view->f_is_pipe) {
                strnz__cpy(em0,
                           "View doesn't support editing of standard input",
                           MAXLEN - 1);
                strnz__cpy(
                    em1, "You may write the data to a file and edit that file",
                    MAXLEN - 1);
                strnz__cpy(em2, "use the w command to initiate the write",
                           MAXLEN - 1);
                strnz__cpy(em3, "and try again.", MAXLEN - 1);
                break;
            }
            editor_ptr = getenv("DEFAULTEDITOR");
            if (editor_ptr == NULL || *editor_ptr == '\0')
                editor_ptr = DEFAULTEDITOR;
            if (editor_ptr == NULL || *editor_ptr == '\0') {
                Perror("set DEFAULTEDITOR environment variable");
                break;
            }
            view->prev_file_pos = view->page_top_pos;
            munmap(view->buf, view->file_size);
            view->next_file_spec_ptr = view->file_spec_ptr;
            strnz__cpy(shell_cmd_spec, editor_ptr, MAXLEN - 5);
            strnz__cat(shell_cmd_spec, " ", MAXLEN - 5);
            strnz__cat(shell_cmd_spec, view->cur_file_str, MAXLEN - 5);
            full_screen_shell(shell_cmd_spec);
            return 0;
            /**  'w' - Write to File
             @details This case handles the command to write the contents of the
             currently viewed file to a specified output file. The 'w' command
             allows the user to save the data being viewed, which is especially
             useful when working with piped input. When this command is entered,
             the user is prompted to enter the name of the output file using a
             form. The application then verifies the specified output file path
             and checks for write permissions. If the file can be opened
             successfully, it writes the contents of the currently viewed file
             to the specified output file. After writing, it displays a
             confirmation message indicating how many bytes were written and
             refreshes the view to reflect any changes. This feature provides
             users with a convenient way to save the contents of their current
             view to a new location on their filesystem. */
        case 'w':
            strnz__cpy(earg_str,
                       "form -d filename.f -o \"~/menuapp/data/form-out\"",
                       MAXLEN - 1);
            eargc = str_to_args(eargv, earg_str, MAX_ARGS);
            zero_opt_args(init);
            parse_opt_args(init, eargc, eargv);
            begy = view->begy + view->lines - 7;
            begx = 4;
            rc = init_form(init, eargc, eargv, begy, begx);
            if (rc == P_CANCEL) {
                destroy_form(init);
                view->f_redisplay_page = true;
                break;
            }
            strnz__cpy(tmp_str, "~/menuapp/data/form-out", MAXLEN - 1);
            verify_spec_arg(view->in_spec, tmp_str, "~/menuapp/data", ".",
                            R_OK);
            view->in_fp = fopen(view->in_spec, "r");
            if (view->in_fp == NULL) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 2);
                strnz__cpy(em1, "fopen ", MAXLEN - 1);
                strnz__cat(em1, view->in_spec, MAXLEN - 1);
                strerror_r(errno, em2, MAXLEN - 1);
                display_error(em0, em1, em2, NULL);
                return (1);
            }
            fgets(tmp_str, MAXLEN - 1, view->in_fp);
            fclose(view->in_fp);
            l = strlen(tmp_str);
            if (l > 0 && tmp_str[l - 1] == '\n')
                tmp_str[l - 1] = '\0';
            view->f_out_spec = verify_spec_arg(
                view->out_spec, tmp_str, "~/menuapp/data", ".", W_OK | S_QUIET);
            view->out_fd =
                open(view->out_spec, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            bytes_written = write(view->out_fd, view->buf, view->file_size);
            if (bytes_written != (ssize_t)view->file_size) {
                ssnprintf(em0, MAXLEN - 1, "%s, line: %d", __FILE__,
                          __LINE__ - 2);
                strnz__cpy(em1, "fwrite ", MAXLEN - 1);
                strnz__cat(em1, view->out_spec, MAXLEN - 1);
                strerror_r(errno, em2, MAXLEN - 1);
                strnz__cpy(em3, "Not all bytes written", MAXLEN - 1);
                display_error(em0, em1, em2, em3);
                return (1);
            }
            close(view->out_fd);
            ssnprintf(tmp_str, MAXLEN - 1, "Wrote %jd bytes to %s",
                      bytes_written, view->out_spec);
            cmd_line_prompt(view, tmp_str);
            view->f_redisplay_page = true;
            break;
        case CT_VIEW:
            break;
            /// Version Information
        case 'V':
            Perror("View: Version 8.0");
            break;
        default:
            break;
        }
        view->cmd_arg[0] = '\0';
    }
}
/** @brief Get Command Character and Numeric Argument
   @details This function captures user input for command characters and numeric
  arguments in the view. It handles both keyboard input and mouse events,
  specifically for mouse wheel scrolling. The function uses the ncurses library
  to detect mouse events and determine if the mouse wheel was scrolled up or
  down, returning the appropriate command character for navigation. For keyboard
  input, it captures numeric characters to build a numeric argument string,
  which is then converted to an off_t value and stored in the provided pointer.
  This allows the application to respond to user commands that may include
  numeric arguments for actions such as navigating to a specific line or
  percentage of the file.
   @note Handling mouse events, specifically for mouse wheel scrolling, which is
  common in terminal applications. The function uses the ncurses library's mouse
  handling capabilities to detect mouse events and determine if the mouse wheel
  was scrolled up or down. If a mouse event is detected, the function checks the
  state of the mouse event to see if it corresponds to a scroll up
  (BUTTON4_PRESSED) or scroll down (BUTTON5_PRESSED) action and returns the
  appropriate command character (KEY_UP or KEY_DOWN).
  @note If a mouse event is detected but does not correspond to a scroll action,
  it returns MA_ENTER_OPTION to indicate that an option should be entered.
   @note BUTTON4 and BUTTON5 are typically used for mouse wheel up and down
   */
int get_cmd_char(View *view, off_t *n) {
    int c = 0, i = 0;
    char cmd_str[33];
    cmd_str[0] = '\0';
    MEVENT event;
    mousemask(BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);
    tcflush(2, TCIFLUSH);
    do {
        c = xwgetch(view->win);
        if (c == KEY_MOUSE) {
            if (getmouse(&event) != OK)
                return (MA_ENTER_OPTION);
            if (event.bstate & BUTTON4_PRESSED)
                return (KEY_UP);
            else if (event.bstate & BUTTON5_PRESSED) {
                return (KEY_DOWN);
            }
        } else {
            if (c >= '0' && c <= '9' && i < 32) {
                cmd_str[i++] = (char)c;
                cmd_str[i] = '\0';
            }
        }
    } while (c >= '0' && c <= '9');
    *n = atol(cmd_str);
    view->cmd_arg[0] = '\0';
    return (c);
}
int get_cmd_arg(View *view, char *prompt) {
    int c;
    int numeric_arg = false;
    char *cmd_p;
    char *cmd_e;
    char prompt_s[PAD_COLS + 1];
    char *n;
    int rc, prompt_l;
    prompt_l = strnz__cpy(prompt_s, prompt, view->cols - 4);
    if (view->cmd_arg[0] != '\0')
        return 0;
    cmd_p = view->cmd_arg;
    cmd_e = view->cmd_arg + MAXLEN - 2;
    wmove(view->win, view->cmd_line, 0);
    if (prompt_l == 0)
        numeric_arg = true;
    if (prompt_l > 1) {
        wstandout(view->win);
        waddch(view->win, ' ');
        waddstr(view->win, prompt_s);
        waddch(view->win, ' ');
        wstandend(view->win);
    } else {
        if (*prompt == ':')
            numeric_arg = true;
        else {
            n = prompt;
            if (*n >= '0' && *n <= '9') {
                *cmd_p++ = *n;
                *cmd_p = '\0';
                numeric_arg = true;
            }
        }
        waddstr(view->win, prompt_s);
        wmove(view->win, view->cmd_line, prompt_l);
    }
    wclrtoeol(view->win);
    while (1) {
        rc = prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                      view->smincol, view->smaxrow, view->smaxcol);
        if (rc == ERR) {
            Perror("Error refreshing screen");
        }
        c = xwgetch(view->win);
        switch (c) {
        /// Basic Editing Keys for Command Line
        case KEY_LEFT:
        case KEY_BACKSPACE:
        case '\b':
            if (cmd_p > view->cmd_arg) {
                cmd_p--;
                if (*cmd_p < ' ' || *cmd_p == 0x7f) {
                    getyx(view->win, view->cury, view->curx);
                    if (view->curx > 0) {
                        view->curx--;
                        wmove(view->win, view->cmd_line, view->curx);
                        waddch(view->win, ' ');
                        wmove(view->win, view->cmd_line, view->curx);
                    }
                }
                getyx(view->win, view->cury, view->curx);
                if (view->curx > 0) {
                    view->curx--;
                    wmove(view->win, view->cmd_line, view->curx);
                    waddch(view->win, ' ');
                    wmove(view->win, view->cmd_line, view->curx);
                }
            }
            break;
        case '\n':
        case KEY_ENTER:
            return c;
        case '\033':
        case KEY_F(9):
            return c;
        case KEY_MOUSE:
            continue;
        default:
            *cmd_p++ = (char)c;
            *cmd_p = '\0';
            if ((char)c < ' ') {
                waddch(view->win, '^');
                c |= '@';
            } else if ((uchar)c == 0x7f)
                c = '?';
            waddch(view->win, (char)c);
            if (cmd_p >= cmd_e)
                return 0;
            if (numeric_arg && (c < '0' || c > '9'))
                return -1;
            break;
        }
    }
    return c;
}
/** @brief Build Prompt String
   @details This function constructs the prompt string that is displayed to the
  user in the command line area of the view. The prompt string can include
  various pieces of information about the current state of the view, such as the
  file name, column position, file position, percentage through the file, and
  elapsed time for timer display. The function takes into account the specified
  prompt type (short, long, or none) and builds the prompt string accordingly,
  concatenating different pieces of information based on the current state of
  the view and the provided parameters. This allows users to have a clear and
  informative prompt that reflects their current position and context within the
  file they are viewing. */
void build_prompt(View *view, int prompt_type, char *prompt_str,
                  double elapsed) {
    prompt_type = PT_LONG;
    strnz__cpy(prompt_str, " ", MAXLEN - 1);
    if (prompt_type == PT_LONG) {
        if (view->f_is_pipe)
            strnz__cat(prompt_str, "stdin", MAXLEN - 1);
        else
            strnz__cat(prompt_str, view->file_name, MAXLEN - 1);
    }
    if (view->pmincol > 0) {
        sprintf(tmp_str, "Col %d of %d", view->pmincol, view->maxcol);
        if (prompt_str[0] != '\0')
            strnz__cat(prompt_str, "|", MAXLEN - 1);
        strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
    }
    if (view->argc > 1 && prompt_type == PT_LONG) {
        sprintf(tmp_str, "File %d of %d", view->curr_argc + 1, view->argc);
        if (prompt_str[0] != '\0') {
            strnz__cat(prompt_str, "|", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1); // File Of
        }
    }
    if (prompt_type == PT_LONG) { // Byte of Byte
        if (view->page_top_pos == NULL_POSITION)
            view->page_top_pos = view->file_size;
        sprintf(tmp_str, "Pos %zd-%zd", view->page_top_pos, view->page_bot_pos);
        if (prompt_str[0] != '\0') {
            strnz__cat(prompt_str, "|", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
        if (!view->f_is_pipe) {
            if (view->file_size > 0) {
                sprintf(tmp_str, " of %zd", view->file_size);
                strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
            }
        }
    }
    if (!view->f_eod && prompt_type != PT_NONE) { // Percent
        if (view->file_size > 0 && view->page_bot_pos != 0) {
            sprintf(tmp_str, "(%zd%%)",
                    (100 * view->page_bot_pos) / view->file_size);
            if (prompt_str[0] != '\0')
                strnz__cat(prompt_str, "|", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_eod) { // End
        if (prompt_str[0] != '\0')
            strnz__cat(prompt_str, " ", MAXLEN - 1);
        strnz__cat(prompt_str, "(End)", MAXLEN - 1);
        if (view->curr_argc + 1 < view->argc) {
            base_name(tmp_str, view->argv[view->curr_argc + 1]);
            strnz__cpy(prompt_str, " Next File: ", MAXLEN - 1);
            strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
        }
    }
    if (view->f_timer) {
        sprintf(tmp_str, " secs. %.6f\n", elapsed);
        strnz__cat(prompt_str, tmp_str, MAXLEN - 1);
    }
}
/** @brief Concatenate File to Standard Output
   @details This function reads the contents of the currently viewed file and
  outputs it to the standard output (stdout). It uses a loop to read characters
  from the file until it reaches the end of data (EOD). The function checks for
  the EOD condition after each character is read, and if it is reached, it
  breaks out of the loop. Otherwise, it outputs the character to stdout using
  putchar(). This allows users to easily output the contents of the file they
  are viewing to the terminal or to another command in a pipeline. */
void cat_file(View *view) {
    int c;
    while (1) {
        get_next_char();
        if (view->f_eod)
            break;
        putchar(c);
    }
}
/** @brief Send File to Print Queue
   @details This function sends the currently viewed file to the print queue
  using a specified print command. It retrieves the print command from the
  environment variable PRINTCMD or uses a predefined PRINTCMD if the environment
  variable is not set. The function constructs a shell command by concatenating
  the print command with the name of the file to be printed (PrintFile) and
  executes it using the shell() function. This allows users to easily print the
  contents of the file they are viewing by sending it to their configured print
  command. */
void lp(char *PrintFile) {
    /// Send File to Print Queue
    char *print_cmd_ptr;
    char shell_cmd_spec[MAXLEN];
    print_cmd_ptr = getenv("PRINTCMD");
    if (print_cmd_ptr == NULL || *print_cmd_ptr == '\0')
        print_cmd_ptr = PRINTCMD;
    sprintf(shell_cmd_spec, "%s %s", print_cmd_ptr, PrintFile);
    cmd_line_prompt(view, shell_cmd_spec);
    //  wrefresh(view->win);
    prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
             view->smincol, view->smaxrow, view->smaxcol);
    shell(shell_cmd_spec);
}
/** @brief Go to Mark
   @note Marks have been disabled and may be removed in future versions of View.
   @details This function allows the user to navigate to a specific mark in the
   file being viewed. Marks are typically set by the user at specific positions
   in the file for easy reference. The function takes a character representing
   the mark (from 'a' to 'z' or '\'') and retrieves the corresponding file
   position from the view's mark table. If the mark is not set (i.e., the file
   position is NULL_POSITION), it displays an error message indicating that the
   mark is not set. If the mark is set, it calls go_to_position() with the
   retrieved file position to navigate to that location in the file. This
   feature allows users to quickly jump to predefined positions in the file,
   enhancing navigation and efficiency when working with large files. Note that
   marks have been disabled and may be removed in future versions of View. */
void go_to_mark(View *view, int c) {
    if (c == '\'')
        view->file_pos = view->mark_tbl[(NMARKS - 1)];
    else
        view->file_pos = view->mark_tbl[c - 'a'];
    if (view->file_pos == NULL_POSITION)
        Perror("Mark not set");
    else
        go_to_position(view, view->file_pos);
}
/** @brief Go to End of File
   @details This function navigates to the end of the file being viewed. It sets
   the file position to the size of the file, which effectively moves the view
   to the end. It also updates the page_top_pos to match the new file position.
   After setting these positions, it calls get_prev_char() to adjust the view
   and then calls prev_page() to display the last page of the file. This allows
   users to quickly jump to the end of the file and view its contents from
   there. */
void go_to_eof(View *view) {
    int c;
    view->file_pos = view->file_size;
    view->page_top_pos = view->file_pos;
    get_prev_char();
    prev_page(view);
}
/** @brief Go to Specific Line
   @details This function allows the user to navigate to a specific line number
   in the file being viewed. Since C-Menu View does not maintain an index of
   line numbers, it reads through the file from the beginning up to the
   specified line number. The function takes a line index (1-based) as input and
   iterates through the file character by character, counting newline characters
   until it reaches the desired line. If it reaches the end of data (EOD) before
   finding the specified line, it displays an error message indicating that the
   end of data was reached at a certain number of lines. If it successfully
   reaches the specified line, it updates the page_top_pos to the current file
   position and calls prev_page() to display the page starting from that line.
   This allows users to jump to a specific line in the file, although it may be
   less efficient for large files due to the need to read through the file
   sequentially. Future implementations may consider adding a line index for
   faster access to specific lines. */
int go_to_line(View *view, off_t line_idx) {
    int c = 0;
    off_t line_cnt = 0;
    if (line_idx <= 1) {
        go_to_position(view, 0);
        return EOF;
    }
    view->f_forward = true;
    view->file_pos = 0;
    view->page_top_pos = view->file_pos;
    line_idx = 0;
    do {
        while (c != '\n') {
            get_next_char();
            if (view->f_eod) {
                sprintf(tmp_str, "End of data at %ld lines", line_cnt - 1);
                Perror(tmp_str);
                return EOF;
            }
        }
        get_next_char();
    } while (line_cnt < line_idx - 1);
    view->page_top_pos = view->file_pos;
    prev_page(view);
    return 0;
}
void go_to_percent(View *view, int Percent) {
    /// Go to Percent of File
    int c = 0;
    if (view->file_size < 0) {
        Perror("Cannot determine file length");
        return;
    }
    view->file_pos = (Percent * view->file_size) / 100;
    view->f_forward = true;
    get_next_char();
    while (c != '\n') {
        get_prev_char();
        if (view->f_bod)
            break;
    }
    get_next_char();
    next_page(view);
}
/** @brief Go to Specific File Position
   @details This function allows the user to navigate to a specific file
   position in the file being viewed. It takes a file position (go_to_pos) as
   input and updates the view's file_pos to that position. It also sets the
   page_bot_pos to match the new file position. After updating these positions,
   it calls next_page() to display the page starting from the specified file
   position. This allows users to jump directly to a specific byte offset in the
   file, which can be useful for large files or when navigating based on byte
   positions rather than line numbers. The function locates the nearest line
   starting at or after the specified file position and displays the page
   starting from that line. */
void go_to_position(View *view, off_t go_to_pos) {
    view->f_forward = true;
    view->file_pos = go_to_pos;
    view->page_bot_pos = view->file_pos;
    next_page(view);
}
/** @brief Search for Regular Expression Pattern
   @details This function performs a search for a regular expression pattern
   within the file being viewed. It supports extended regular expressions and
   allows for both forward and backward searching based on the search command
   provided ('/' for forward, '?' for backward). The function takes a regex
   pattern as input and compiles it using the regcomp() function. It then
   iterates through the lines of the file, applying the compiled regular
   expression to each line using regexec(). If a match is found, it displays the
   matching line along with some leading context lines. The search continues
   until the entire file has been searched, wrapping around to the starting
   position if necessary. If no new matches are found after wrapping, a message
   is displayed indicating that the search is complete. The function also
   handles highlighting of matches in the display and provides information about
   the location of matches in the command line prompt. This allows users to
   efficiently search for specific patterns within large files while providing
   visual feedback on where matches occur. */
bool search(View *view, int search_cmd, char *regex_pattern, bool repeat) {
    int REG_FLAGS = 0;
    regmatch_t pmatch[1];
    regex_t compiled_regex;
    int reti;
    int line_offset;
    int line_len;
    int match_len;
    int cury = 0;
    off_t srch_curr_pos;
    bool f_page = false;
    srch_curr_pos = view->page_top_pos;
    view->srch_beg_pos = -1;
    if (repeat) {
        if (search_cmd == '/')
            srch_curr_pos = view->page_bot_pos;
    }
    if (view->f_ignore_case)
        REG_FLAGS = REG_ICASE | REG_EXTENDED;
    else
        REG_FLAGS = REG_EXTENDED;
    reti = regcomp(&compiled_regex, regex_pattern, REG_FLAGS);
    if (reti) {
        Perror("Invalid pattern");
        return false;
    }
    while (1) {
        if (search_cmd == '/') {
            if (srch_curr_pos == view->file_size && view->srch_beg_pos == 0)
                srch_curr_pos = 0;
            if (srch_curr_pos == view->srch_beg_pos) {
                if (!view->f_wrap) {
                    view->f_wrap = true;
                } else {
                    ssnprintf(tmp_str, MAXLEN - 1,
                              "Search complete: %zd bytes for %s",
                              view->file_size, regex_pattern);
                    cmd_line_prompt(view, tmp_str);
                    regfree(&compiled_regex);
                    return false;
                }
            }
            if (cury == view->scroll_lines)
                return true;
            if (!view->f_wrap)
                view->srch_beg_pos = srch_curr_pos;
            srch_curr_pos = get_next_line(view, srch_curr_pos);
            view->page_bot_pos = srch_curr_pos;
        } else {
            if (srch_curr_pos == 0 && view->srch_beg_pos == view->file_size)
                srch_curr_pos = view->file_size;
            if (srch_curr_pos == view->srch_beg_pos) {
                if (!view->f_wrap) {
                    view->f_wrap = true;
                } else {
                    ssnprintf(tmp_str, MAXLEN - 1,
                              "Search complete: %ld bytes for %s",
                              view->file_size, regex_pattern);
                    cmd_line_prompt(view, tmp_str);
                    regfree(&compiled_regex);
                    return false;
                }
            }
            srch_curr_pos = get_prev_line(view, srch_curr_pos);
            view->page_top_pos = srch_curr_pos;
        }
        fmt_line(view);
        /**  Perform extended regular expression matching. ANSI sequences and
         * Unicode characters are stripped before matching, so matching
         * corresponds to the visual display of the line. */
        reti = regexec(&compiled_regex, view->stripped_line_out,
                       compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
        /** Once a match is found, leading context lines are displayed at the
         * top of the page. After that, all subsequent lines without matches are
         * displayed until the page is full. */
        if (reti == REG_NOMATCH) {
            if (f_page) {
                display_line(view);
                if (view->cury == view->scroll_lines)
                    break;
            }
            continue;
        }
        if (reti) {
            char err_str[MAXLEN];
            regerror(reti, &compiled_regex, err_str, sizeof(err_str));
            strnz__cpy(tmp_str, "Regex match failed: ", MAXLEN - 1);
            strnz__cat(tmp_str, err_str, MAXLEN - 1);
            Perror(tmp_str);
            regfree(&compiled_regex);
            return false;
        }
        if (!f_page) {
            view->f_forward = true;
            view->cury = 0;
            wmove(view->win, view->cury, 0);
            wclrtobot(view->win);
            view->page_top_pos = srch_curr_pos;
            f_page = true;
        }
        /** Search continues displaying matches until the page is full. */
        display_line(view);
        //----------------------------------------------
        cury = view->cury;
        /** All matches on the current line are highlighted, including those not
         * displayed on the screen. Track first and last match columns for
         * prompt display. */
        view->first_match_x = -1;
        view->last_match_x = 0;
        line_len = strlen(view->stripped_line_out);
        line_offset = 0;
        while (1) {
            view->curx = line_offset + pmatch[0].rm_so;
            match_len = pmatch[0].rm_eo - pmatch[0].rm_so;
            mvwchgat(view->win, view->cury - 1, view->curx, match_len,
                     WA_REVERSE, cp_norm, NULL);
            if (view->first_match_x == -1)
                view->first_match_x = pmatch[0].rm_so;
            view->last_match_x = line_offset + pmatch[0].rm_eo;
            line_offset += pmatch[0].rm_eo;
            if (line_offset >= line_len)
                break;
            view->line_out_p = view->stripped_line_out + line_offset;
            reti = regexec(&compiled_regex, view->line_out_p,
                           compiled_regex.re_nsub + 1, pmatch, REG_FLAGS);
            /** @note lines may be much longer than the screen width, so
             * continue searching even if the line is not displayed completely.
             * The pad's complex characters (cchar_t) will handle the display
             * for horizantal scrolling. */
            if (reti == REG_NOMATCH)
                break;
            if (reti) {
                char msgbuf[100];
                regerror(reti, &compiled_regex, msgbuf, sizeof(msgbuf));
                sprintf(tmp_str, "Regex match failed: %s", msgbuf);
                Perror(tmp_str);
                regfree(&compiled_regex);
                return false;
            }
            if (view->cury == view->scroll_lines) {
                regfree(&compiled_regex);
                return true;
            }
        }
    }
    /** Update view positions and prepare prompt string for match information */
    view->file_pos = srch_curr_pos;
    view->page_bot_pos = srch_curr_pos;
    if (view->last_match_x > view->maxcol)
        ssnprintf(view->tmp_prompt_str, MAXLEN - 1,
                  "%s|%c%s|Match Cols %d-%d of %d-%d|(%zd%%)", view->file_name,
                  search_cmd, regex_pattern, view->first_match_x,
                  view->last_match_x, view->pmincol, view->smaxcol - view->begx,
                  (view->page_bot_pos * 100 / view->file_size));
    else
        ssnprintf(view->tmp_prompt_str, MAXLEN - 1,
                  "%s|%c%s|Pos %zu-%zu|(%zd%%)", view->file_name, search_cmd,
                  regex_pattern, view->page_top_pos, view->page_bot_pos,
                  (view->page_bot_pos * 100 / view->file_size));
    regfree(&compiled_regex);
    return true;
}
/** @brief Resize Viewing Page
   @details This function adjusts the size of the viewing window based on the
  current terminal dimensions and the view's settings. If the view is set to
  full screen, it resizes to occupy the entire terminal. Otherwise, it checks if
  the view's dimensions exceed the terminal size and adjusts them accordingly.
  If a resize occurs, it sets a flag to indicate that the page needs to be
  redisplayed. This function ensures that the viewing window is appropriately
  sized for the terminal, providing an optimal viewing experience for users. */
void resize_page(Init *init) {
    int scr_lines, scr_cols;
    bool f_resize = false;
    view = init->view;
    if (view->f_full_screen) {
        getmaxyx(stdscr, view->lines, view->cols);
        view->scroll_lines = view->lines - 1;
        view->cmd_line = view->lines - 1;
        view->smaxrow = view->lines - 1;
        view->smaxcol = view->cols - 1;
        wresize(view->win, view->lines, view->cols);
        wrefresh(view->win);
        wsetscrreg(view->win, 0, view->scroll_lines);
        f_resize = true;
    } else {
        getmaxyx(stdscr, scr_lines, scr_cols);
        if (view->begy + view->lines + 2 > scr_lines) {
            view->lines = (scr_lines - view->begy) - 2;
            f_resize = true;
        }
        if (view->begx + view->cols + 2 > scr_cols) {
            view->cols = (scr_cols - view->begx) - 2;
            f_resize = true;
        }
        if (f_resize) {
            view->scroll_lines = view->lines - 1;
            view->cmd_line = view->lines - 1;
            view->smaxrow = view->lines - 1;
            view->smaxcol = view->cols - 1;
            win_resize(view->lines + 2, view->cols + 2, view->title);
            restore_wins();
            wsetscrreg(view->win, 0, view->scroll_lines);
        }
    }
    if (f_resize)
        view->f_redisplay_page = true;
    else
        view->f_redisplay_page = false;
}
/** @brief Redisplay Current Page
   @details This function is responsible for redisplaying the current page of
   the file being viewed. It clears the screen and displays the lines starting
   from the top of the page (view->page_top_pos) up to the bottom of the page
   (view->page_bot_pos). The function iterates through the lines, formatting
   each line and displaying it on the screen. It also keeps track of the maximum
   column width encountered for proper horizontal scrolling. This function is
   typically called when the view needs to be refreshed, such as after resizing
   or when returning to a previously viewed page. It ensures that the current
   page is accurately displayed to the user based on the current file position
   and view settings. */
void redisplay_page(View *view) {
    int i;
    int line_len;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->page_bot_pos = view->page_top_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        view->page_bot_pos = get_next_line(view, view->page_bot_pos);
        if (view->f_eod)
            break;
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
}
/** @brief Advance to Next Page
   @details This function advances the view to the next page of the file being
   viewed. It updates the file position to the bottom of the current page and
   reads lines forward until it fills the screen with the next set of lines. The
   function keeps track of the top and bottom positions of the page, as well as
   the maximum column width for proper horizontal scrolling. It clears the
   screen and displays the next page of lines based on the updated file
   position. This allows users to navigate through the file page by page, moving
   forward through the content. */
void next_page(View *view) {
    int i;
    int line_len;
    curs_set(0);
    if (view->page_bot_pos == view->file_size)
        return;
    view->maxcol = 0;
    view->f_forward = true;
    view->cury = 0;
    view->file_pos = view->page_bot_pos;
    view->page_top_pos = view->file_pos;
    wmove(view->win, view->cury, 0);
    wclrtobot(view->win);
    for (i = 0; i < view->scroll_lines; i++) {
        get_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
    view->page_bot_pos = view->file_pos;
    curs_set(1);
}
/** @brief display previous page
   @details updates the file position to the top of the current page and reads
   lines backward until it fills the screen with the previous set of lines. The
   function keeps track of the top and bottom positions of the page, as well as
   the maximum column width for proper horizontal scrolling. It clears the
   screen and displays the previous page of lines based on the updated file
   position. This allows users to navigate backward through the file page by
   page, moving back through the content they have already viewed. */
void prev_page(View *view) {
    int i;
    curs_set(0);
    if (view->page_top_pos == 0)
        return;
    view->maxcol = 0;
    view->f_forward = false;
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->file_pos = view->page_top_pos;
    view->page_bot_pos = view->file_pos;
    for (i = 0; i < view->scroll_lines; i++) {
        get_pos_prev_line(view, view->file_pos);
        if (view->f_bod)
            break;
    }
    view->page_bot_pos = view->file_pos;
    next_page(view);
}
/** @brief Scroll N Lines
   @details This function scrolls the view up or down by a specified number of
   lines (n). It adjusts the page top and bottom pointers accordingly and
   updates the display to reflect the new position in the file. The function
   takes into account the direction of scrolling (forward or backward) and
   ensures that it does not scroll beyond the beginning or end of the file. It
   also updates the maximum column width for proper horizontal scrolling. This
   allows users to scroll through the file line by line, providing more granular
   control over navigation compared to page-by-page scrolling. */
void scroll_down_n_lines(View *view, int n) {
    int i = 0;
    int line_len;
    curs_set(0);
    if (view->page_bot_pos == view->file_size)
        return;
    view->f_forward = true;
    // Locate New Top of Page
    view->file_pos = view->page_top_pos;
    for (i = 0; i < n; i++) {
        view->page_top_pos = get_pos_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
    }
    n = i;
    // Scroll
    wscrl(view->win, n);
    // Fill in Page Bottom
    view->cury = view->scroll_lines - n;
    wmove(view->win, view->cury, 0);
    view->file_pos = view->page_bot_pos;
    for (i = 0; i < n; i++) {
        get_next_line(view, view->file_pos);
        if (view->f_eod)
            break;
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
    view->page_bot_pos = view->file_pos;
    curs_set(1);
}
/** @brief Scroll Up N Lines
   @details This function scrolls the view up (back) by a specified number of
   lines (n). It adjusts the page top and bottom pointers accordingly and
   updates the display to reflect the new position in the file. The function
   takes into account the direction of scrolling (backward) and ensures that it
   does not scroll beyond the beginning of the file. It also updates the maximum
   column width for proper horizontal scrolling. This allows users to scroll
   backward through the file line by line, providing more granular control over
   navigation compared to page-by-page scrolling. */
void scroll_up_n_lines(View *view, int n) {
    int i;
    int line_len;
    curs_set(0);
    if (view->page_top_pos == 0)
        return;
    // Locate New Top of Page
    for (i = 0; i < n; i++) {
        if (view->f_bod)
            break;
        view->page_top_pos = get_pos_prev_line(view, view->page_top_pos);
    }
    n = i;
    // Locate New Bottom of Page
    view->f_bod = false;
    for (i = 0; i < n; i++) {
        if (view->f_bod)
            break;
        view->page_bot_pos = get_pos_prev_line(view, view->page_bot_pos);
    }
    // Scroll Up
    if (n < view->scroll_lines)
        wscrl(view->win, -n);
    // Fill in Page Top
    view->cury = 0;
    wmove(view->win, view->cury, 0);
    view->file_pos = view->page_top_pos;
    for (i = 0; i < n; i++) {
        if (view->f_eod)
            break;
        get_next_line(view, view->file_pos);
        line_len = fmt_line(view);
        if (line_len > view->maxcol)
            view->maxcol = line_len;
        display_line(view);
    }
    curs_set(1);
    return;
}
/** @brief Get Next Line from File
   @details This function reads the next line from the file being viewed and
   stores it in the view's line_in_s buffer. It takes into account various
   factors such as skipping carriage return characters, handling newline
   characters to terminate lines, and compressing multiple blank lines if the
   f_squeeze flag is set. The function updates the file position pointer
   (view->file_pos) as it reads through the file and returns the updated file
   position after reading the line. This allows users to read lines sequentially
   from the file while properly handling different line-ending conventions and
   formatting requirements. */
off_t get_next_line(View *view, off_t pos) {
    uchar c;
    char *line_in_p;
    view->file_pos = pos;
    view->f_forward = true;
    do {
        if (view->file_pos == view->file_size) {
            view->f_eod = true;
            break;
        }
        c = view->buf[view->file_pos++];
    } while (c == 0x0d);
    if (view->f_eod)
        return view->file_pos;
    line_in_p = view->line_in_s;
    view->line_in_beg_p = view->line_in_s;
    view->line_in_end_p = view->line_in_s + LINE_IN_PAD_COLS;
    while (1) {
        if (c == (uchar)'\n')
            break;
        if (line_in_p >= view->line_in_end_p)
            break;
        *line_in_p++ = c;
        do {
            if (view->file_pos == view->file_size) {
                view->f_eod = true;
                break;
            }
            c = view->buf[view->file_pos++];
        } while (c == 0x0d);
        if (view->f_eod)
            return view->file_pos;
    }
    *line_in_p = '\0';
    if (view->f_squeeze) {
        while (1) {
            get_next_char();
            if (view->f_eod)
                break;
            if (c != (uchar)'\n')
                break;
        }
        get_prev_char();
    }
    return view->file_pos;
}
/** @brief Get Previous Line from File
   @details This function reads the previous line from the file being viewed and
   stores it in the view's line_in_s buffer. It takes into account various
   factors such as skipping carriage return characters, handling newline
   characters to terminate lines, and compressing multiple blank lines if the
   f_squeeze flag is set. The function updates the file position pointer
   (view->file_pos) as it reads through the file and returns the updated file
   position after reading the line. This allows users to read lines sequentially
   backward from the file while properly handling different line-ending
   conventions and formatting requirements. */
off_t get_prev_line(View *view, off_t pos) {
    uchar c;
    view->file_pos = pos;
    view->f_forward = false;
    get_prev_char();
    if (view->f_bod)
        return view->file_pos;
    while ((uchar)c != '\n')
        get_prev_char();
    if (view->f_bod)
        return view->file_pos;
    if (view->f_squeeze) {
        if ((uchar)c == '\n') {
            while (1) {
                get_prev_char();
                if (view->f_bod)
                    return view->file_pos;
                if ((uchar)c != '\n')
                    break;
            }
            get_next_char();
        }
    }
    while (1) {
        if ((uchar)c == '\n')
            break;
        get_prev_char();
        if (view->f_bod)
            break;
    }
    if (view->file_pos < view->file_size)
        view->f_eod = false;
    return view->file_pos;
}
/** @brief Get Position of Next Line
   @details This function locates the nearest line starting at or after the
   specified file position and returns the file position pointer for that line.
   It takes into account various factors such as skipping carriage return
   characters, handling newline characters to terminate lines, and compressing
   multiple blank lines if the f_squeeze flag is set. The function updates the
   file position pointer (view->file_pos) as it reads through the file and
   returns the updated file position after locating the next line. This allows
   users to navigate through the file by lines, moving forward to the next line
   based on a given file position. */
off_t get_pos_next_line(View *view, off_t pos) {
    uchar c;
    if (pos == view->file_size) {
        view->f_eod = true;
        return view->file_pos;
    }
    view->file_pos = pos;
    view->f_forward = true;
    get_next_char();
    if (view->f_eod)
        return view->file_pos;
    if (view->f_squeeze) {
        while (1) {
            if (c != '\n')
                break;
            get_next_char();
            if (view->f_eod)
                return view->file_pos;
        }
        get_prev_char();
        if (view->f_eod)
            return view->file_pos;
    }
    while (!view->f_eod) {
        if (c == '\n')
            break;
        get_next_char();
    }
    return view->file_pos;
}
/** @brief Get Position of Previous Line
   @details This function locates the nearest line starting at or before the
   specified file position and returns the file position pointer for that line.
   It takes into account various factors such as skipping carriage return
   characters, handling newline characters to terminate lines, and compressing
   multiple blank lines if the f_squeeze flag is set. The function updates the
   file position pointer (view->file_pos) as it reads through the file and
   returns the updated file position after locating the previous line. This
   allows users to navigate through the file by lines, moving backward to the
   previous line based on a given file position. */
off_t get_pos_prev_line(View *view, off_t pos) {
    uchar c;
    view->file_pos = pos;
    if (view->file_pos == 0) {
        view->f_bod = true;
        return view->file_pos;
    }
    view->f_forward = false;
    get_prev_char();
    if (view->f_bod)
        return view->file_pos;
    if (c == '\n') {
        get_prev_char();
        if (view->f_bod)
            return view->file_pos;
    }
    while (!view->f_bod) {
        if (c == '\n') {
            get_next_char();
            break;
        }
        get_prev_char();
    }
    return view->file_pos;
}
/** @brief Display Line on Pad
   @details This function is responsible for displaying a line of text on the
   pad (the viewing window). It uses the current line position (view->cury) and
   the complex character buffer (view->cmplx_buf) to render the line on the
   screen. The function also takes into account the pad refresh parameters
   (view->pminrow, view->pmincol, view->sminrow, view->smincol, view->smaxrow,
   view->smaxcol) to ensure that the display is updated correctly. After
   rendering the line, it calls prefresh() to refresh the pad and display the
   changes on the screen. This function is typically called after formatting a
   line for display to ensure that it is rendered properly in the viewing
   window. */
void display_line(View *view) {
    int rc;
    if (view->cury < 0)
        view->cury = 0;
    if (view->cury > view->scroll_lines)
        view->cury = view->scroll_lines;
    wmove(view->win, view->cury, 0);
    wclrtoeol(view->win);
    wadd_wchstr(view->win, view->cmplx_buf);
    view->cury++;
    refresh();
    rc = prefresh(view->win, view->pminrow, view->pmincol, view->sminrow,
                  view->smincol, view->smaxrow, view->smaxcol);
    if (rc == ERR)
        Perror("Error refreshing screen");
}
/** @brief Format Line for Display
   @details This function processes the input line (view->line_in_s) and formats
   it for display on the screen. It handles ANSI SGR escape sequences to apply
   text attributes and color pairs, as well as Unicode multi-byte characters to
   ensure proper rendering. The function also handles tab characters by
   inserting spaces until the next tab stop. The resulting formatted line is
   stored in the view's cmplx_buf as complex characters (cchar_t) for display,
   while a stripped version of the line without ANSI sequences is stored in
   view->stripped_line_out for searching purposes. The function returns the
   length of the processed line, which is used to track the maximum column width
   for display and searching. This allows for accurate rendering of lines with
   complex formatting and character sets while maintaining a clean version of
   the line for search operations. */
int fmt_line(View *view) {
    attr_t attr = WA_NORMAL;
    char ansi_tok[MAXLEN];
    int cpx = cp_norm;
    int i = 0, j = 0;
    int len = 0;
    const char *s;
    wchar_t wc = L'\0';
    cchar_t cc = {0};
    char *in_str = view->line_in_s;
    cchar_t *cmplx_buf = view->cmplx_buf;
    rtrim(view->line_out_s);
    /** Initialize multibyte to wide char conversion mbtowc, setcchar, and
     * getcchar can sometimes behave badly, depending on what you feed them.
     * Make sure your locale is set properly before calling them. */
    mbtowc(NULL, NULL, 0);
    while (in_str[i] != '\0') {
        if (in_str[i] == '\033' && in_str[i + 1] == '[') {
            len = strcspn(&in_str[i], "mK ") + 1;
            memcpy(ansi_tok, &in_str[i], len + 1);
            ansi_tok[len] = '\0';
            if (ansi_tok[0] == '\0') {
                if (i + 2 < MAXLEN)
                    i += 2;
                continue;
            }
            if (len == 0 || in_str[i + len - 1] == ' ') {
                i += 2;
                continue;
            } else if (in_str[i + len - 1] == 'K') {
                i += len;
                continue;
            }
            parse_ansi_str(ansi_tok, &attr, &cpx);
            i += len;
        } else {
            if (in_str[i] == '\033') {
                i++;
                continue;
            }
            s = &in_str[i];
            if (*s == '\t') {
                /**  Handle Tab Character */
                wc = L' ';
                do {
                    setcchar(&cc, &wc, attr, cpx, NULL);
                    view->stripped_line_out[j] = ' ';
                    cmplx_buf[j++] = cc;
                } while ((j < PAD_COLS - 2) && (j % view->tab_stop != 0));
                i++;
            } else {
                /** Handle Multi-byte Character Use mbtowc to get the wide
                 * character and its length in bytes from the multibyte string
                 */
                len = mbtowc(&wc, s, MB_CUR_MAX);
                if (len <= 0) {
                    /** Invalid multibyte sequence, replace with '?' */
                    wc = L'?';
                    len = 1;
                }
                /** Convert wide character + attributes to complex character */
                if (setcchar(&cc, &wc, attr, cpx, NULL) != ERR) {
                    if (len > 0 && (j + len) < PAD_COLS - 1) {
                        view->stripped_line_out[j] = *s;
                        cmplx_buf[j++] = cc;
                    }
                }
                i += len;
            }
        }
    }
    if (j > view->maxcol)
        view->maxcol = j;
    wc = L'\0';
    setcchar(&cc, &wc, WA_NORMAL, cpx, NULL);
    cmplx_buf[j] = cc;
    view->stripped_line_out[j] = '\0';
    return j;
}
/** @brief Parse ANSI SGR Escape Sequence
   @param ansi_str is the ANSI SGR escape sequence string to be parsed
   @param attr is a pointer to an attr_t variable where the parsed text
   attributes will be stored
   @param cpx is a pointer to an integer where the parsed color pair index will
   be stored
   @details This function parses ANSI SGR (Select Graphic Rendition) escape
   sequences to extract text attributes and color information. It supports SGR
   sequences for Xterm 256-color and RGB colors, allowing for a wide range of
   text formatting options. The function updates the provided attr_t variable
   with the appropriate text attributes (such as bold, italic, underline) and
   the color pair index (cpx) based on the parsed ANSI sequence. It handles
   various forms of SGR sequences, including those for setting foreground and
   background colors, as well as resetting to default colors. The function also
   includes error handling for invalid parameters in the ANSI sequence. This
   allows for dynamic and flexible text formatting based on ANSI escape codes
   embedded in the input string. */
void parse_ansi_str(char *ansi_str, attr_t *attr, int *cpx) {
    char *tok;
    char t0, t1;
    int len, x_idx;
    int fg, bg;
    int fg_clr, bg_clr;
    char *ansi_p = ansi_str + 2;
    extended_pair_content(*cpx, &fg_clr, &bg_clr);
    fg = fg_clr;
    bg = bg_clr;
    RGB rgb;
    tok = strtok((char *)ansi_p, ";m");
    bool a_toi_error = false;
    while (1) {
        if (tok == NULL || *tok == '\0')
            break;
        len = strlen(tok);
        if (len == 2) {
            t0 = tok[0];
            t1 = tok[1];
            if (t0 == '3' || t0 == '4') {
                if (t1 == '8') {
                    tok = strtok(NULL, ";m");
                    if (tok != NULL) {
                        a_toi(tok, &a_toi_error);
                        if (*tok == '5') {
                            tok = strtok(NULL, ";m");
                            if (tok != NULL) {
                                x_idx = a_toi(tok, &a_toi_error);
                                rgb = xterm256_idx_to_rgb(x_idx);
                            }
                        } else if (*tok == '2') {
                            tok = strtok(NULL, ";m");
                            rgb.r = a_toi(tok, &a_toi_error);
                            tok = strtok(NULL, ";m");
                            rgb.g = a_toi(tok, &a_toi_error);
                            tok = strtok(NULL, ";m");
                            rgb.b = a_toi(tok, &a_toi_error);
                        }
                    }
                    if (t0 == '3')
                        fg_clr = rgb_to_curses_clr(rgb);
                    else if (t0 == '4')
                        bg_clr = rgb_to_curses_clr(rgb);
                } else if (t1 == '9') {
                    if (t0 == '3')
                        fg_clr = COLOR_WHITE;
                    else if (t0 == '4')
                        bg_clr = COLOR_BLACK;
                } else if (t1 >= '0' && t1 <= '7') {
                    if (t0 == '3') {
                        x_idx = a_toi(&t1, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        fg_clr = rgb_to_curses_clr(rgb);
                    } else if (t0 == '4') {
                        x_idx = a_toi(&t1, &a_toi_error);
                        rgb = xterm256_idx_to_rgb(x_idx);
                        bg_clr = rgb_to_curses_clr(rgb);
                    }
                }
            } else if (t0 == '0') {
                *tok = t1;
                len = 1;
            }
        }
        if (len == 1) {
            if (*tok == '0') {
                *attr = WA_NORMAL;
                fg_clr = COLOR_WHITE;
                bg_clr = COLOR_BLACK;
            } else {
                switch (a_toi(tok, &a_toi_error)) {
                case 1:
                    *attr |= WA_BOLD;
                    break;
                case 2:
                    *attr |= WA_DIM;
                    break;
                case 3:
                    *attr |= WA_ITALIC;
                    break;
                case 4:
                    *attr |= WA_UNDERLINE;
                    break;
                case 5:
                    *attr |= WA_BLINK;
                    break;
                case 7:
                    *attr |= WA_REVERSE;
                    break;
                case 8:
                    *attr |= WA_INVIS;
                    break;
                default:
                    break;
                }
            }
        } else if (len == 0) {
            *attr = WA_NORMAL;
            fg_clr = COLOR_WHITE;
            bg_clr = COLOR_BLACK;
        }
        tok = strtok(NULL, ";m");
    }
    if (!a_toi_error && (fg_clr != fg || bg_clr != bg)) {
        clr_pair_idx = get_clr_pair(fg_clr, bg_clr);
        *cpx = clr_pair_idx;
    }
    return;
}
/** @brief Display Command Line Prompt
   @details This function displays a command line prompt at the bottom of the
   viewing window. It takes a string (s) as input and formats it to fit within
   the width of the pad. The prompt is displayed in reverse video (highlighted)
   to distinguish it from the regular content. The function also ensures that
   the prompt is properly aligned and does not exceed the maximum width of the
   pad. After displaying the prompt, it refreshes the window to show the updated
   content. This allows for interactive prompts to be shown to the user, such as
   search results or command feedback, while maintaining a clear and organized
   display. */
void cmd_line_prompt(View *view, char *s) {
    /// Display Command Line Prompt
    /// @param view is the current view data structure
    /// @param s is the prompt string
    char message_str[PAD_COLS + 1];
    int l;
    l = strnz__cpy(message_str, s, PAD_COLS);
    wmove(view->win, view->cmd_line, view->pmincol);
    if (l != 0) {
        wclrtoeol(view->win);
        wattron(view->win, WA_REVERSE);
        waddstr(view->win, " ");
        waddstr(view->win, message_str);
        waddstr(view->win, " ");
        wattroff(view->win, WA_REVERSE);
        waddstr(view->win, " ");
        wmove(view->win, view->cmd_line, view->pmincol + l + 2);
    }
    wrefresh(view->win);
}
/** @brief Remove File
   @details This function prompts the user to confirm the removal of the
   currently viewed file. If the user confirms by entering 'Y' or 'y', the
   function proceeds to remove the file from the filesystem. The prompt is
   displayed on the command line of the viewing window, and the user's input is
   captured to determine whether to proceed with the file removal. This allows
   users to easily delete files directly from the view interface while providing
   a confirmation step to prevent accidental deletions. */
void remove_file(View *view) {
    /// Remove File
    /// @param view is the current view data structure
    char c;
    if (view->f_at_end_remove) {
        wmove(view->win, view->cmd_line, 0);
        waddstr(view->win, "Remove File (Y or N)->");
        wclrtoeol(view->win);
        c = (char)xwgetch(view->win);
        waddch(view->win, (char)toupper(c));
        if (c == 'Y' || c == 'y')
            remove(view->cur_file_str);
    }
}
/** @brief Display View Help File
   @details This function displays the help file for the view feature. */
void view_display_help(Init *init) {
    int eargc;
    char *eargv[MAXARGS];
    View *view_save = init->view;
    init->view = NULL;
    zero_opt_args(init);
    eargv[0] = strdup("mview");
    eargv[1] = strdup(VIEW_HELP_FILE);
    eargv[2] = NULL;
    eargc = 2;
    parse_opt_args(init, eargc, eargv);
    init->lines = 40;
    init->cols = 54;
    init->begy = 0;
    init->begx = 0;
    strnz__cpy(init->title, "View Help", MAXLEN - 1);
    mview(init, eargc, eargv);
    init->view = view_save;
    init->view->f_redisplay_page = true;
}
