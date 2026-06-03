---
title: "menu"
section: 1
header: User Manuals
footer: C-Menu Version 0.2.9
author: Bill Waller
date: June 2026
---

# NAME

menu

# SYNOPSIS

Usage: menu [-DWk?V] [-a file_spec] [-C number] [-L number] [-n number]
[-o file_spec] [-X number] [-Y number] [-A file_spec]
[-c file_spec] [-d file_spec] [-H file_spec] [-i file_spec]
[-R file_spec] [-S file_spec] [-T text] [-w seconds] [-e bool]
[-j bool] [-s bool] [-x bool] [-f char] [-N[bool]] [-t number]
[-u text] [--f_dump_config] [--f_write_config]
[--minitrc=file_spec] [--parent_cmd] [--cols=number]
[--lines=number] [--select_max=number] [--out_spec=file_spec]
[--begx=number] [--begy=number] [--cmd_all=file_spec]
[--cmd=file_spec] [--mapp_spec=file_spec] [--help_spec=file_spec]
[--in_spec=file_spec] [--receiver_cmd=file_spec]
[--provider_cmd=file_spec] [--title=text] [--wait_timeout=seconds]
[--f_erase_remainder=bool] [--f_strip_ansi=bool]
[--f_squeeze=bool] [--f_ignore_case=bool] [--fill_char=char]
[--f_ln[=bool]] [--tab_stop=number] [--brackets=text]
[--bg_clr_x=hex_clr] [--bo_clr_x=hex_clr] [--fg_clr_x=hex_clr]
[--ln__bg_clr_x=hex_clr] [--ln_clr_x=hex_clr] [--blue_gamma=float]
[--gray_gamma=float] [--green_gamma=float] [--red_gamma=float]
[--bblack=hex_clr] [--bblue=hex_clr] [--bcyan=hex_clr]
[--bgreen=hex_clr] [--black=hex_clr] [--blue=hex_clr]
[--bmagenta=hex_clr] [--bred=hex_clr] [--bwhite=hex_clr]
[--byellow=hex_clr] [--cyan=hex_clr] [--editor=text]
[--green=hex_clr] [--magenta=hex_clr] [--red=hex_clr]
[--white=hex_clr] [--yellow=hex_clr] [--mapp_data=directory]
[--mapp_help=directory] [--mapp_home=directory]
[--mapp_msrc=directory] [--mapp_user=directory] [--help] [--usage]
[--version] [INPUT] [OUTPUT] [HELP] [ARG4] [ARG5]

# DESCRIPTION

# OPTIONS

Usage: menu [OPTION...] [INPUT] [OUTPUT] [HELP] [ARG4] [ARG5]

## GEOMETRY

    By default, C-Menu determines the size and position of the Window based on
    content and terminal size. The following options allow you to specify the
    location and size of Windows, but C-Menu may override if the specified
    geometry is too large for the terminal.

-Y, --begy=number

        The terminal line on which the top of the Window is placed.

-X, --begx=number

        The terminal column on which the left side of the Window is placed.

-C, --cols=number

        Window width in columns.

-L, --lines=number

        Window height in lines.

## CONFIGURATION

    C-Menu Menu, Form, Pick, and View read configuration data from $CMENU_HOME/.minitrc. The following options allow you to write configuration data to a file or read configuration data from a file other than $CMENU_HOME/.minitrc.

-D, --f_dump_config

        Write configuration data to $CMENU_HOME/minitrc.dmp

-W, --f_write_config

        Write resident configuration to $CMENU_HOME/minitrc. This file may be
        used as a backup configuration, used as a tmplate for new configurations
        or used to transfer resident configuration to other machines. Copy it to
        $CMENU_HOME/.minitrc to make it your active configuration.

-a, --minitrc=file_spec

        Read configuration from file_spec instead of $CMENU_HOME/.minitrc.

## INPUT/OUTPUT

    Input and output for C-Menu Form, Pick, and View may utilize files,
    standard IO, passing arguments through direct execution, or in the
    near future network connections.

### FILES

-o, --out_spec=file_spec

        Form and Pick write output to file_spec instead of stdout.

-A, --cmd_all=file_spec

        This command will be provided to view's command processor for immediate
        execution on startup. It may be used to set View options or execute any other command recognized by the view command processor. For example, "/pattern"
        would search for pattern on startup.

-c, --cmd=file_spec

        This command may be executed at arbitrary points during various events.
        For example, Form may use a -c command to execute an SQL query to
        provide information related to the current form. The presence of these
        command hooks will be doumented with each feature for which it is used.

-d, --mapp_spec=file_spec

        Description files determine the operational characteristics of C-Menu
        Menu and Form components. In the example C-Menu application, these files are
        stored in $CMENU_HOME/menuapp/msrc. Any file names may be used, but it
        may be useful to append .m to menu description files and .f to form
        description files.

-H, --help_spec=file_spec

        Help files provide text that is displayed in the help window. In the
        example C-Menu application, these files are stored in
        $CMENU_HOME/menuapp/help. Any file names may be used, but it may be useful to append .help to help files. Conventionally, source help files are designated
        with the extension _help.

-i, --in_spec=file_spec input spec

        C-Menu Form, Pick, and view may read input from file_spec.

## DIRECTORIES

--mapp_data=directory

        In the example C-Menu application, this directory is
        $CMENU_HOME/menuapp/data. It contains data files used by the application.
        C-Menu Form may read data from files in this directory to populate
        fields.

--mapp_help=directory

        In the example C-Menu application, this directory is
        $CMENU_HOME/menuapp/help. It contains context sensitive help files used
        by the application. C-Menu Form, Pick, and View may read help from files in this directory.

--mapp_home=directory

        In the example C-Menu application, this directory is
        $CMENU_HOME/menuapp. It contains the msrc, help, and data directories.

--mapp_msrc=directory
In the example C-Menu application, this directory is
$CMENU_HOME/menuapp/msrc. It contains menu and form description files.

--mapp_user=directory

        In the example C-Menu application, this directory is
        $CMENU_HOME/menuapp/user. It may be used to store user specific data.

## COMMANDS

-R, --receiver_cmd=file_spec

        A receiver is an executable file that reads its input from piped
        output of the calling program. This is not a named pipe or a
        network connection, but a direct connection between the calling
        program and the receiver.

-S, --provider_cmd=file_spec execute provider of piped input

        A provider is the inverse of a receiver, that sends output to the
        calling program. The calling program receives input from the piped
        output of the provider program. This is not a named pipe or a
        network connection, but a direct connection between the calling
        program and the provider.

-T, --title=text

        Window title displayed on the top line of the window.

-w, --wait_timeout=seconds

        Determines how long to wait for IO before timing out. This feature is
        useful when using receiver and provider commands, but it may also be used for file IO. If the timeout is reached, the pick engine will display a countdown
        window indicating that it is waiting for IO, and the user may choose to cancel the process or continue waiting in timeout intervals.

-e, --f_erase_remainder=bool erase remainder of line on enter
-j, --f_strip_ansi=bool always strip ansi when writing
-s, --f_squeeze=bool squeeze multiple blank lines
-x, --f_ignore_case=bool ignore case in search
-f, --fill_char=char field fill_char
-N, --f_ln[=bool] line numbers in view
-t, --tab_stop=number number of spaces per tab
-u, --brackets=text brackets around fields

-k, --parent_cmd

-n, --select_max=number

        Number of selections allowed, 0 for unlimited. Once the user has
        selected the maximum number of items, the pick engine will proceed
        as if the user had pressed enter.

## THEME

      --nt_fg=hex_clr           normal foreground
      --nt_bg=hex_clr           normal background
      --nt_rev_fg=hex_clr       normal reverse foreground
      --nt_rev_bg=hex_clr       normal reverse background
      --nt_hl_fg=hex_clr        normal highlight foreground
      --nt_hl_bg=hex_clr        normal highlight background
      --nt_hl_rev_fg=hex_clr    normal highlight reverse foreground
      --nt_hl_rev_bg=hex_clr    normal highlight reverse background

      --bg_clr=hex_clr          background
      --bg_clr_x=hex_clr        background
      --bo_clr_x=hex_clr        border
      --fg_clr_x=hex_clr        foreground

      --ln_clr_x=hex_clr        line number foreground
      --ln__bg_clr_x=hex_clr    line number background

## GAMMA

      --blue_gamma=float     blue_gamma (View)
      --gray_gamma=float     gray gamma (View)
      --green_gamma=float    green gamma (View)
      --red_gamma=float      red gamma (View)

## STANDARD COLORS

      --bblack=hex_clr       bright black (#7f7f7f)
      --bblue=hex_clr        bright blue (#00cfFF)
      --bcyan=hex_clr        bright cyan (#00FFFF)
      --bgreen=hex_clr       bright green (#00FF7f)
      --black=hex_clr        black (#000000)
      --blue=hex_clr         blue (#0000FF)
      --bmagenta=hex_clr     bright magenta (#FF00FF)
      --bred=hex_clr         bright red (#FF3737)
      --bwhite=hex_clr       bright white (#FFFFFF)
      --byellow=hex_clr      bright yellow (#FFeF00)
      --cyan=hex_clr         cyan (#00dfdf)
      --editor=text          default editor
      --green=hex_clr        green (#00cf00)
      --magenta=hex_clr      magenta (#9f009f)
      --red=hex_clr          red (#bf0000)
      --white=hex_clr        white (#d0d0d0)
      --yellow=hex_clr       yellow (#efbf00)

-?, --help Give this help list
--usage Give a short usage message
-V, --version Print program version-?, --help

    Give this help list

--usage

    Give a short usage message

-V, --version

    Print program version

A space after short options is optional. For example, -s10M and -s 10M are both valid.

Option arguments may be ganged. For example, to list all files, directories, and
links, you can use -t f -t d -t l or -tfdl.

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

# EXAMPLES

# REPORTING BUGS

Report bugs to <billxwaller@gmail.com>.

# COPYRIGHT

Copyright © 2026 Bill Waller.

# LICENSE

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# SEE ALSO

C-Menu Menu, Form, Pick, View, RSH, C-Keys
