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

#### int trim(char \*str)

Removes leading and trailing whitespace characters from the given string.

- **Parameters**:
  - `char *str`: The string to be trimmed.
- **Returns**:
  - `int`: The new length of the trimmed string.

#### ssnprintf(char *str, size_t size, const char *format, ...)

A safe version of snprintf that ensures the output string is null-terminated.

- **Parameters**:
  - `char *str`: The buffer to write the formatted string to.
  - `size_t size`: The size of the buffer.
  - `const char *format`: The format string.
  - `...`: Additional arguments to be formatted.
- **Returns**:
  - `int`: The number of characters written, excluding the null terminator.

#### str_to_args(char *argv[], char *arg_str, int max_args)

Splits a string into an array of arguments based on whitespace.

- **Parameters**:
  - `char *argv[]`: The array to store the arguments.
  - `char *arg_str`: The input string to be split.
  - `int max_args`: The maximum number of arguments to extract.
- **Returns**:
  - `int`: The number of arguments extracted.

Text surrounded by double quotes '"' will be treated as a single argument.

#### str_to_lower(char \*str)

Converts all characters in the string to lowercase.

- **Parameters**:
  - `char *str`: The string to be converted.
- **Returns**:
  - `char *`: A pointer to the converted string.

#### str_to_upper(char \*str)

Converts all characters in the string to uppercase.

- **Parameters**:
  - `char *str`: The string to be converted.
- **Returns**:
  - `char *`: A pointer to the converted string.

#### strnz\_\_cpy

Copies a string from source to destination with a specified maximum length.

- **Parameters**:
  - `char *dest`: The destination buffer.
  - `const char *src`: The source string.
  - `size_t n`: The maximum number of characters in the resulting string.
- **Returns**:
  - int: The length of the resulting string

strnz**cpy differs from strncpy in that it is limited, not by the number of characters copied, but by the size of the destination buffer, ensuring null-termination. With strnz**cpy, you can prevent a buffer overrun by setting the third parameter to the size of the destination buffer - 1, leaving space for the null terminator.

#### strnz\_\_cat

Concatenates a source string to a destination string with a specified maximum length.

- **Parameters**:
  - `char *dest`: The destination buffer.
  - `const char *src`: The source string.
  - `size_t n`: The maximum size of the destination buffer.
- **Returns**:
  - int: The length of the resulting string

strnz**cat differs from strncat in that it is limited, not by the number of characters concatenated, but by the size of the destination buffer, ensuring null-termination. With strnz**cat, you can prevent a buffer overrun by setting the third parameter to the size of the destination buffer - 1, leaving space for the null terminator.

#### strnz(char \*str, int max_len)

Ensures that a string is null-terminated within a specified maximum length. Terminates the string on encountering a line-feed ('\n') or carriage-return ('\r').

- **Parameters**:
  - `char *str`: The string to be checked.
  - `int max_len`: The maximum length of the string.
- **Returns**:
  - int: Length of the null-terminated string.

#### strnz_dup(char \*str, int max_len)

Duplicates a string up to a specified maximum length, a line-feed ('\n'), or a carriage-return ('\r'), ensuring null-termination. Because strnz_dup allocates memory, it is up to the caller to free the memory when it is no longer needed.

- **Parameters**:
  - `char *str`: The string to be duplicated.
  - `int max_len`: The maximum length of the string.
- **Returns**:
  - char \*: A pointer to the newly allocated duplicated string.

#### void str_subc(char *d, char *s, char ReplaceChr, char \*Withstr, int l)

Replaces all occurrences of a specified character in a string with another string.

- **Parameters**:
  - `char *d`: The destination buffer.
  - `char *s`: The source string.
  - `char ReplaceChr`: The character to be replaced.
  - `char *Withstr`: The string to replace the character with.
  - `int l`: The maximum length of the destination string.

#### bool stripz_quotes(char \*s)

Removes surrounding double quotes from a string if they exist.

- **Parameters**:
  - `char *s`: The string to be processed.
- **Returns**:
  - `bool`: True if quotes were removed, false otherwise.

#### chrep(char \*s, char old_chr, char new_chr)

Replaces all occurrences of a specified character in a string with another character.

- **Parameters**:
  - `char *s`: The string to be processed.
  - `char old_chr`: The character to be replaced.
  - `char new_chr`: The character to replace with.
- **Returns**:
  - void: no return

#### file_spec_path(char *fp, char *fs)

Extracts the path component from a file specification and places it in fp. It is the caller's responsibility to ensure that fp has enough space to hold the path.

- **Parameters**:
  - `char *fp`: The buffer to store the extracted path.
  - `char *fs`: The file specification string.
- **Returns**:
  - `void`: no return

Unlike the POSIX implementation of basename(), this function does not modify the input string. Also, a character array may be used as the first argument, obviating the need for dynamic memory allocation.

#### file_spec_name(char *fn, char *fs)

Extracts the file name component from a file specification and places it in fn. It is the caller's responsibility to ensure that fn has enough space to hold the file name.

- **Parameters**:
  - `char *fn`: The buffer to store the extracted file name.
  - `char *fs`: The file specification string.
- **Returns**:
  - `void`: no return

Unlike the POSIX implementation of dirname(), this function does not modify the input string. Also, a character array may be used as the first argument, obviating the need for dynamic memory allocation. There is no GNU version of dirname().
