# C-Menu Example Application Menu

<!-- mtoc-start -->

* [Menu Title](#menu-title)
* [Full Screen (Root) Shell](#full-screen-root-shell)
* [Youtube (in Firefox)](#youtube-in-firefox)
* [C-Menu (in Ghostty)](#c-menu-in-ghostty)
* [HTOP (in Kitty)](#htop-in-kitty)
* [HTOP (in Ghostty)](#htop-in-ghostty)
* [Issue RSH Certificate](#issue-rsh-certificate)
* [Workstation Configuration](#workstation-configuration)
* [Diagnostic Utilities](#diagnostic-utilities)
* [Installment Loan Calculations](#installment-loan-calculations)
* [Cash Receipts](#cash-receipts)
* [Rustlings Source](#rustlings-source)
* [View Manual Pages](#view-manual-pages)
* [Edit C-Menu Description Files](#edit-c-menu-description-files)
* [View C-Menu Source with Tree-Sitter](#view-c-menu-source-with-tree-sitter)
* [View Source with Tree-sitter](#view-source-with-tree-sitter)
  * [View LSP Log](#view-lsp-log)
* [Help Menu](#help-menu)
* [Menu Description With Bat Syntax Highlighting](#menu-description-with-bat-syntax-highlighting)
* [View C-Menu Command Line Options](#view-c-menu-command-line-options)
* [View Highlighted view_engine.c](#view-highlighted-view_enginec)
* [Exit Applications](#exit-applications)

<!-- mtoc-end -->

---

![Main Menu](../screenshots/applications_menu.png)



## Menu Title

![app_menu_title](../screenshots/app_menu_title.png)

```
:    MAIN MENU
```

The title of the menu is specified as the first text line in the menu
description file.

---

## Full Screen (Root) Shell

![app_menu_01](../screenshots/app_menu_01.png)

```
:   -RFull Screen (Root) Shell
!exec rsh
```

- The -R option tells C-Menu to use the letter R as the hotkey to launch the command.
- !exec (execute) rsh launches the rsh command as a subprocess of C-Menu. rsh takes over the entire terminal window and runs in full screen mode. When rsh exits, control returns to C-Menu.

---

## Youtube (in Firefox)

![app_menu_02](../screenshots/app_menu_02.png)

```
:     Youtube (in Firefox)
!dexe firefox https://www.youtube.com
```

- Because no hotkey is specified, C-Menu uses the first letter of the menu selection
text as the hot key, which is the Y in Youtube.
- !dexe (detached execute) launches the firefox command as a detached and independent process. C-Menu does not wait for firefox to exit before returning control to C-Menu.

---

## C-Menu (in Ghostty)

![app_menu_03](../screenshots/app_menu_03.png)

```
:     C-Menu (in Ghostty)
!dexe ghostty -e menu
```

- The -e option tells ghostty to execute the menu command in a new terminal window. C-Menu does not wait for ghostty to exit before returning control to C-Menu.

## HTOP (in Kitty)


![app_menu_04](../screenshots/app_menu_04.png)

```
:     HTOP (in Kitty)
!exec kitty --detach -o initial_window_width=80c -o initial_window_height=20c htop
```

- We don't use !dexe with Kitty because Kitty has a built-in --detach option that allows it to run as a detached process. C-Menu does not wait for Kitty to exit before returning control to C-Menu.

- We use the -o option to specify both initial window width and hieght in
characters (c). see [Kitty Command Line Options](https://sw.kovidgoyal.net/kitty/command-line-options/) 
---

## HTOP (in Ghostty)

![app_menu_05](../screenshots/app_menu_05.png)

```
:     HTOP (in Ghostty)
!dexe ghostty --window-width=80 --window-height=20 -e htop
```

There are several things to learn about this menu selection.

- The hotkey assigned by C-Menu is the left parenthesis character (. C-Menu assigns the first character in the menu selection text that has not been reserved by assignment to another menu selection. C-Menu assigns hotkeys to menu selections in order, but letters assigned with the "-" dash are processed first, thus reserving those assignments. Because H, T, O, and P have already been assigned, the next available character is the left parenthesis. If you don't like that hotkey, you may specify a different character as the hotkey by adding it at the beginning of the line and preceding it with a dash. For example:

```
:     -HHTOP (in Ghostty)
```

- However, there is a gotcha. When you specify the letter H as the hotkey for
HTOP (in Ghostty), C-Menu will assign the left parenthesis character as the
hotkey for HTOP (in Kitty) because it has already been reserved by prepending
-H to HTOP (in Ghostty).

- We use !dexe with Ghostty because it does not have a built-in --detach option.
C-Menu does not wait for Ghostty to exit before returning control to C-Menu.

- As with HTOP (in Kitty), we specify the --window_width and --window-height in
  columns, and use Ghostty's -e option to execute htop in the new terminal window.

---

## Issue RSH Certificate

:     Issue RSH Certificate
!form rshusers.f -i rshusers.dat -o rshusers.dat

- This menu selection launches C-Menu's form utility to display an example form
  for issuing RSH certificates.

- The form utility provides a structured way to collect user input. It can be
used to:

- Create new data, or Edit existing data

- Read input from an external program via command line arguments, pipe, or file.

- Submit output to external programs for processing via command line arguments, pipe, or File.

- Interact with external programs to perform tasks such as validation, calculations, or data manipulation.

Currently, the form utility is a work in progress and is not fully implemented. It is included in this example application to demonstrate how it can be used to create a simple form for issuing RSH certificates. As distributed, C-Menu simply creates a file, /etc/pam.d/rsh_auth containing "auth required pam_permit.so".


There are many ways to implement authentication and authorization


---

## Workstation Configuration

```
:   Workstation Configuration
!menu workstation_config.m
```

- This menu selection launches a sub-menu.

---

## Diagnostic Utilities

```
:   Diagnostic Utilities
!menu diag.m
```

- This menu selection launches a sub-menu.

---

## Installment Loan Calculations

```
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -R "view -L60 -C62 -Nf -S \"amort %%\"" -o iloan.dat
```

- This menu selection launches a simple installment loan calculator that solves
for a fourth variable given the other three.

- The -S option specifies a command to run when the user commits a loan record. In this case, it runs the amort, a command that prints an amortization schedule for the selected loan.

---

## Cash Receipts

```
:     Cash Receipts
!form receipt.f -i receipt.dat -o receipt.dat
```

- This menu selection launches a simple cash receipts form that allows the user
  to enter a cash receipt transaction and save it to a file. If it were part of
a real accounting application, form would have a -c option specifying a program
or script that would process the cash receipt and post it to the general ledger.

---

## Rustlings Source

```
:     Rustlings Source
!pick -S "lf -S rustlings -d3 \".*exercises.*\.rs$\"" -v -n 1 -T "Rustlings Source - Edit" -c "nvim %%"
```

- For anyone learning Rust, the Rustlings exercises are a great way to practice
  Rust programming. This menu selection launches a file picker that allows the
user to quickly find and select a Rustlings source file to edit in the default
editor. In this example, we use nvim.

- The opening Pick window displays a list of all 94 Rustlings source files. The
user can tab to the Search field and enter a search string to filter the list of
files. The file highlighted by the Selector bar will be opened in View instantly
and displayed below the Pick window. The user can browse the file in View, or
press the space bar to edit the file in nvim. When the user exits nvim, control
returns to the Pick window, which allows the user to select another file to
edit. The Rustlings exercises are generally small and fairly simple, but very
powerful learning tools because they provide practical experience with the Rust
programming language.

- With this menu option using C-Menu, I have seen students finish 30 or more
exercises in an afternoon, and they have a lot of fun doing it. The Rustlings
exercises are a great way to learn Rust, and C-Menu makes it easy to find and
edit the source files.

- Hats off to the Rustlings team for creating such a great learning resource.

- To begin your Rust journey:

1. Get the book: [The Rust Programming Language](https://doc.rust-lang.org/book/)
2. Install Rustup: [Rustup](https://rustup.rs/)
3. Install Rustlings:

```
cargo install rustlings
```

Enjoy!


## View Manual Pages

```
:     -PView Manual Pages
!pick -S "listman.sh" -n 1 -T \"Select Manual Page to View\" -c "readman.sh %%"
```
---

## Edit C-Menu Description Files

```
:     Edit C-Menu Description Files
!pick -S list_msrc -n1 -T "C-Menu Description Files - Select File to Edit" -c edit_msrc %%
```

- This menu selection uses lf to display a list of all C-Menu description files
in the application's msrc directory. The user can select a file to edit in the
default text editor. In this example, we use nvim.

---

## View C-Menu Source with Tree-Sitter

```
:     -SView C-Menu Source with Tree-Sitter
!pick -S project_src -n 1 -T "Select Project Source to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
```

- This menu selection uses lf to display a list of .c and .h files in the C-Menu source directory. To use this option, you will need to have tree-sitter-cli installed and the tree-sitter highlight command available in your path. The user can select a source file to view with syntax highlighting provided by tree-sitter.

- You will also need to have the tree-sitter grammars for the languages you want
to highlight installed. You can find the grammars on the tree-sitter GitHub
page: [Tree-Sitter Grammars](https://github.com/tree-sitter-grammars).

---

## View Source with Tree-sitter

```
:     -TView Source with Tree-Sitter
!pick -S "lf -S -d 5 . \".*\.(rs|c|h|sh|lua|py|cpp|js|html|css)$\"" -T "Select Source File to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
```

- This menu selection uses lf to display a list of source files in the current
directory and its subdirectories. The user can select a source file to view with syntax highlighting provided by tree-sitter.

- In addition to tree-sitter-cli, you will also need to have the tree-sitter grammars for the languages you want to highlight installed. You can find the grammars on the tree-sitter GitHub page: [Tree-Sitter Grammars](https://github.com/tree-sitter-grammars)
---

### View LSP Log

```
:   View LSP Log
!view -L60 -C80 /home/bill/.local/state/nvim/logs/lsp.log
```

- This menu selection uses C-Menu's view utility to display the log file for the Language Server Protocol (LSP) used by Neovim. The log file is located at $HOME/.local/state/nvim/logs/lsp.log. The -L option specifies the number of lines to display, and the -C option specifies the number of columns to display.

- The LSP log has long lines. In view set the horizontal scroll width to about half a page (50 columns if your terminal is 100 columns wide). You can also expand your View window by expanding your terminal window.

- Beware: The LSP log can be very large, and you may not be able to load in vim or nvim, and if you do it will be very slow and unwieldy. No problem for view. It eats multi-gigabyte files for breakfast. You can scroll through the log file quickly and easily, and search for specific entries using the built-in search functionality, and you can do so lickety-split. (How is that for an onomatopoeic idiom?)

---

## Help Menu

```
:     Help Menu
!menu help.m
```

- This menu selection opens a sub-menu with a list of help topics.

---

## Menu Description With Bat Syntax Highlighting

```
:     -BMenu Description With Bat Syntax Highlighting
!view -Nf -L 39 -C 85 -S "bat --theme ansi -l Crystal -f ~/menuapp/msrc/main.m"
```

- You will need to have bat installed and the bat command available in your path
  to use this menu selection. -Nf tells view not to display line numbering as
bat does its own line numbering. -L and -C specify the number of lines and
columns respectively.

- Install bat with the following command:

```
cargo install --locked bat
```

---

## View C-Menu Command Line Options

```
:     -OView C-Menu Command Line Options
!view -Nf -L66 -C75 ~/menuapp/help/menu.help
```

- This menu selection displays C-Menu's command line options. The -Nf option
suppresses line numbers in view.


---

## View Highlighted view_engine.c

```
:     -eView Highlighted view_engine.c
!view -N -L66 -C85 ~/menuapp/help/view_engine.c
```

- This menu option displays the C-Menu source file view_engine.c with syntax
highlighting by tree-sitter. 

---

## Exit Applications

- This selection is not needed because it is already built in to every C-Menu menu. For sub-menus, the letter q is reserved as the hotkey to exit the sub-menu and return to the parent menu. For the root menu, the hotkey q exits the application. However, we include it here to demonstrate that you can add your own exit selection to the menu if you like. 

```
:     Exit Applications
!return
```
