# CMENU - A TUI Menu System

## Featuring Menu, Form, Pick, View, and RSH

CMENU is a lightweight, customizable, and easy-to-learn suite of programs
for quickly and easily creating menus, entry forms, and pickers with a text-based user interface(TUI) for applications running on Linux and Unix-like operating systems. CMENU is designed to be simple to use while providing powerful features for developers who need to implement menu driven frameworks for user applications in terminal environments.

Replace the GUI nonsense with a real text-based terminal interface just like the real nerds. Back to the Darkscreen!

## Programs Overview

MENU reads a simple description file like the one below and displays a context menu to the user. When the user selects an item, MENU executes the corresponding command. It's like writing a shell script, but with a nice TUI menu interface.

## Features

- Lightweight and fast
- Highly customizable appearance and behavior
- Easy integration with existing applications
- Supports multiple menu styles and themes
- Simple and intuitive API

## Installation

To install CMENU, simply download the source code from the repository and follow the installation instructions provided in the INSTALL.md file.

## Examples

### Sample Menu Description File

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

### Sample Menu With Curses Keys

<img src="/usr/local/src/cmenu/src/screenshots/Curses_Keys.png" style="width: 80%" />

<img src="/usr/local/src/cmenu/src/screenshots/Pick.png" style="width: 80%" />

<img src="/usr/local/src/cmenu/src/screenshots/Receipt.png" style="width: 80%" />

<img src="/home/bill/Document_root/work.png" style="width: 80%" />
