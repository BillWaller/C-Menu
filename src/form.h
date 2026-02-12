/** @file form.h
 *  @brief Form data structures, enums, types, end external declarations
 *  @author Bill Waller
 *  Copyright (c) 2025
 *  MIT License
 *  billxwaller@gmail.com
 *  @date 2026-02-09
 */

#ifndef _FORM_H
#define _FORM_H 1
#include "cm.h"

#ifndef _COMMON_H
typedef struct Init Init;
#endif

#define TBL_COLS 1024
#define MAXFIELDS 100
#define F_NOMETAS 1  /**< flag - no metacharacters allowed in field input */
#define F_NOTBLANK 2 /**< flag - field input cannot be blank */
#define F_NOECHO                                                               \
    4 /**< flag - do not echo field input on the screen, e.g. for password     \
         fields */

/** Form process types */
enum {
    P_CONTINUE =
        302,  /**< continue processing the form, e.g. after accepting a field */
    P_ACCEPT, /**< accept the form, e.g. after accepting a field and all fields
                 are valid */
    P_HELP, /**< display the help information for the form, e.g. after accepting
               a field and the user requests help */
    P_CANCEL, /**< cancel the form, e.g. after accepting a field and the user
                 cancels the form */
    P_REFUSE, /**< refuse the form, e.g. after accepting a field and the field
                 value is invalid */
    P_CALC,   /**< calculate the form, e.g. after accepting a field and
                 calculating the values of other fields based on the accepted
                 field value */
    P_EDIT,   /**< edit the form, e.g. after accepting a field and allowing the
                 user to edit the form fields */
    P_END     /**< end the form processing, e.g. after accepting a field and all
                 fields are valid and the form is accepted */
};

/** Form field formats */
enum FieldFormat {
    FF_STRING, /**< a string field format, e.g. for accepting a name or other
                  text input */
    FF_DECIMAL_INT, /**< a decimal integer field format, e.g. for accepting an
                       age or other numeric input */
    FF_HEX_INT, /**< a hexadecimal integer field format, e.g. for accepting a
                   color code or other hexadecimal input */
    FF_FLOAT,  /**< a floating point field format, e.g. for accepting a price or
                  other decimal input */
    FF_DOUBLE, /**< a double precision floating point field format, e.g. for
                  accepting a more precise decimal input */
    FF_CURRENCY, /**< a currency field format, e.g. for accepting a price or
                    other monetary input, formatted as a decimal number with two
                    decimal places and a currency symbol */
    FF_YYYYMMDD, /**< a date field format, e.g. for accepting a date input,
                    formatted as a string in the format "YYYY-MM-DD" */
    FF_HHMMSS,   /**< a time field format, e.g. for accepting a time input,
                    formatted as a string in the format "HH:MM:SS" */
    FF_APR,    /**< an APR field format, e.g. for accepting an annual percentage
                  rate input, formatted as a decimal number with two decimal places
                  and a percent symbol */
    FF_INVALID /**< an invalid field format, used for error handling and
                  validation purposes */
    /** Future Implementation of Additional Field Formats
     * FF_EMAIL - an email field format, e.g. for accepting an email address
     * input, formatted as a string that matches a regular expression for valid
     * email addresses
     * FF_PHONE - a phone number field format, e.g. for accepting a phone number
     * input, formatted as a string that matches a regular expression for valid
     * phone numbers
     * FF_URL - a URL field format, e.g. for accepting a URL input, formatted as
     * a string that matches a regular expression for valid
     */
};

/** ff_tbl - a table of field format strings used in the Form description
 *  file to identify field data types
 *  the index of the field format in this table corresponds to the FieldFormat
 *  enum values, e.g. ff_tbl[FF_STRING] = "string", ff_tbl[F_DECIMAL_INT] =
 *  "decimal_int", etc.
 */
extern char
    ff_tbl[][26]; /**< a table of field format strings used in the Form
                     description file to identify field data types, e.g.
                     "string", "decimal_int", "hex_int", etc. The index of the
                     field format in this table corresponds to the FieldFormat
                     enum values, e.g. ff_tbl[FF_STRING] = "string",
                     ff_tbl[FF_DECIMAL_INT] = "decimal_int", etc. This table is
                     used for parsing the form description file and determining
                     the field formats for each field in the form. */

/** @struct Text
   @brief structure for form fields */
typedef struct {
    int line; /**< the line number on the form window where this text string
                 should be displayed */
    int col;  /**< the column number on the form window where this text string
                 should be displayed */
    char str[SCR_COLS]; /**< the text string to be displayed on the form window
                         */
    int len; /**< the length of the text string, used for display and formatting
                purposes */
} Text;

/** @struct Field
   @brief structure for form fields */
typedef struct {
    int line; /**< the line number on the form window where this field should be
                 displayed */
    int col;  /**< the column number on the form window where this field should
                 be displayed */
    int len;  /**< the length of the field input area, used for display and
                 formatting purposes */
    int ff;   /**< the field format, represented as an integer corresponding to
                 the FieldFormat enum values, e.g. FF_STRING, FF_DECIMAL_INT, etc.
                 This is used for validating and formatting the field input values
                 according to their specified formats. */
    char input_s[MAXLEN]; /**< the input string for this field, used for storing
                             the user's input value for this field during form
                             processing */
    char accept_s[MAXLEN];  /**< the accepted string for this field, used for
                               storing the validated and accepted value for this
                               field after processing the user's input during
                               form processing */
    char display_s[MAXLEN]; /**< the display string for this field, used for
                               storing the formatted string that will be
                               displayed in the field input area on the form
                               window during form processing, based on the
                               accepted value and the field format */
    char filler_s[MAXLEN];  /**< the filler string for this field, used for
                               storing the string that will be used to fill the
                               field input area on the form window during form
                               processing, based on the field length and the fill
                               character specified in the form structure */
} Field;

/** @struct Form */
typedef struct {
    int fg_color; /**< the foreground color for the form window */
    int bg_color; /**< the background color for the form window */
    int bo_color; /**< the border color for the form window */
    int lines;    /**< the number of lines for the form window */
    int cols;     /**< the number of columns for the form windowi */
    int begy; /**< the screen line number for the upper left corner of the form
                 window */
    int begx; /**< the screen column number for the upper left corner of the
                 form window */
    WINDOW *win;           /**< ncurses window structure for form */
    WINDOW *box;           /**< ncurses window structure for form box border */
    char title[MAXLEN];    /**< the title of the form, displayed in the form box
                              border */
    char chyron_s[MAXLEN]; /**< the chyron string for the form, displayed in the
                              form box border or at the bottom of the form
                              window to provide instructions or information to
                              the user during form processing */
    FILE *in_fp; /**< input stream pointer, e.g. for reading from a file or pipe
                  */
    FILE *out_fp; /**< output stream pointer, e.g. for writing to a file or pipe
                   */
    int in_fd;  /**< input file descriptor, e.g. for reading from a file or pipe
                 */
    int out_fd; /**< output file descriptor, e.g. for writing to a file or pipe
                 */
    char mapp_spec[MAXLEN]; /**< the menu application description file spec */
    char provider_cmd
        [MAXLEN]; /**< the provider command, which can be executed in the
                     background to provide dynamic content for a program called
                     by the menu application. This command can be used to
                     generate or retrieve data that will be displayed on the
                     form or used for processing the form fields, allowing for
                     dynamic and interactive forms that can adapt to changing
                     data or user input during form processing. */
    char receiver_cmd
        [MAXLEN];     /**< the receiver command, which can be executed in the
                         background to process the output of a program called by the
                         menu application. This command can be used to handle the
                         results or output generated by a program that is executed
                         as part of the form processing, allowing for dynamic and
                         interactive forms that can respond to the results of
                         external programs or commands during form processing. */
    char cmd[MAXLEN]; /**< a command that can be executed in the foreground,
                         possibly taking control of the screen, by the menu
                         application, e.g. an editor. This command can be used
                         to allow the user to perform additional actions or
                         tasks as part of the form processing, such as editing a
                         file or running a script, providing a more interactive
                         and flexible user experience during form processing. */
    bool f_mapp_spec; /**< flag - mapp_spec verified */
    char help_spec
        [MAXLEN]; /**< the menu application help file spec, e.g. a qualified
                     path to a file containing help information for the menu
                     application and its associated forms. This file can be used
                     to provide context-sensitive help to the user during form
                     processing, allowing them to access relevant information
                     and guidance based on their current actions or the specific
                     form they are working with. */
    char in_spec[MAXLEN];  /**< the input file spec, e.g. a qualified path to a
                              file containing initial values for the form fields.
                              This file can be used to pre-populate the form
                              fields with existing data or default values during
                              form processing, allowing for a more efficient and
                              user-friendly experience when working with forms
                              that require input values. */
    char out_spec[MAXLEN]; /**< the output file spec, e.g. a qualified path to a
                              file where the accepted values for the form fields
                              will be written after form processing. This file
                              can be used to save or export the results of the
                              form processing, allowing for further use or
                              analysis of the accepted field values after the
                              form has been completed. */
    bool f_in_spec; /**< flag - in_spec verified, indicating that the input file
                       spec has been validated and is ready for use during form
                       processing */
    bool f_out_spec;  /**< flag - out_spec verified, indicating that the output
                         file spec has been validated and is ready for use during
                         form processing */
    bool f_in_pipe;   /**< flag - in_spec is a pipe, indicating that the input
                         file spec is a command that can be executed to provide
                         input data for the form fields during form processing,
                         allowing for dynamic and flexible input sources for the
                         form fields. */
    bool f_out_pipe;  /**< flag - out_spec is a pipe, indicating that the output
                         file spec is a command that can be executed to handle
                         the accepted field values for the form fields during
                         form processing, allowing for dynamic and flexible
                         output handling for the form results. */
    bool f_help_spec; /**< flag - help_spec verified, indicating that the help
                         file spec has been validated and is ready for use
                         during form processing */
    bool
        f_erase_remainder; /**< flag - if set, causes the data above the cursor
                              and to the right to be cleared when the user
                              presses the enter key, e.g. to prevent leftover
                              characters from previous field values from being
                              left on the screen when a shorter value is entered
                              in a field. This is essential for veteran keyboard
                              users, accountants and the like, who rarely even
                              look at the screen during data entry. */
    bool f_calculate; /**< flag - if set, Form presents an option to perform a
                         calculation on the data contained in the form fields,
                         e.g. to calculate the values of other fields based on
                         the accepted field values. This can be useful for forms
                         that require calculations or data processing based on
                         user input, allowing for dynamic and interactive forms
                         that can provide immediate feedback or results based on
                         the entered data during form processing. */

    bool f_query; /**< flag - if set, the action key label will be "query"
                     instead of "calculate", indicating that pressing the action
                     key will perform a query based on the accepted field
                     values, rather than a calculation. This can be useful for
                     forms that are designed for data entry and retrieval
                     purposes, allowing users to easily understand that pressing
                     the action key will execute a query to retrieve data based
                     on their input, rather than performing a calculation. This
                     flag can be used in conjunction with f_calculate to allow
                     for both calculation and querying functionality within the
                     same form, depending on the needs of the application. */

    bool f_stop_on_error; /**< flag - if set, causes the form processing to stop
                             if an error is encountered, e.g. if a field value
                             fails validation or if a command execution fails.
                             This can be useful for ensuring data integrity and
                             preventing further errors from occurring if an
                             issue is detected during form processing. */
    bool help; /**< flag - if set, indicates that the user has requested help
                  during form processing, e.g. by pressing a help key or button.
                  This can be used to trigger the display of context-sensitive
                  help information for the form, providing guidance and
                  assistance to the user based on their current actions or the
                  specific form they are working with. */
    bool f_provider_cmd; /**< flag - if set, indicates that the provider command
                            has been verified and is valid for execution. This
                            can be used to ensure that the provider command
                            specified in the form structure is valid and can be
                            executed successfully during form processing,
                            allowing for dynamic content generation or retrieval
                            as part of the form's functionality. */
    bool f_receiver_cmd; /**< flag - if set, indicates that the receiver command
                            has been verified and is valid for execution. This
                            can be used to ensure that the receiver command
                            specified in the form structure is valid and can be
                            executed successfully during form processing,
                            allowing for dynamic handling of the results or
                            output generated by external programs or commands as
                            part of the form's functionality. */
    bool f_cmd; /**< flag - if set, indicates that the command has been verified
                   and is valid for execution. This can be used to ensure that
                   the command specified in the form structure is valid and can
                   be executed successfully during form processing, allowing for
                   additional actions or tasks to be performed as part of the
                   form's functionality, such as editing a file or running a
                   script. */
    char brackets
        [3]; /**< the characters used to indicate field input areas in the form
                description file, e.g. "[]" for square brackets or "<>" for
                angle brackets. The first character is the left bracket and the
                second character is the right bracket. These characters may be
                left blank, especially if fill_char is used. The choice of
                brackets can affect the visual appearance of the form and how
                users perceive the input areas for each field, with some
                preferring the traditional look of brackets while others may
                prefer a cleaner look without them. The use of brackets can also
                help to visually separate the field input areas from other text
                on the form, making it easier for users to identify where they
                need to enter their input during form processing. */
    char fill_char[2]; /**< the character used to fill the field input areas in
                          the form, e.g. "_" for underscores or " " for spaces.
                          This character is used to visually indicate the input
                          area for each field in the form, and it can be used in
                          conjunction with brackets or on its own if brackets
                          are not used. Many feel that using fill characters
                          without brackets creates a cleaner and more modern
                          look for the form, while others prefer the traditional
                          look of brackets. The choice of whether to use
                          brackets, fill characters, or both is a matter of
                          personal preference and design style for the form. */
    int fidx;          /**< the index of the currently selected field, used for
                          highlighting and selection purposes during form processing.
                          This index corresponds to the position of the field in the
                          field array, allowing for easy access to the current field's
                          data and properties during form processing. The fidx can be
                          updated as the user navigates through the form fields,
                          providing visual feedback and allowing for dynamic interactions
                          based on the selected field. */
    int fcnt; /**< the number of fields in the form, used for iterating through
                 the field array and managing field data during form processing.
                 This count can be used to determine when the user has reached
                 the end of the fields or to validate that all required fields
                 have been completed before accepting the form. The fcnt can
                 also be used to dynamically allocate resources or manage memory
                 for the field data during form processing. */
    int didx; /**< an index to the array of text strings to be displayed on the
                 form window, used for iterating through the text array and
                 managing the display of text during form processing. This index
                 can be updated as needed to control which text strings are
                 currently being displayed on the form window, allowing for
                 dynamic updates to the form's content based on user actions or
                 other events during form processing. */
    int dcnt; /**< the number of text strings to be displayed on the form
                 window, used for iterating through the text array and managing
                 the display of text during form processing. This count can be
                 used to determine how many text strings need to be displayed on
                 the form window and to validate that all necessary text has
                 been provided for display during form processing. The dcnt can
                 also be used to dynamically allocate resources or manage memory
                 for the text data during form processing. */
    Text *text[MAXFIELDS]; /**< an array of pointers to the text structures for
                              the text strings to be displayed on the form
                              window, used for managing and displaying the text
                              content of the form during form processing. Each
                              pointer in this array corresponds to a Text
                              structure that contains the line, column, string,
                              and length information for a specific text string
                              to be displayed on the form window. This allows
                              for dynamic management of the text content on the
                              form, enabling updates and changes to the
                              displayed text based on user actions or other
                              events during form processing. */
    Field
        *field[MAXFIELDS]; /**< an array of pointers to the field structures for
                              the fields in this form, used for managing and
                              processing the form fields during form processing.
                              Each pointer in this array corresponds to a Field
                              structure that contains the line, column, length,
                              field format, input string, accepted string,
                              display string, and filler string information for
                              a specific field in the form. This allows for
                              dynamic management of the form fields, enabling
                              validation, formatting, and processing of user
                              input based on the specified field formats and
                              properties during form processing. */
} Form;
extern Form
    *form; /**< a pointer to the current form structure, used for managing and
              processing the form during form processing. This pointer can be
              updated to point to different form structures as needed, allowing
              for dynamic handling of multiple forms within the same application
              or session. The form pointer provides access to all the properties
              and data of the current form, enabling efficient management and
              processing of the form fields and text during user interactions
              and other events during form processing. */

extern int form_accept_field(Form *);
extern int form_display_field(Form *);
extern int form_display_field_n(Form *, int);
extern int form_open_win(Form *);
extern int form_enter_fields(Form *);
extern int form_read_description(Form *);
extern int form_fmt_field(Form *, char *s);
extern int form_desc_error(int, char *, char *);
extern void form_help(char *);
#endif
