# CMENU - USER GUIDE

## Programs: Menu, Form, Pick, View, and RSH

## CMENU

<img src="screenshots/sample_menu.m.png" alt="Curses Keys" title="Sample Menu" />

MENU reads a simple description file like the one above and displays a menu to the user. When the user selects an item, MENU executes the corresponding command. It's like writing a shell script, but with a nice TUI menu interface.

The structure of the description file is straightforward. Each menu item consists of a label and a command to execute. The label is displayed in the menu, and the command is executed when the user selects that item. The first character on each line indicates the type of line: ":" for a menu item, "H:" for the menu header, "!" for commands, and '#' for comments.

The string of alpha-numeric characters following the "!", and terminated by a space may be certain internal commands like !help, !return, !ckeys, !pick, !form, or !rsh. These commands invoke other CMENU programs to provide additional functionality. If the string is not one of these internal commands, it is treated as an external command to be executed by the shell. This allows you to run any shell command or script from within the menu. The remainder of the line after the command is passed as arguments to the command.

## CKEYS

CKeys is not a structural part of CMENU, but it's included as a demonstration
of C-Menu's user interface and as a handy utility for developers working with Curses-based applications. It displays a list of Curses key symbols along with their corresponding octal, decimal, and hex values. It also shows mouse actions and pointer coordinates.

<img src="screenshots/Curses_Keys.png" alt="Curses Keys" title="Curses Keys" />

## ILOAN

ILoan is a sample application included with CMENU that demonstrates how to create a loan calculator interface using the Form program. Given any three of Present Value, Number of Periods, Payment Amount, or Interest Rate, it calculates the fourth value. When the user presses "F5 Calculate", Form passes the variables entered to ILoan, a stand-alone executable and uses the output to fill in the missing value, which is Payment Amount in the screen below.

<img src="screenshots/iloan.png" alt="ILoan" title="ILoan" />

## FORM

FORM reads a description file that defines a data entry form. It displays the form to the user, allowing them to input data in a structured manner. The entered data can then be processed by a specified command or script, or written to a file.

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

## Line Type Speecifiers (H, T, F, C, and ?)

- '#' Comment line (ignored)
- 'H' The header to be displayed at the top of the form
- 'T' Text field (line:column:length:text)
- 'F' Input field (line:column:length:type)
- 'C' Instructs Form to provide an "F5 Calculate" option
- '?' A user supplied help file for the form.

## Field Delimiters:

The ":" character is used as a delimiter in the fields above, but any
character that is placed immediately after the line designator (H, T, F, C, or ?)
will be used as a delimiter. For example, the following two lines are
equivalent:

```
T:2:4:Enter any three of the four values to calculate the fourth.
T|2|4|Enter any three of the four values to calculate the fourth.
```

## FORM Data Types

FORM displays data entry forms based on a description file. It allows users
to input data in a structured manner. The entered data can then be processed
by a specified command or script.

<img src="screenshots/data-types.f.png" />

<img src="screenshots/data-types.png" />

Decision, Inc. used CMENU's FORM program to augment it's Radio Broadcast
accounting, scheduling and management system. It was particularly useful
as a front-end for our SQL database applications.

<img src="screenshots/Receipt.png" />

Need quick and easy Cash Receipts, General Journal, or wedding invitation
list? FORM has you covered. The application shown above took about 10
minutes from design to test. It doesn't post transactions, or keep running
balances yet, but that's why we have people like you.

FORM also makes a great front-end for SQL database queries.

```
       String: Any text
       Decimal_Int: Integer number
       Hex_Int: Hexadecimal integer
       Float: Floating point number
       Double: Double precision floating point number
       Currency: Currency amount
       APR: Annual Percentage Rate
       Date: {int yyyy; int mm; int dd}
       Time: {int hh; int mm; int ss}
```

All field data is passed through "format_field()" for validation and conversion.
It can easily be passed to user programs in this same module, and it can be passed
as unformatted strings, formatted strings, or C data types as shown below.

Currently, the data types are as shown in the following listing, and more will
be added in future releases. This listing also shows how each data type is processed
within the "format_field()" function in fields.c and serves as a guide for adding new types.

The send and receive client functions can be used to serialize and transmit/receive
data to and from user programs via simple files, json, or network protocols.

```c
 fields.c

 int format_field(Form *, char *s) {

     char   field_s[MAXLEN];
     int    decimal_int_n = 0;
     int    hex_int_n = 0;
     float  float_n = 0.0;
     double double_n = 0.0;
     double currency_n = 0.0;

     struct {
         int yyyy;
         int mm;
         int dd;
     } Date;

     struct {
        int hh;
        int mm;
        int ss;
     } Time;

     switch (form->field[form->fidx]->data_type) {
     case FF_CURRENCY:
         numeric(field_s, input_s); // remove "," and "$"
         sscanf(field_s, "%lf", &currency_n);
         if (!valid_currency(currency_n))
             return false;
         sprintf(accept_s, "%.2lf", currency_n);
         sprintf(display_s, "%'.2lf", currency_n);
         right_justify(display_s, fl);
         send_client_currency(form->fidx, currency_n);
         break;

     case FF_YYYYMMDD:
         numeric(field_s, input_s); // remove "/" and "-"
         sscanf(field_s, "%4d%2d%2d", &Date.yyyy, &Date.mm, &Date.dd);
         if (!is_valid_date(Date.yyyy, Date.mm, Date.dd))
             return false;
         sprintf(display_s, "%04d-%02d-%02d", Date.yyyy, Date.mm, Date.dd);
         send_client_date(form->fidx, Date);
         break;
     ...

```

The Field Format Specifiers can be any combination of upper and lower case,
and new types can be easily added by modifying the source code.

## RSH


## CMENU with PICK

<img src="screenshots/Pick.png" />

This program provides a list of objects from arguments or a text file
and lets the user select any number to be written to a file or provided
as arguments to an executable specified in the description file.

## FORM

FORM is a lightweight and flexible form handling library designed to simplify the process of creating, validating, and managing forms in text-based applications.

It provides a straightforward API for defining form fields, handling user input, and performing validation checks.

## Key Features

- Easy Form Creation: Define forms with various field types such as text, number, email, and more.
- Validation: Built-in validation rules to ensure data integrity, including required fields, format checks, and custom validators.
- User Input Handling: Seamlessly capture and process user input from the command line or text-based interfaces.
- Customizable: Extendable architecture allowing developers to create custom field types and validation rules.
- Integration: Designed to work well with other components of the C-Menu Project, enabling a cohesive development experience.

## Numeric Formats Supported

<img src="screenshots/form.png" alt="Curses Keys" title="Sample Menu" />

FORM displays data entry forms based on a description file. It allows users
to input data in a structured manner. The entered data can then be processed
by a specified command or script.

<img src="screenshots/form.png" alt="Curses Keys" title="Sample Menu" />

Decision, Inc. used CMENU's FORM program to augment it's Radio Broadcast
accounting, scheduling and management system. It was particularly useful
as a front-end for our SQL database applications.

<img src="screenshots/Receipt.png" />

Need quick and easy Cash Receipts, General Journal, or wedding invitation
list? FORM has you covered. The application shown above took about 10
minutes from design to test. It doesn't post transactions, or keep running
balances yet, but that's why we have people like you.

FORM also makes a great front-end for SQL database queries.

## A Sample Menu Description File

```
H:SAMPLE MENU

:Gnumeric
!exec gnumeric

:Shell Script
!exec bash -c script.sh

:Shell Script as Root
!exec rsh -c script.sh

:Full Screen (root) Shell
!exec rsh

:Test Curses Keys
!ckeys

:Pick Items From a List
!pick -i picklist -M -c vi picklist.out

:Cash Receipts
!form receipt.d -c receipt.sh

:Help
!help ~/menuapp/doc/applications.hlp

:Exit Applications
!return
```

As you can see, the description file is straightforward and easy to read. Each menu item consists of a label and a command to execute. The label is displayed in the menu, and the command is executed when the user selects that item.

Here's just one example of how easy it is to create useful programs with the C-Menu Form facility.

<img src="screenshots/iloan_f.png" alt="Curses Keys" title="Sample Application" />

<img src="screenshots/iloan.png" alt="Curses Keys" title="Sample Application" />

We hope you find CMENU useful for your projects. It's a powerful tool that can
greatly simplify the process of creating text-based user interfaces for
your applications.

## VIEW

**VIEW** is a simple text file viewer that allows users to view text files in a
terminal environment. It supports basic navigation and regular expression
search functionality, which comes in handy for displaying help files or
other text-based documentation. **VIEW** can be invoked from within MENU, FORM,
or PICK to provide contextual help or information.

You may have noticed that Nvim doesn't render ANSI escape sequences. Why should it? How often do you need to edit a file that contains ANSI escape sequences? That's what pagers like **less** and **C-Menu View** were designed to do.

#### Nvim Screenshot

<img src="screenshots/nvim-log.png" alt="nvim log" title="nvim log" />

#### C-Menu View Screenshot

<img src="screenshots/view.png" alt="View" title="View" />

One especially useful feature of VIEW is its incredible speed with very large
text files. VIEW can open and display multi-megabyte text files almost
instantaneously, making it an excellent choice for viewing log files or other
large documents. While NVIM and other modern editors are outstanding for code
editing, it's just not practical to open a 100MB log file in them. VIEW
handles large files with ease and zips through them with lightning speed.

## RSH

<img src="screenshots/rsh.png" />

Despite its name, RSH is not a shell. It is a shell runner, which allows
you to specify your shell of choice, and provides a consistent environment
for running shell scripts and commands. RSH was designed to be invoked from
within MENU, FORM, or PICK to execute commands that require elevated
privileges, but its functionality extends beyond that.

You can execute commands in either user or root mode, making it a versatile
tool for developing aplication front-ends. RSH ensures that your scripts
and executables run in a controlled environment, reducing the chances of
unexpected behavior due to differing shell environments. RSH forks and waits
for its spawn to complete before returning control to the calling program.
When executed under CMenu's signal handler, it catches and displays the
exit status of the command, allowing for better error handling. Instead of
using su -c or sudo to run commands as root, you can use rsh -c to achieve
the same result in a more streamlined manner. You can literally have root
access within a fraction of a second, making it ideal for work that
requires frequent switching between user and root modes for various
administrative tasks.

Many system administrators and developers find RSH invaluable for tasks
that require elevated privileges. RSH eliminates the need to repeatedly enter
passwords or switch users, streamlining workflows and improving efficiency. We all
know it's not a good idea to run everything as root, but sometimes a user want's to
avoid precious seconds it takes to enter passwords for su. With RSH, it takes three
keystrokes to enter root mode and two keystrokes to get out.

Please be very careful when using RSH in setuid root mode. Keep the
executable protected in your home directory with appropriate permissions
to prevent promiscuous access by unauthorized users. RSH should be provided
only to trusted users who understand the implications of executing commands
with elevated privileges. Used inappropriately, it can lead to system
instability or security vulnerabilities.

## Build Instructions

### Makefile

<img src="screenshots/make.png" alt="Makefile Build" title="Makefile Build" />

### CMake Build

<img src="screenshots/cmake_build.png" alt="CMake Build" title="CMake Build" />

### CMake Install

<img src="screenshots/cmake_install.png" alt="CMake install" title="CMake install"  />

## Features

- Create and manage multiple menus, forms, and pickers

- Define interfaces using simple configuration files

- Perfect for shell scripting, command-line, and terminal based applications

- Made for Linux and Unix-like operating systems

- Blazingly fast, even on older hardware

- Text-based user interface (TUI) using ncurses

- Easily customize menu options and actions

- Any level of sub-menus

- Navigation using keyboard inputs the way God intended

- Configurable appearance and behavior

- Cross-platform compatibility

- Open-source and free to use

## CMENU Command Line Options

```
usage: {menu|pick|form|view}

long option          type      group       mask  flg description
-------------------  -------   ----------  ----- --- --------------------------------
--minitrc             string    file spec   mpfv  -a: configuration file spec
--cmd_spec            string    misc        .pfv  -c: command executable
--mapp_spec           string    file name   mpfv  -d: description spec
--f_erase_remainder   yes/no    flag        ..f.  -e: erase remainder of line on enter
--in_spec             string    file name   .p..  -i: input spec
--mapp_home           string    directory   mpfv  -m: home directory
--selections          integer   parameters  .p..  -n: number of selections
--out_spec            string    file name   .p..  -o: output spec
--f_at_end_remove     yes/no    flag        ...v  -r: remove file at end of program
--f_squeeze           yes/no    flag        ...v  -s  squeeze multiple blank lines
--tab_stop            integer   parameters  ...v  -t: number of spaces per tab
--mapp_user           string    directory   mpfv  -u: user directory
--f_ignore_case       yes/no    flag        ...v  -x: ignore case in search
--f_at_end_clear      yes/no    flag        mpfv  -z  clear screen at end of program
--answer_spec         string    file name   ..f.  -A: answer spec
--bg_color            integer   parameters  mpfv  -B: background_color
--cols                integer   parameters  mpfv  -C: height in columns
--fg_color            integer   parameters  mpfv  -F: foreground_color
--help_spec           string    file name   mpfv  -H: help spec
--lines               integer   parameters  mpfv  -L: width in lines
--f_mutiple_cmd_args  integer   parameters  mpfv  -M  multiple command arguments
--bo_color            integer   parameters  mpfv  -O: border_color
--prompt              string    misc        ...v  -P: prompt (S-Short, L-Long, N-None)[string]
--start_cmd           string    misc        ...v  -S  command to execute at start of program
--title               string    misc        mpfv  -T: title
--begx                integer   parameters  mpfv  -X: begin on column
--begy                integer   parameters  mpfv  -Y: begin on line
--f_stop_on_error     yes/no    flag        mpfv  -Z  stop on error
--mapp_data           string    directory   mpfv      data directory
--mapp_help           string    directory   mpfv      help directory
--mapp_msrc           string    directory   mpfv      source directory
```

## MINITRC Runtime Configuration

<img src="screenshots/minitrc.png" />

User's can have multiple runtime configurations. In the snippet above, the
standard ISO 6429 / ECMA-48 colors have been redefined and orange has been
added.

## Installation

To install CMENU, simply download the source code from the repository and follow the installation instructions provided in the INSTALL.md file.
