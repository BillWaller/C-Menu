# C-Menu User Guide

## Introduction

The C-Menu is a versatile and user-friendly menu system designed for various applications. This guide provides detailed instructions on how to use and customize the C-Menu to fit your needs.

## Table of Contents

1. [Requirements](#requirements)
2. [Not Required, but Recommended](#not-required-but-recommended)
3. [Getting Started](#getting-started)
4. [Menu](#menu)
   - [Menu Example](#menu-example)
   - [Menu Description File Format](#menu-description-file-format)
5. [C-Menu Form](#form)
   - [Form Description File Example](#form-description-file-example)
   - [Form Line Type Specifiers (#, H, T, F, and ?)](#form-line-type-specifiers--h-t-f-and-)
   - [Form Field Delimiters](#form-field-delimiters)
   - [Form Data Types](#data-types)
   - [Form Line Syntax](#form-line-syntax)
   - [Form Options](#form-options)
6. [C-Menu Pick](#c-menu-pick)
   - [Pick Usage](#pick-usage)
   - [Pick Options](#pick-options)

### Requirements

- A compatible operating system (e.g., Linux, macOS).
- Basic knowledge of command-line operations.

### Not Required, But Recommended

- Familiarity with a text editor and configuration files.
- Rust Cargo for installing ancillary tools.
- Tree-sitter-cli for enhanced syntax highlighting.
- Root access for certain advanced features.

## Getting Started

To begin using the C-Menu, follow these steps:

1. Install the C-Menu package from the official repository.

```bash
gh repo clone BillWaller/C-Menu
```

2. Copy the sample menuapp directory structure to your home directory Initialize the C-Menu with default settings. Copy the sample /menuapp/minitrc to your home directory as ~/.minitrc:

   ```bash
   cp -Rdup C-Menu/menuapp ~/
   cp ~/menuapp/minitrc ~/.minitrc
   ```

3. Build C-Menu:

   ```bash
   cd C-Menu/src
   make
   sudo make install
   ```

To enable root access features, you need to install the RSH (Remote Shell) program with setuid root permissions. This allows certain menu items to execute commands with root privileges.

WARNING: Do not install RSH unless you understand the security implications of setuid root programs. RSH allows users to execute commands with root privileges, which can pose significant security risks if not managed properly.

4. Explore the ~/menuapp directory to familiarize yourself with its features.

## Menu

C-Menu parses a menu description file, which contains text lines to display and command lines, which are essentially operating system commands.

### Menu Example

```
:     APPLICATIONS
:     Neovim
!exec nvim
:     Root Neovim
!exec rsh -c nvim
:     Full Screen (root) Shell
!exec rsh
```

In this example, "Neovim" is a menu item that, when selected, will execute the command `nvim`. The user can select it by clicking on "Neovim" or by typing the corresponding letter assigned to it.

### Menu Description File Format

- Text lines start with a colon (:) in column 0.
- Command lines start with a bang (!).
- Each menu item consists of a text line followed by its corresponding command line.
- Blank lines and lines starting with a hash (#) are ignored.
- Lines can be continued by ending them with a backslash (\).
- Comments can be added after a hash (#) on any line.
- Leading and trailing whitespace on lines is ignored.
- Menu items can be grouped into sections by using text lines without corresponding command lines.

## Form

Form is a companion tool for C-Menu that allows users to create and manage forms for data entry and editing. It can be used to gather user input before executing commands from the C-Menu.

### Form Description File Example

```
H:Installment Loan Calculator
#
T:1:4:Enter any three of the four values to calculate the fourth.
T:2:4:Only one field can be left blank or zero.
T:3:4:Press F5 to calculate the missing value.
#
#
T:5:14:Principal Amount:
F:5:33:14:Currency
T:6:14:Number of Months:
F:6:33:5:Decimal_Int
T:7:10:Annual Interest Rate:
F:7:33:5:APR
T:8:16:Payment Amount:
F:8:33:12:Currency
T:10:1:First Payment Date (Yyyymmdd):
F:10:33:10:Yyyymmdd
C
?iloan.hlp
```

#### Form Line Type Speecifiers (#, H, T, F, and ?)

```
 # - Comment line (ignored)
 H - The header to be displayed at the top of the form
 T - Text field (line:column:length:text)
 F - Input field (line:column:length:type)
 ? - A user supplied help file for the form. If no path is given, Form will look fora file with the same name as the form but with a .hlp extension. It will search in the current directory and then in the menu help directory, ~/menuapp/help.
```

#### Form Field Delimiters

The ":" character is used as a delimiter in the fields above, but any
character that is placed immediately after the line designator (H, T, F, ?)
can be used as a delimiter. For example, the following two lines are
equivalent:

```
T:2:4:Enter any three of the four values to calculate the fourth.
T|2|4|Enter any three of the four values to calculate the fourth.
```

### Data Types

```
       String: Any text
  Decimal_Int: Integer number
      Hex_Int: Hexadecimal integer
        Float: Floating point number
       Double: Double precision floating point number
     Currency: Currency amount
          APR: Annual Percentage Rate
     Yyyymmdd: Date in YYYYMMDD format
       HHMMSS: Time in HHMMSS format
```

Data types determine the format of displayed data. Of course, all data is initially a text string, but Form converts numeric data to internal numeric binary according to the data type specified.

WARNING: For applications that require extreme accuracy, such as banking, financial, or scientific applications, none of the data types currently available in Form are recommended. The plan is to add 128-bit BCD support via decNumber or rust-decimal in a future release.

The Field Format Specifiers can be any combination of upper and lower case,
and new types can be easily added by modifying the source code.

#### Form Line Syntax

```
 H<delimiter>Header Text
 T<delimiter>Line<delimiter>Column<delimiter>Length<delimiter>Text
 F<delimiter>Line<delimiter>Column<delimiter>Length<delimiter>Type
 ?<delimiter>Help File Path
 # Comment line (ignored)

```

#### Form Options

```
 -h            Display help information
 -v            Display version information
 -d <file>     Specify the form description file
 -o <file>     Specify the output file for form data
 -i <file>     Specify the input file for pre-filling form data
 -S <string>   Command to provide input via a STDIN pipe
 -R <string>   Command to receive output via a STDOUT pipe
 -c <string>   Command to be executed with arguments provided by form
```

## C-Menu Pick

C-Menu Pick is a utility that allows users to create and manage pick lists for selection within the C-Menu system, or stand-alone. It can be used to present a list of options for the user to choose from.

### Pick Usage

Pick does not have a description file, but instead takes its input from standard input (stdin) or a file. Each line of input represents a separate pick item. The user can select an item by clicking on it or moving the cursor to highlight it and pressing the space bar to toggle it on or off. The number of items that can be selected is configurable by a command-line option (-n).

Pick can be invoked from within C-Menu or from the command line using the following syntax:

```
pick [options] [input_file]

or

some_command | pick [options]
```

### Pick Options

```
 -n <number>   Set the maximum number of items that can be selected
 -h            Display help information
 -T            Title to be displayed at the top of the pick list
 -S <string>   Command to provide input via a STDIN pipe
 -R <string>   Command to receive output via a STDOUT pipe
 -c <string>   Command to be executed with arguments provided by pick
 -v            Display version information
```

## C-Menu View

C-Menu View is a utility that allows users to view text files within the C-Menu system, or stand-alone. It provides a simple interface for reading files without the need for an external text editor.

### View Options

```
 -L <number>   Set the number of lines for the view window
 -C <number>   Set the number of columns for the view window
 -y <number>   Set the beginning line for the view window
 -x <number>   Set the beginning column for the view window
 -h            Display help information
 -T            Title to be displayed at the top of the pick list
 -S <string>   Command to provide input via a STDIN pipe
 -v            Display version information
```

If -L and -C are not specified, View will attempt to use the terminal size. If -L and
-C are set, view will open in a box window.

View supports syntax highlighting via tree-sitter. To enable this feature, ensure that tree-sitter-cli is installed and that the appropriate grammar files are available. Alternatively, you can use "pygmentize" or "bat", but tree-sitter is preferred for performance and flexibility.

View uses complex-characters (cchar_t) for rendering text, which allows it to handle a wide range of character sets and encodings including ASCII, UTF-8, multi-byte, and wide-character (wchar_t) formats. View does not write ANSI escape sequences to the display, but instead converts and incorporates character attributes directly into the character data structures.

View displays its output on a virtual pad, which can be larger than the actual display window enabling the user to scroll horizontally and vertically.

View supports extended regular expressions (regex) for advanced text searching capabilities.

## C-Menu Options

<img src="screenshots/options.png" title="C-Menu Options" />


## Troubleshooting

If you encounter issues while using C-Menu, consider the following troubleshooting steps:

- Ensure that the menu description file is correctly formatted.
- Check for any syntax errors in command lines.
- Verify that all required dependencies are installed.
- Consult the FAQs section for common issues and solutions.
- Try running commands from a command line to isolate problems.
