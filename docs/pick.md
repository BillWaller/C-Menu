---
title: "pick"
section: 1
header: User Manuals
footer: C-Menu Version 0.2.9
author: Bill Waller
date: June 2026
---

# NAME

pick - select items from pick list

# SYNOPSIS

view [-Wk?V] [-a file_spec] [-C number] [-L number] [-T text]
[-X number] [-Y number] [-A file_spec] [-c file_spec]
[-d file_spec] [-g file_spec] [-H file_spec] [-i file_spec]
[-l text] [-o file_spec] [-R file_spec] [-S file_spec] [-Z text]
[-b text] [-e[bool]] [-f char] [-j[bool]] [-M[bool]] [-n number]
[-N[bool]] [-r[bool]] [-s[bool]] [-t number] [-u text] [-v[bool]]
[-w[bool]] [-x[bool]] [-z number] [--f_write_config]
[--minitrc=file_spec] [--parent_cmd] [--cols=number]
[--lines=number] [--title=text] [--begx=number] [--begy=number]
[--cmd_all=file_spec] [--cmd=file_spec] [--mapp_spec=file_spec]
[--log_file_spec=file_spec] [--help_spec=file_spec]
[--in_spec=file_spec] [--log_level=text] [--out_spec=file_spec]
[--receiver_cmd=file_spec] [--provider_cmd=file_spec]
[--timeout_secs=text] [--border=text] [--editor=text]
[--f_erase_remainder[=bool]] [--fill_char=char]
[--f_strip_ansi[=bool]] [--f_multiple_cmd_args[=bool]]
[--select_max=number] [--f_ln[=bool]] [--f_read_theme[=bool]]
[--f_squeeze[=bool]] [--tab_stop=number] [--brackets=text]
[--p_view_files[=bool]] [--wrap[=bool]] [--f_ignore_case[=bool]]
[--h_shift=number] [--bg=hex_clr] [--box_bg=hex_clr]
[--box_fg=hex_clr] [--brackets_bg=hex_clr] [--brackets_fg=hex_clr]
[--cmdln_bg=hex_clr] [--cmdln_fg=hex_clr] [--fg=hex_clr]
[--fill_char_bg=hex_clr] [--fill_char_fg=hex_clr]
[--ind_bg=hex_clr] [--ind_fg=hex_clr] [--ln_bg=hex_clr]
[--ln_fg=hex_clr] [--nt_bg=hex_clr] [--nt_fg=hex_clr]
[--nt_hl_bg=hex_clr] [--nt_hl_fg=hex_clr] [--nt_hl_rev_bg=hex_clr]
[--nt_hl_rev_fg=hex_clr] [--nt_rev_bg=hex_clr]
[--nt_rev_fg=hex_clr] [--ran_bg=hex_clr] [--ran_fg=hex_clr]
[--title_bg=hex_clr] [--title_fg=hex_clr] [--blue_gamma=float]
[--gray_gamma=float] [--green_gamma=float] [--red_gamma=float]
[--bblack=hex_clr] [--bblue=hex_clr] [--bcyan=hex_clr]
[--bgreen=hex_clr] [--black=hex_clr] [--blue=hex_clr]
[--bmagenta=hex_clr] [--bred=hex_clr] [--bwhite=hex_clr]
[--byellow=hex_clr] [--cyan=hex_clr] [--green=hex_clr]
[--magenta=hex_clr] [--red=hex_clr] [--white=hex_clr]
[--yellow=hex_clr] [--mapp_data=directory] [--mapp_help=directory]
[--mapp_home=directory] [--mapp_msrc=directory]
[--mapp_theme=file] [--mapp_user=directory] [--help] [--usage]
[--version] [INPUT] [OUTPUT] [HELP] [ARG4] [ARG5]


# DESCRIPTION

C-Menu Pick displays a list of items from which the user can select.

Specification:

```bash
!pick [ -n maximum_number_of_selections ][-m] \
    [ -i input_file ][ -S executable_provider ] \
    [ -o output_file ][ -c executable %% ]
```

-n maximum_number_of_selections. "-n n" is a convenience which directs Pick to automatically accept selections when the specified maximum number of items, "n" have been selected. For example, "-n 1" is commonly used to direct Pick to automatically accept the first item selected without requiring the user pressing to F10 Accept key. This feature is designed to optimize the user's economy of motion, making the selection process extremely fast and efficient. When "-n 1" is specified, the user can simply select an item and Pick will immediately dispatch the specified action.

-c execute command substituting "%%" with the selected item(s). Whether the
command specified with the -c option is executed once per selection or once for
all selections is determined by the presence or absence of the "-m option". Without the "-m" option, by default, if multiple items are selected, the command specified with -c will be executed once for each selection with that selection as an argument.

-m multiple_arguments flag. The -m option directs Pick to construct a command line with all selections as individual arguments. The specified command is executed once with all selections combined as individual arguments on a single command line.

An example use case for "-m" would be if you wanted to open multiple files in
C-Menu View using View's ":n" and ":p" commands to navigate between files. In that case, you would specify "-m" to have Pick execute View once with all selected files as arguments, allowing you to use View's built-in file navigation features. The same technique works with Vim, nvim, and less.

-i input_file directs Pick to read input from the specified file

-S executable_provider directs Pick to execute the specified external command
and read input from the command's standard output. The command specified with the -S option is executed when starting Pick, and its output is used as the list of items from which selections are made.

-o output_file directs Pick to write selected items to the specified file when the user presses F10 Accept.

Pick must have exactly one input method, either -i input_file or -S executable_provider_command. Combining -o and -c options is permissible, and will direct Pick to write the list of selected items to the specified file and also pass the list of selected items to the command specified by -c according to the presence or absence of the -m option. The selections are written to file before executing the specified command, so the command can read the selections from the file if needed.
