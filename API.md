# C-Menu API Documentation

## Table of Contents

- [Overview](#overview)
- [Utility Functions](#utility-functions)
- [String Functions](#string-functions)
- [Chyron Functions](#chyron-functions)
- [Color Functions](#color-functions)

## Overview

C-Menu is a User Interface Builder. It allows developers to create and
manage UI components quickly and easily. In developing C-Menu, we focused
on modularity, ease of use, and flexibility. This documentation provides an
overview of the API, including its main features and how to use them.

Note 1: This documentation assumes familiarity with C programming and
basic concepts of user interface design.

Note 2: The C-Menu API is designed to work with the NCurses library for
terminal-based user interfaces. Ensure that you have NCurses installed and
properly configured in your development environment.

Note 3: This document is a work-in-progress and will be updated regularly.
Only a fraction of the API functions are documented here at present with many
more to be added.

===============================================================

## Utility Functions

---

### RTRIM

int rtrim(char \*str)

Removes trailing whitespace characters from the given string.

- **Parameters**:
  - `char \*str`: The string to be trimmed.
- **Returns**:
  - `int`: The new length of the trimmed string.

---

### TRIM

int trim(char \*str)

Removes leading and trailing whitespace characters from the given string.

- **Parameters**:
  - `char \*str`: The string to be trimmed.
- **Returns**:
  - `int`: The new length of the trimmed string.

---

### SSNPRINTF

ssnprintf(char \*str, size_t size, const char \*format, ...)

A safe version of snprintf that ensures the output string is null-terminated.

- **Parameters**:
  - `char \*str`: The buffer to write the formatted string to.
  - `size_t size`: The size of the buffer.
  - `const char \*format`: The format string.
  - `...`: Additional arguments to be formatted.
- **Returns**:
  - `int`: The number of characters written, excluding the null terminator.

---

### STR_TO_ARGS

str_to_args(char \*argv[], char \*arg_str, int max_args)

Splits a string into an array of arguments based on whitespace.

- **Parameters**:
  - `char \*argv[]`: The array to store the arguments.
  - `char \*arg_str`: The input string to be split.
  - `int max_args`: The maximum number of arguments to extract.
- **Returns**:
  - `int`: The number of arguments extracted.

Text surrounded by double quotes '"' will be treated as a single argument.

---

### STR_TO_LOWER

str_to_lower(char \*str)

Converts all characters in the string to lowercase.

- **Parameters**:
  - `char \*str`: The string to be converted.
- **Returns**:
  - `char \*`: A pointer to the converted string.

---

### STR_TO_UPPER

str_to_upper(char \*str)

Converts all characters in the string to uppercase.

- **Parameters**:
  - `char \*str`: The string to be converted.
- **Returns**:
  - `char \*`: A pointer to the converted string.

---

### STRNZ\_\_CPY

strnz\_\_cpy

Copies a string from source to destination with a specified maximum length.

- **Parameters**:
  - `char \*dest`: The destination buffer.
  - `const char \*src`: The source string.
  - `size_t n`: The maximum number of characters in the resulting string.
- **Returns**:
  - int: The length of the resulting string

strnz**cpy differs from strncpy in that it is limited, not by the
number of characters copied, but by the size of the destination buffer,
ensuring null-termination. With strnz**cpy, you can prevent a buffer
overrun by setting the third parameter to the size of the destination
buffer - 1, leaving space for the null terminator.

---

### STRNZ\_\_CAT

strnz\_\_cat

Concatenates a source string to a destination string with a specified maximum length.

- **Parameters**:
  - `char \*dest`: The destination buffer.
  - `const char \*src`: The source string.
  - `size_t n`: The maximum size of the destination buffer.
- **Returns**:
  - int: The length of the resulting string

strnz**cat differs from strncat in that it is limited, not by the number
of characters concatenated, but by the size of the destination buffer,
ensuring null-termination. With strnz**cat, you can prevent a buffer
overrun by setting the third parameter to the size of the destination
buffer - 1, leaving space for the null terminator.

---

### STRNZ

strnz(char \*str, int max_len)

Ensures that a string is null-terminated within a specified maximum length.
Terminates the string on encountering a line-feed ('\n') or
carriage-return ('\r').

- **Parameters**:
  - `char \*str`: The string to be checked.
  - `int max_len`: The maximum length of the string.
- **Returns**:
  - int: Length of the null-terminated string.

---

### STRNZ_DUP

strnz_dup(char \*str, int max_len)

Duplicates a string up to a specified maximum length, a line-feed
('\n'), or a carriage-return ('\r'), ensuring null-termination.
Because strnz_dup allocates memory, it is up to the caller to free
the memory when it is no longer needed.

- **Parameters**:
  - `char \*str`: The string to be duplicated.
  - `int max_len`: The maximum length of the string.
- **Returns**:
  - char \*: A pointer to the newly allocated duplicated string.

---

### STR_SUBC

void str_subc(char \*d, char \*s, char ReplaceChr, char \*Withstr, int l)

Replaces all occurrences of a specified character in a string with another string.

- **Parameters**:
  - `char \*d`: The destination buffer.
  - `char \*s`: The source string.
  - `char ReplaceChr`: The character to be replaced.
  - `char \*Withstr`: The string to replace the character with.
  - `int l`: The maximum length of the destination string.

---

### STRIPZ_QUOTES

bool stripz_quotes(char \*s)

Removes surrounding double quotes from a string if they exist.

- **Parameters**:
  - `char \*s`: The string to be processed.
- **Returns**:
  - `bool`: True if quotes were removed, false otherwise.

---

### CHREP

chrep(char \*s, char old_chr, char new_chr)

Replaces all occurrences of a specified character in a string with another character.

- **Parameters**:
  - `char \*s`: The string to be processed.
  - `char old_chr`: The character to be replaced.
  - `char new_chr`: The character to replace with.
- **Returns**:
  - void: no return

---

### FILE_SPEC_PATH

file_spec_path(char \*fp, char \*fs)

Extracts the path component from a file specification and places it in fp.
It is the caller's responsibility to ensure that fp has enough space to
hold the path.

- **Parameters**:
  - `char \*fp`: The buffer to store the extracted path.
  - `char \*fs`: The file specification string.
- **Returns**:
  - `void`: no return

Unlike the POSIX implementation of basename(), this function does not
modify the input string. Also, a character array may be used as the first
argument, obviating the need for dynamic memory allocation.

---

### FILE_SPEC_NAME

file_spec_name(char \*fn, char \*fs)

Extracts the file name component from a file specification and places it in
fn. It is the caller's responsibility to ensure that fn has enough space to
hold the file name.

- **Parameters**:
  - `char \*fn`: The buffer to store the extracted file name.
  - `char \*fs`: The file specification string.
- **Returns**:
  - `void`: no return

Unlike the POSIX implementation of dirname(), this function does not modify
the input string. Also, a character array may be used as the first
argument, obviating the need for dynamic memory allocation. There is no GNU
version of dirname().

---

### STR_TO_BOOL

bool str_to_bool(const char \*)

Converts a string representation of a boolean value to its corresponding
boolean type based on the first character of the string.

- **Parameters**:
  - `const char \*`: The string to be converted.
- **Returns**:
  - `bool`: The boolean value represented by the string. Returns true for
    'Y', '1' and false for 'f', 'F', 'n', 'N', '0'. For any
    other character, the function returns false.

---

### EXPAND_TILDE

bool expand_tilde(char \*out_buf, const char \*in_buf, size_t buf_size)

Expands a tilde ('~') at the beginning of a file path to the user's home directory.

- **Parameters**:
  - `char \*out_buf`: The buffer to store the expanded file path.
  - `const char \*in_buf`: The input file path that may contain a tilde.
  - `size_t buf_size`: The size of the output buffer.
- **Returns**:
  - `bool`: True if the expansion was successful, false otherwise. If the
    input path does not start with a tilde, the function copies the input
    path to the output buffer without modification.

---

### TRIM_PATH

bool trim_path(char \*char) {

Trims redundant slashes and resolves relative path components ('.' and
'..') in a file path.

- **Parameters**:
  - `char \*path`: The file path to be trimmed and resolved.
- **Returns**:
  - `bool`: True if the path was successfully trimmed and resolved, false otherwise.
    The function modifies the input path in place.
    It is up to the caller to ensure that the input path is valid and
    writable, and that the receiving string pointer has enough space to hold
    the modified path.

---

### TRIM_EXT

bool trim_ext(char \*buf, char \*filename)

Removes the file extension from a given filename.

- **Parameters**:
  - `char \*buf`: The buffer to store the filename without the extension.
  - `char \*filename`: The original filename with the extension.
- **Returns**:
  - `bool`: True if the extension was successfully removed, false
    otherwise. The function modifies the input filename in place.
    It is up to the caller to ensure that the input filename is valid and
    writable, and that the receiving string pointer has enough space to hold
    the modified filename.

---

### BASE_NAME

bool base_name(char \*buf, const char \*filename)

Extracts the base name (file name without path) from a given file path.

- **Parameters**:
  - `char \*buf`: The buffer to store the base name.
  - `const char \*filename`: The original file path.
- **Returns**:
  - `bool`: True if the base name was successfully extracted, false otherwise.

The function leaves the input filename intact and copies the
basename to buf.

It is up to the caller to ensure that the input filename is valid,
and that the receiving string pointer has enough space to hold the
base name.

---

### DIR_NAME

bool dir_name(char \*buf, char \*path)

Extracts the directory name (path without file name) from a given file path.

- **Parameters**:
  - `char \*buf`: The buffer to store the directory name.
  - `char \*path`: The original file path.
- **Returns**:
  - `bool`: True if the directory name was successfully extracted,
    false otherwise. The function leaves the input path intact and
    copies the dirname to buf. It is up to the caller to ensure that
    the input path is valid, and that the receiving string pointer has
    enough space to hold the directory name.

---

### VERIFY_DIR

bool verify_dir(char \*spec, int imode)

Verifies the existence of a directory specified by the given path.

- **Parameters**:
  - `char \*spec`: The directory path to be verified.
  - `int imode`: The mode of verification (e.g., existence, readability, writability).
- **Returns**:
  - `bool`: True if the directory exists and meets the specified mode,
    false otherwise. The function checks if the directory specified by
    spec exists and meets the criteria defined by imode. It is up to
    the caller to ensure that the input path is valid.

```c
        modes: R_OK, W_OK, X_OK, F_OK, see access(2) for details.
        extended modes: S_WCOK - write or create OK
        S_QUIET - Don't complain about errors
```

---

### VERIFY_FILE

bool verify_file(char \*in_spec, int imode)

Verifies the existence of or ability to create a file specified by the given path.

- **Parameters**:
  - `char \*in_spec`: The file path to be verified.
  - `int imode`: The mode: see below
- **Returns**:
  - `bool`: True if the file exists and meets the specified mode, or
    can be created if S_WCOK is specified, false otherwise.

The function checks if the file specified exists and meets
the criteria defined by imode, or if it can be created when S_WCOK
is specified.

It is up to the caller to ensure that the input path is valid.

---

### LOCATE_FILE_IN_PATH

bool locate_file_in_path(char \*file_spec, char \*file_name)

Searches for a file in the system's PATH environment variable and
returns its full path if found.

- **Parameters**:
  - `char \*file_spec`: The buffer to store the full path of the found file.
  - `char \*file_name`: The name of the file to be located.
- **Returns**:
  - `bool`: True if the file was found in the PATH, false otherwise.

The function searches through the directories listed in the PATH
environment variable for the specified file name. It is up to the
caller to ensure that the receiving string pointer has enough
space to hold the full path of the found file.

---

### LIST_FILES

bool list_files(char \*dir, char \*regexp, bool f_recurse)

Lists files in a specified directory that match a given regular
expression, with an option to recurse into subdirectories.

- **Parameters**:
  - `char \*dir`: The directory to search for files.
  - `char \*regexp`: The regular expression to match file names against.
  - `bool f_recurse`: A flag indicating whether to recurse into subdirectories.
- **Returns**:
  - `list_t \*`: A pointer to a list of matching file names.
    The function searches the specified directory for files that match
    the provided regular expression. If f_recurse is true, the function
    will also search in subdirectories. It is up to the caller to free
    the returned list when it is no longer needed.

---

### LF_FIND_DIRS

bool lf_find_dirs(char \*dir, char \*re)

Finds directories within a specified directory that match a given regular expression.

- **Parameters**:
  - `char \*dir`: The directory ato search for subdirectories.
  - `char \*re`: The regular expression to match directory names against.

- **Returns**: true if a matching file is found or false otherwise

The function searches the specified directory for subdirectories for
files that match the provided regular expression.

---

### lf_find_files(char \*dir, char \*re)

Finds files within a specified directory that match a given regular expression.

- **Parameters**:
  - `char \*dir`: The directory to search for files.
  - `char \*re`: The regular expression to match file names against.
- **Returns**: true if a matching file is found or false otherwise.

  The function searches the specified directory for files that match
  the provided regular expression.

---

### CANONICALIZE_FILE_SPEC

int canonicalize_file_spec(char \*spec)

Removes quotes and trims at first space.

- **Parameters**:
  - `char \*spec`: The file specification to be canonicalized.
- **Returns**:
  - void: length of resulting file specification string

The function modifies the input file specification in place.

---

### REP_SUBSTRING

char \*rep_substring(const char \*org_s, const char \*tgt_s, const char \*rep_s)

Replaces all occurrences of a target substring within an original
string with a replacement substring.

- **Parameters**:
  - `const char \*org_s`: The original string.
  - `const char \*tgt_s`: The target substring to be replaced.
  - `const char \*rep_s`: The replacement substring.
- **Returns**:
  - `char \*`: A pointer to the newly allocated string with
    replacements made.

        The function allocates memory for the new string, which must be
        freed by the caller when no longer needed.

===============================================================

## String functions

---

### STRING STRUCT

```c

typedef struct {
char *str;     // Pointer to the string data
size_t length;  // Length of the string
} String;
```

Represents a dynamic string with its length.

---

### TO_STRING

String to_string(const char \*s)

Creates a new String object from a C-style string.

- **Parameters**:
  - `const char \*s`: The C-style string to be converted.
- **Returns**:
  - `String`: The newly created String object.

---

### MK_STRING

String mk_string(size_t l)

Creates a new String object with a specified length.

- **Parameters**:
  - `size_t l`: The length of the string to be created.
  - char \*s: The string data to be assigned to the String object.
- **Returns**:
  - `String`: The newly created String object.

if l is 0, str is set to NULL.

---

### FREE_STRING

free_string(String s)

Frees the memory allocated for a String object.

- **Parameters**:
  - `String s`: The String object to be freed.
- **Returns**:
  - `void`: no return

---

### STRING_CPY

size_t string_cpy(String \*d, const char \*s)

Copies a C-style string to a String object.

- **Parameters**:
  - `String \*d`: Pointer to the destination String object.
  - `const char \*s`: The source C-style string to be copied.
- **Returns**:
  - `size_t`: length of resulting string

If the source string is longer than the destination String's
current length, the destination String's str pointer is reallocated
to accommodate the new string.

---

### STRING_CAT

size_t string_cat(String \*d, const char \*s)

Concatenates a C-style string to a String object.

- **Parameters**:
  - `String \*d`: Pointer to the destination String object.
  - `const char \*s`: The source C-style string to be concatenated.
- **Returns**:
  - `size_t`: length of resulting string

If the concatenation would result in a string longer than the
destination String's length, the destination String's str pointer
is reallocated.

---

### STRING_NCAT

size_t string_ncat(String \*d, const char \*s, size_t n)

Concatenates up to n characters of a C-style string to a String object.
Concatenates characters from a source String object to a destination
String.

- **Parameters**:
  - `String \*d`: Pointer to the destination String object.
  - `const char \*s`: The source C-style string to be concatenated.
  - `size_t n`: The maximum number of characters to concatenate.
- **Returns**:
  - `size_t`: length of resulting string

If the resulting string would be longer than the destination String's
length, the destination String's str pointer is reallocated.

---

### STRING_NCPY

size_t string_ncpy(String \*dest, const String \*src, size_t n)

Copies up to "n" characters from a C-style string to a destination
String object.

- **Parameters**:
  - `String \*dest`: Pointer to the destination String object.
  - `const String \*src`: Pointer to the source String object.
  - `size_t n`: The maximum number of characters to copy.
- **Returns**:
  - `size_t`: length of resulting string

If the resulting string would be longer than the destination String's
length, the destination String's str pointer is reallocated.

===============================================================

## Chyron Functions

---

### Chyron Overview

Chyrons are text overlays used in video production to display information
such as names, titles, or other relevant data. The C-Menu API provides
functions to create, manage, and render chyrons in a user interface. Though
not exactly like they chyrons you see on TV news broadcasts, they serve a similar
purpose in providing on-screen information in a banner across the bottom of
the screen and present options, in the form of function keys to the user.
The Function Key command can be selected by pressing the indicated F Key or
clicking on the chyron within the vertical bars separating the F Keys.

![C-Menu Pick Chyron](screenshots/Pick.png)

### KEY_CMD_TBL

![C-Menu key_cmd_tbl](screenshots/cmd_tbl.png)

As you can see in the table above, the key_cmd_tbl structure keeps track of
the function key commands and their positions within the chyron. This allows
the program to determine which function key was selected based on the x-coordinate
of a mouse click.

Usually, not all function keys are used in the chyron. This is because the
chyron is designed to be flexible and can accommodate different numbers of
function keys depending on the application's needs. The unused function keys
are not displayed in the chyron.

You may have also noticed, Function Keys are just one way to use the chyron
system. The keycode can be any integer value you choose, allowing for
custom commands or actions. You don't have to use a key at all. You could
populate the chyron with unicode glyphs or other symbols to create a custom
menu bar.

---

### SET_FKEY

void set_fkey(int k, char \*s)

Assigns a command string to a function key where k is the function key number,
currently 1 through 14, and s is the command description associated with the
function key. For example, to insert "F5 Calculate" into the chyron, you would
call

```c
setkey(5, "Calculate");
```

---

### UNSET_FKEY

void unset_fkey(int k)

Removes a function key command from the chyron. For example, to remove the
command associated with function key 5, you would call

```c
unset_fkey(5);
```

---

### CHYRON_MK

int chyron_mk(key_cmd_tbl *fc, char *s)

Creates a chyron with function key commands, keeping track of the function key
coordinates within the chyron so that a mouse click can be mapped to the appropriate
function key command. The function key table pointer fc is passed to the chyron
so that when a function key is selected, the corresponding command string can be
retrieved from the table. The string s is the chyron text, which contains the
function key commands to be displayed on the bottom of the screen.

chyron_mk returns the length of the chyron string to be displayed, and position
information is stored in the structure, key_cmd_tbl.

---

### GET_CHYRON_KEY

int get_chyron_key(key_cmd_tbl \*fc, int x)

Determines which function key was selected based on the x-coordinate of a mouse
click. get_chyron_key returns the number of the function key clicked.

```c

cmdkey = get_chyron_key(&fkey_table, mouse_x);

---

bool is_set_fkey(int k)

Checks if a function key command is set in the chyron. The use case is to
determine if a key_cmd_tbl element is already in use so that you don't
accidentally overwrite it with a new assignment.

- **Parameters**:
  - `int k`: The function key number to check.
- **Returns**:
  - `bool`: True if the function key command is set, false otherwise.


```

===============================================================

## Color Functions

Note 1: These functions require NCurses to be initialized with color support.

Note 2: NCurses uses color pairs to manage foreground and background colors. A
color pair is a combination of a foreground color and a background color. Each
color pair is assigned a unique index, which is used to apply the colors to text
and other UI elements.

Note 3: The maximum number of colors and color pairs is determined by the terminal
emulator and NCurses configuration. The constants MAX_COLORS and MAX_COLOR_PAIRS
define these limits.

Note 4: RGB is a structure representing a color in terms of its red, green,
and blue components. The RGB structure is defined as follows:

```c
typedef struct {
    unsigned char r; // Red component (0-255)
    unsigned char g; // Green component (0-255)
    unsigned char b; // Blue component (0-255)
} RGB;
```

Note 5: NCurses uses a 0-1000 scale for RGB values, so the RGB values
must be converted from the standard 0-255 scale to the 0-1000 scale
before being used with NCurses functions.

---

### GET_CLR_PAIR

int get_clr_pair(int fg, int bg)

Retrieves a color pair index for the specified foreground and background
colors, or if the color pair doesn't exist, creates it. If the maximum number
of colors, MAX_COLOR_PAIRS, is reached, ERR is returned. This function allows
NCurses to manage color pairs efficiently by reusing existing pairs when possible.

- **Parameters**:
  - `int fg`: The foreground color index.
  - `int bg`: The background color index.
- **Returns**:
  - `int`: The color pair index, or ERR if the maximum number of color pairs
    has been reached.

---

### GET_CLR

int get_clr(RGB rgb)

Retrieves the color index for the specified RGB color, or if the color doesn't exist,
creates it. If the maximum number of colors, MAX_COLORS, is reached, ERR is returned.
This function allows NCurses to manage colors efficiently by reusing existing colors
when possible. Choose from 16,777,216 if your terminal supports it.

- **Parameters**:
  - `RGB rgb`: The RGB color to be retrieved or created.
- **Returns**:
  - `int`: The color index, or ERR if the maximum number of colors has been
    reached.

---

### RGB_TO_XTERM256_IDX

int rgb_to_xterm256_idx(RGB rgb)

This function maps the RGB color to the nearest color in the xterm-256
palette, which consists of 256 colors.

- **Parameters**:
  - `RGB rgb`: The RGB color to be converted.
- **Returns**:
  - `int`: The xterm-256 color index corresponding to the given RGB color.

---

### XTERM256_IDX_TO_RGB

RGB xterm256_idx_to_rgb(int idx)

Converts an xterm-256 color index to its corresponding RGB color.

- **Parameters**:
  - `int idx`: The xterm-256 color index to be converted.
- **Returns**:
  - `RGB`: The RGB color corresponding to the given xterm-256 color index.

---

### APPLY_GAMMA

void apply_gama(RGB \*rgb)

Applies gamma correction to the given RGB color. Gamma correction adjusts the
brightness of colors to account for the non-linear way humans perceive light and
color. C-Menu View uses a default gamma value of 2.2 for correction, but the
user can modify this value in the .minitrc configuration.

- **Parameters**:
  - `RGB *rgb`: Pointer to the RGB color to be corrected.
- **Returns**:
  - `void`: no return

---

### INIT_CLR_PALETTE

void init_clr_palette(Init \*init)

Initializes an xterm 256 color palette as a starting point. This works well
in practice because most terminal emulators support at least 256 colors, and
many programs use these colors. When the get_clr() function above is called, it
checks this table first, and if a match is found, it returns the corresponding
color index. If the color requested is not found in the palette, get_clr()
creates a new color entry.

NCurses assigns names to the first 16 colors (the EGA/ANSI palette), but these
colors can be redefined in C-Menu's configuration, .minitrc, using RGB values.
init_clr_palette() any of these named colors with the RGB values specified
in the configuration file.

- **Parameters**:
  - `Init *init`: Pointer to the initialization structure containing configuration
    data.
- **Returns**:
  - `void`: no return

---

### INIT_HEX_COLOR

void init_hex_color(int idx, char \*s)

The function converts the hexadecimal color string to an RGB color and
initializes the color at the specified index using the RGB values.

- **Parameters**:
  - `int idx`: The index at which to initialize the color.
  - `char *s`: The hexadecimal color string (e.g., "#RRGGBB").
- **Returns**:
  - `void`: no return

---

### HEX_CLR_STR_TO_RGB

RGB hex_clr_str_to_rgb()

Converts a hexadecimal color string to an RGB color.

---
