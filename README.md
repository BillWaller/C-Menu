 # CMENU - A TUI Menu System

## Programs: Menu, Form, Pick, View, and RSH

CMENU is a lightweight, customizable, and easy-to-learn suite of programs
for quickly and easily creating menus, entry forms, and pickers with a
text-based user interface(TUI) for applications running on Linux and Unix-like
operating systems. CMENU is designed to be simple to use while providing
powerful features for developers who need to implement menu driven
frameworks for user applications in terminal environments.

### CMENU with Test Curses Keys

href="https://github.com/BillWaller/Curses_keys">https://github.com/BillWaller/Curses_Keys</a><br>
<p>Copyright (C) 2025-2025 BillWaller</p>
<figure>
<img src="Curses_Keys.png" alt="image" />
<figcaption aria-hidden="true">image</figcaption>

MENU reads a simple description file like the one below and displays a context
menu to the user. When the user selects an item, MENU executes the
corresponding command. It's like writing a shell script, but with a nice TUI
menu interface.

Is a particular key not working for your project? Curses Keys (or CKeys)
provides an easy way to determine whether the problem is with your code
or your terminfo/termcap files. Or, if you just don't remember the key
symbol for Curses. It also gives you the Octal, Decimal, and Hex codes
for keys not defined in Curses, so you can provide your own custom keys.

### CMENU with PICK

href="https://github.com/BillWaller/Pick">https://github.com/BillWaller/Pick</a><br>
<p>Copyright (C) 2025-2025 BillWaller</p>
<figure>
<img src="Pick.png" alt="image" />
<figcaption aria-hidden="true">image</figcaption>

This program provides a list of objects from arguments or a text file
and lets the user select any number to be written to a file or provided
as arguments to an executable specified in the description file.

### FORM

href="https://github.com/BillWaller/Receipt">https://github.com/BillWaller/Receipt</a><br>
<p>Copyright (C) 2025-2025 BillWaller</p>
<figure>
<img src="Receipt.png" alt="image" />
<figcaption aria-hidden="true">image</figcaption>

FORM displays data entry forms based on a description file. It allows users
to input data in a structured manner. The entered data can then be processed
by a specified command or script.

Decision, Inc. used CMENU's FORM program to augment it's Radio Broadcast
accounting, scheduling and management system. It was particularly useful
as a front-end for our SQL database applications.

Do you need to design a quick and easy Cash Receipts, General Journal,
or wedding invitation list? FORM has you covered. This particular
program took about 10 minutes from design to test. It doesn't post
transactions, yet. That's why we have people like you. FORM makes a
great front-end for SQL database queries.


### A Sample Menu Description File

```
:SAMPLE MENU

:
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

:
:Help
!help ~/menuapp/doc/applications.hlp

:Exit Applications
!return
```

### VIEW

VIEW is a simple text file viewer that allows users to view text files in a
terminal environment. It supports basic navigation and search functionality,
which comes in handy for displaying help files or other text-based
documentation. VIEW can be invoked from within MENU, FORM, or PICK to provide
contextual help or information.

One especially useful feature of VIEW is its incredible speed with very large
text files. VIEW can open and display multi-megabyte text files almost
instantaneously, making it an excellent choice for viewing log files or other
large documents. While NVIM and other modern editors are outstanding for code
editing, it's just not practical to open a 100MB log file in them. VIEW
handles this task with ease and zip through them with lightning speed.

### RSH

ref="https://github.com/BillWaller/rsh">https://github.com/BillWaller/rsh</a><br>
<p>Copyright (C) 2025-2025 BillWaller</p>
<figure>
<img src="rsh.png" alt="image" />
<figcaption aria-hidden="true">image</figcaption>

RSH is not a shell. It is a shell runner, which allows you to specify your
shell of choice, and provides a consistent environment for running shell
scripts and commands. You can execute commands in either user mode or root
mode, making it a versatile tool for various tasks. RSH ensures that your
scripts run in a controlled environment, reducing the chances of unexpected
behavior due to differing shell environments. RSH forks and waits for the
command to complete before returning control to the calling program. It
catches and displays the exit status of the command, allowing for better
error handling. Instead of using su -c or sudo to run commands as root,
you can use rsh -c to achieve the same result in a more streamlined manner.
You can literally have root access within a fraction of a second, making it
ideal for quick administrative tasks. RSH is particularly useful when
invoked from within MENU, FORM, or PICK to execute commands that require
elevated privileges.

You knew this was coming. Please be very careful when using RSH in root
mode, as it can potentially lead to system instability or security
vulnerabilities if misused.

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


### CMENU Command Line Options

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



## Installation

To install CMENU, simply download the source code from the repository and follow the installation instructions provided in the INSTALL.md file.
```
