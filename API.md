# C-Menu API Documentation

## Overview

C-Menu is a User Interface Builder. It allows developers to create and manage UI components quickly and easily. In developing C-Menu, we focused on modularity, ease of use, and flexibility. This documentation provides an overview of the API, including its main features and how to use them.

## Utility Functions

### int rtrim(char \*str)

Removes trailing whitespace characters from the given string.

- **Parameters**:
  - `char *str`: The string to be trimmed.
- **Returns**:
  - `int`: The new length of the trimmed string.

### int trim(char \*str)

Removes leading and trailing whitespace characters from the given string.

- **Parameters**:
  - `char *str`: The string to be trimmed.
- **Returns**:
  - `int`: The new length of the trimmed string.

### ssnprintf(char *str, size_t size, const char *format, ...)

A safe version of snprintf that ensures the output string is null-terminated.

- **Parameters**:
  - `char *str`: The buffer to write the formatted string to.
  - `size_t size`: The size of the buffer.
  - `const char *format`: The format string.
  - `...`: Additional arguments to be formatted.
- **Returns**:
  - `int`: The number of characters written, excluding the null terminator.

### str_to_args(char *argv[], char *arg_str, int max_args)

Splits a string into an array of arguments based on whitespace.

- **Parameters**:
  - `char *argv[]`: The array to store the arguments.
  - `char *arg_str`: The input string to be split.
  - `int max_args`: The maximum number of arguments to extract.
- **Returns**:
  - `int`: The number of arguments extracted.

Text surrounded by double quotes '"' will be treated as a single argument.

### str_to_lower(char \*str)

Converts all characters in the string to lowercase.

- **Parameters**:
  - `char *str`: The string to be converted.
- **Returns**:
  - `char *`: A pointer to the converted string.

### str_to_upper(char \*str)

Converts all characters in the string to uppercase.

- **Parameters**:
  - `char *str`: The string to be converted.
- **Returns**:
  - `char *`: A pointer to the converted string.

### strnz\_\_cpy

Copies a string from source to destination with a specified maximum length.

- **Parameters**:
  - `char *dest`: The destination buffer.
  - `const char *src`: The source string.
  - `size_t n`: The maximum number of characters in the resulting string.
- **Returns**:
  - int: The length of the resulting string

strnz**cpy differs from strncpy in that it is limited, not by the number of characters copied, but by the size of the destination buffer, ensuring null-termination. With strnz**cpy, you can prevent a buffer overrun by setting the third parameter to the size of the destination buffer - 1, leaving space for the null terminator.

### strnz\_\_cat

Concatenates a source string to a destination string with a specified maximum length.

- **Parameters**:
  - `char *dest`: The destination buffer.
  - `const char *src`: The source string.
  - `size_t n`: The maximum size of the destination buffer.
- **Returns**:
  - int: The length of the resulting string

strnz**cat differs from strncat in that it is limited, not by the number of characters concatenated, but by the size of the destination buffer, ensuring null-termination. With strnz**cat, you can prevent a buffer overrun by setting the third parameter to the size of the destination buffer - 1, leaving space for the null terminator.

### strnz(char \*str, int max_len)

Ensures that a string is null-terminated within a specified maximum length. Terminates the string on encountering a line-feed ('\n') or carriage-return ('\r').

- **Parameters**:
  - `char *str`: The string to be checked.
  - `int max_len`: The maximum length of the string.
- **Returns**:
  - int: Length of the null-terminated string.

### strnz_dup(char \*str, int max_len)

Duplicates a string up to a specified maximum length, a line-feed ('\n'), or a carriage-return ('\r'), ensuring null-termination. Because strnz_dup allocates memory, it is up to the caller to free the memory when it is no longer needed.

- **Parameters**:
  - `char *str`: The string to be duplicated.
  - `int max_len`: The maximum length of the string.
- **Returns**:
  - char \*: A pointer to the newly allocated duplicated string.

### void str_subc(char *d, char *s, char ReplaceChr, char \*Withstr, int l)

Replaces all occurrences of a specified character in a string with another string.

- **Parameters**:
  - `char *d`: The destination buffer.
  - `char *s`: The source string.
  - `char ReplaceChr`: The character to be replaced.
  - `char *Withstr`: The string to replace the character with.
  - `int l`: The maximum length of the destination string.

### bool stripz_quotes(char \*s)

Removes surrounding double quotes from a string if they exist.

- **Parameters**:
  - `char *s`: The string to be processed.
- **Returns**:
  - `bool`: True if quotes were removed, false otherwise.

### chrep(char \*s, char old_chr, char new_chr)

Replaces all occurrences of a specified character in a string with another character.

- **Parameters**:
  - `char *s`: The string to be processed.
  - `char old_chr`: The character to be replaced.
  - `char new_chr`: The character to replace with.
- **Returns**:
  - void: no return

### file_spec_path(char *fp, char *fs)

Extracts the path component from a file specification and places it in fp. It is the caller's responsibility to ensure that fp has enough space to hold the path.

- **Parameters**:
  - `char *fp`: The buffer to store the extracted path.
  - `char *fs`: The file specification string.
- **Returns**:
  - `void`: no return

Unlike the POSIX implementation of basename(), this function does not modify the input string. Also, a character array may be used as the first argument, obviating the need for dynamic memory allocation.

### file_spec_name(char *fn, char *fs)

Extracts the file name component from a file specification and places it in fn. It is the caller's responsibility to ensure that fn has enough space to hold the file name.

- **Parameters**:
  - `char *fn`: The buffer to store the extracted file name.
  - `char *fs`: The file specification string.
- **Returns**:
  - `void`: no return

Unlike the POSIX implementation of dirname(), this function does not modify the input string. Also, a character array may be used as the first argument, obviating the need for dynamic memory allocation. There is no GNU version of dirname().

### bool str_to_bool(const char \*);

Converts a string representation of a boolean value to its corresponding boolean type
based on the first character of the string.

- **Parameters**:
  - `const char *`: The string to be converted.
- **Returns**:
  - `bool`: The boolean value represented by the string. Returns true for 't',  
    'T', 'y', 'Y', '1' and false for 'f', 'F', 'n', 'N', '0'. For any other character,  
    the function returns false.

### bool expand_tilde(char *out_buf, const char *in_buf, size_t buf_size)

Expands a tilde ('~') at the beginning of a file path to the user's home directory.

- **Parameters**:
  - `char *out_buf`: The buffer to store the expanded file path.
  - `const char *in_buf`: The input file path that may contain a tilde.
  - `size_t buf_size`: The size of the output buffer.
- **Returns**:
  - `bool`: True if the expansion was successful, false otherwise. If the input
    path does not start with a tilde, the function copies the input path to the output
    buffer without modification.

### bool trim_path(char \*char) {

Trims redundant slashes and resolves relative path components ('.' and '..') in a file path.

- **Parameters**:
  - `char *path`: The file path to be trimmed and resolved.
- **Returns**:
  - `bool`: True if the path was successfully trimmed and resolved, false otherwise.
    The function modifies the input path in place.
    It is up to the caller to ensure that the input path is valid and writable, and
    that the receiving string pointer has enough space to hold the modified path.

### bool trim_ext(char *buf, char *filename)

Removes the file extension from a given filename.

- **Parameters**:
  - `char *buf`: The buffer to store the filename without the extension.
  - `char *filename`: The original filename with the extension.
- **Returns**:
  - `bool`: True if the extension was successfully removed, false otherwise.
    The function modifies the input filename in place.
    It is up to the caller to ensure that the input filename is valid and writable, and
    that the receiving string pointer has enough space to hold the modified filename.

### base_name(char *buf, const char *filename)

Extracts the base name (file name without path) from a given file path.

- **Parameters**:
  - `char *buf`: The buffer to store the base name.
  - `const char *filename`: The original file path.
- **Returns**:
  - `bool`: True if the base name was successfully extracted, false otherwise.

The function leaves the input filename intact and copies the basename to buf.

It is up to the caller to ensure that the input filename is valid, and
that the receiving string pointer has enough space to hold the base name.

### bool dir_name(char *buf, char *path)

Extracts the directory name (path without file name) from a given file path.

- **Parameters**:
  - `char *buf`: The buffer to store the directory name.
  - `char *path`: The original file path.
- **Returns**:
  - `bool`: True if the directory name was successfully extracted, false otherwise.
    The function leaves the input path intact and copies the dirname to buf.
    It is up to the caller to ensure that the input path is valid, and
    that the receiving string pointer has enough space to hold the directory name.

### bool verify_dir(char \*spec, int imode)

Verifies the existence of a directory specified by the given path.

- **Parameters**:
  - `char *spec`: The directory path to be verified.
  - `int imode`: The mode of verification (e.g., existence, readability, writability).
- **Returns**:
  - `bool`: True if the directory exists and meets the specified mode, false otherwise.
    The function checks if the directory specified by spec exists and meets the criteria defined by imode.
    It is up to the caller to ensure that the input path is valid.

modes: R_OK, W_OK, X_OK, F_OK, see access(2) for details.
extended modes: S_WCOK - write or create OK
S_QUIET - Don't complain about errors

### bool verify_file(char \*in_spec, int imode)

Verifies the existence of or ability to create a file specified by the given path.

- **Parameters**:
  - `char *in_spec`: The file path to be verified.
  - `int imode`: The mode: see below
- **Returns**:
  - `bool`: True if the file exists and meets the specified mode, or can be created if S_WCOK is specified, false otherwise.
    The function checks if the file specified exists and meets the criteria defined by imode, or if it can be created when S_WCOK is specified.
    It is up to the caller to ensure that the input path is valid.

### bool locate_file_in_path(char *file_spec, char *file_name)

Searches for a file in the system's PATH environment variable and returns its full path if found.

- **Parameters**:
  - `char *file_spec`: The buffer to store the full path of the found file.
  - `char *file_name`: The name of the file to be located.
- **Returns**:
  - `bool`: True if the file was found in the PATH, false otherwise.
    The function searches through the directories listed in the PATH environment variable for the specified file name.
    It is up to the caller to ensure that the receiving string pointer has enough space to hold the full path of the found file.

### list_files(char *dir, char *regexp, bool f_recurse)

Lists files in a specified directory that match a given regular expression, with an option to recurse into subdirectories.

- **Parameters**:
  - `char *dir`: The directory to search for files.
  - `char *regexp`: The regular expression to match file names against.
  - `bool f_recurse`: A flag indicating whether to recurse into subdirectories.
- **Returns**:
  - `list_t *`: A pointer to a list of matching file names.
    The function searches the specified directory for files that match the provided regular expression.
    If f_recurse is true, the function will also search in subdirectories.
    It is up to the caller to free the returned list when it is no longer needed.

### lf_find_dirs(char *dir, char *re)

Finds directories within a specified directory that match a given regular expression.

- **Parameters**:
  - `char *dir`: The directory ato search for subdirectories.
  - `char *re`: The regular expression to match directory names against.

- **Returns**: true if a matching file is found or false otherwise

The function searches the specified directory for subdirectories for files that match the provided regular expression.

### lf_find_files(char *dir, char *re)

Finds files within a specified directory that match a given regular expression.

- **Parameters**:
  - `char *dir`: The directory to search for files.
  - `char *re`: The regular expression to match file names against.
- **Returns**: true if a matching file is found or false otherwise
  The function searches the specified directory for files that match the provided regular expression.

### canonicalize_file_spec(char \*spec)

Removes quotes and trims at first space.

- **Parameters**:
  - `char *spec`: The file specification to be canonicalized.
- **Returns**:
  - void: length of resulting file specification string

The function modifies the input file specification in place.

### rep_substring(const char *org_s, const char *tgt_s, const char \*rep_s)

Replaces all occurrences of a target substring within an original string with a replacement substring.

- **Parameters**:
  - `const char *org_s`: The original string.
  - `const char *tgt_s`: The target substring to be replaced.
  - `const char *rep_s`: The replacement substring.
- **Returns**:
  - `char *`: A pointer to the newly allocated string with replacements made.
    The function allocates memory for the new string, which must be freed by the caller when no longer needed.

## String functions

### String struct

```c
typedef struct {
    char *str;      // Pointer to the string data
    size_t length;  // Length of the string
} String;
```

Represents a dynamic string with its length.

### String to_string(const char \*s)

Creates a new String object from a C-style string.

- **Parameters**:
  - `const char *s`: The C-style string to be converted.
- **Returns**:
  - `String`: The newly created String object.

### String mk_string(size_t l)

Creates a new String object with a specified length.

- **Parameters**:
  - `size_t l`: The length of the string to be created.
  - char \*s: The string data to be assigned to the String object.
- **Returns**:
  - `String`: The newly created String object.

if l is 0, str is set to NULL.

### free_string(String s)

Frees the memory allocated for a String object.

- **Parameters**:
  - `String s`: The String object to be freed.
- **Returns**:
  - `void`: no return

### string_cpy(String *d, const char *s)

Copies a C-style string into a String object.

- **Parameters**:
  - `String *d`: Pointer to the destination String object.
  - `const char *s`: The source C-style string to be copied.
- **Returns**:
  - `void`: no return

### string_cat(String *d, const char *s)

Concatenates a C-style string to a String object.

- **Parameters**:
  - `String *d`: Pointer to the destination String object.
  - `const char *s`: The source C-style string to be concatenated.
- **Returns**:
  - `void`: no return

### string_ncat(String *d, const char *s, size_t n)

Concatenates up to n characters of a C-style string to a String object.

- **Parameters**:
  - `String *d`: Pointer to the destination String object.
  - `const char *s`: The source C-style string to be concatenated.
  - `size_t n`: The maximum number of characters to concatenate.
- **Returns**:
  - `void`: no return

### string_ncpy(String *dest, const String *src, size_t n)

Copies up to n characters from a source String object to a destination String object.

- **Parameters**:
  - `String *dest`: Pointer to the destination String object.
  - `const String *src`: Pointer to the source String object.
  - `size_t n`: The maximum number of characters to copy.
- **Returns**:
  - `void`: no return
