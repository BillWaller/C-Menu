# CMake Build

## Building the Menu System

- To build with Cmake and clang:

From the top level directory of this distribution:

```cd build
$ ./build.sh
$ make
$ make install
```

- To build using gnu make and gnuc:

From the top level directory of this distribution:

```
$ cd src
$ make
$ make install
```

> STOP 🛑 Read this! It applies to the Makefile in the "src" directory, and could
> leave a glaring vulnerability in your system. If your target system will be available
> to other users, you will need to change the permissions on "rsh" from 4711 to 0711
> in the install section of src/Makefile. To do this, you will need to edit the
> Makefile before running make install.

```
Change the last line of the install section.

               program   target directory          perms
               -------   ----------------          -----
From ./instexe rsh       $(PREFIX)/bin    rsh       4711    bin    bin

To:  ./instexe rsh       $(PREFIX)/bin    rsh       0711    bin    bin
```

"rsh" is simply a convenience tool to allow developers to quickly switch to
superuser mode without having to use "sudo -s" or "su -". It is not required for
normal operation of the menu system.

## Running the Menu System

- To start the menu system:

```
$ menu
```

## Configuring the Menu System

The edit the menu system configuration:

```
$ vi ~/.minitrc
```

Application files are in ~/menuapp

To edit the default top-level application description file:

```
 vi ~/menuapp/main.m
```

```
: APPLICATIONS
:
: Documentation
!menu doc.m
: System Setup
!menu setup.m
: More Applications
!menu appl.m
: Test Curses Keys
!ckeys
: Write Configuration File
!write_config
: Install Configuration File
!~/menuapp/inst_config ~/menuapp/shell_msg
: Pick Edit a File
!cpick -i picklist -c vi picklist.out
: Display Banner
!form banner.d
:
: Help
!view main.h
:
: Exit Applications
!return
```

~/menuapp/banner.d uses the menu system's form application to to receive user
input for an ascii-art banner message to be displayed on the screen.
To edit the banner description file:

```
$ vi ~/menuapp/banner.d
```

```
:1:0:DISPLAY BANNER
:3:5:Enter Banner Text
!8!7!30!2!/usr/local/brt/cmenu/src/banner.sh
```

Lines beginning with : are display text.

Lines beginning with ! are commands for menu system.

Numbers within line segments are generally row and column positions. In the last
line of banner.d, the numbers correspond to row, column, width, and validation type.
The remainder of the line is a command to be executed with the user input as
an argument.

This is a work in progress. Additional features and documentation will be added
if there is sufficient interest. Please send comments and suggestions to
billxwaller@gmail.com.

This program is distrubuted under the terms of the MIT lICENSE. See the file LICENSE
in the type level directory of this distribution for details.
