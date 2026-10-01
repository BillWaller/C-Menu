# C-Menu Command Line Interface

## C-Menu Main Suite

The following components may be called as functions from a C program, or used as
command line utilities. The command line utilities are installed in ~/menuapp/bin by default.

See [c-menu documentation](docs/c-menu.md) for more information.

### form - Form Maker

Reads a text file containing simple field definitions to generate an on-screen form.

See [form documentation](docs/form.md) for more information.

### menu - Menu Maker

Reads a text file containing simple menu definitions to generate an on-screen menu.

### pick - Picker

Reads text files containing items from which the user can select items.

See [pick documentation](docs/pick.md) for more information.

### view - Pager/Viewer

Like less, but windowed and blazingly fast. 

See [view documentation](docs/view.md) for more information

## Command Line Utilities

The following may be used as stand-alone command line utilities.

### lf - High Performance File Finder

File finder like find, but more intuitive with multi-threaded concurrency and other
optimizations make it 10x faster than find, and even faster than fd.

See [lf documentation](docs/lf.md) for more information.

### rsh - Root Shell Helper

when installed as setuid root, rsh invokes a root shell without having to enter a password. This shell can be used interactively or to run scripts or commands.

See [rsh documentation](docs/rsh.md) for more information.

### rsh_pam

Runs rsh with PAM authentication. This is the default mode of operation for rsh.

### rsh_static

Statically linked version of rsh that does not require PAM. This is useful for running rsh in environments where some standard libraries are not available, or where
PAM is not available.

### stripansi

Removes ANSI SGR escape sequences from text files making them easier to read and
edit with a text editor. 

### detach

Starts external programs detached from the terminal so that the parent process
can continue to run without waiting for the child process to finish. Can be used
to start GUI programs from a terminal without blocking the terminal.

### whence

Report the locations of executables in the PATH environment variable. Similar to
the which command, but whence can report all locations of an executable in the PATH, not just the first one.

### enterchr

Pause for a single character of input from the user. This is useful for scripts
that need to pause for user input without requiring the user to press Enter.
Sets the terminal state to raw mode and disables echoing of input characters.

### enterstr

Pause for a string of input from the user. This is useful for scripts that need to pause for user input without requiring the user to press Enter. Sets the terminal state to raw mode and optionally disables echoing of input characters. Handy for password input.

## C-Menu Build Scripts

### chkui

Report or switch the current C-Menu UI between NCurses and Notcurses.

### cmake.sh

Creates a CMake build environment for C-Menu. This script is used to build C-Menu from source code.

## C-Menu Trivial Example Programs

### amort

Prints a loan amortization schedule for a given principal, interest rate, and
number of payments. The schedule shows the monthly payment amount, the interest paid, and the remaining balance after each payment.

The purpose of amort is to demonstrate how to integrate an external command line program into a C-Menu application.

### iloan

Given any 3 of 4 parameters, (principal, interest rate, number of payments,
monthly payment amount), iloan computes the missing parameter.

The purpose of iloan is to demonstrate how to integrate an external command line program into a C-Menu application.

### ui_hello

A minimal UAL_UI program to display "Hello World!" in a bordered window.

The purpose of ui_hello is to demonstrate how to create simple UAL_UI programs that display text on UI surfaces and can be integrated into C-Menu applications.

### ui_visual

A minimal UAL_UI program to display an image in a bordered window.

The purpose of ui_visual is to demonstrate how to create simple UAL_UI programs that display image files on UI surfaces and can be integrated into C-Menu applications.

