#!/usr/bin/env bash

# This script is used to build the project using CMake with different UI options.

if [ ! -d build ]; then
    mkdir -p build
fi
cd build

# To build C-Menu with the NCurses library:
# echo "cmake .. -D UI=UAL_UI">UI.cmk

# To build C-Menu with the Notcurses library
# echo "cmake .. -D UI=NOTCURSES_UI">UI.cmk

. ../UI.cmk

# compile_commands.json is needed for clangd to work properly
cp compile_commands.json ..
UI="$(sed 's/^.*=//' ../UI.cmk)"
# copy compile_commands.json to ncurses or notcurses version to
# facilitate chkui -s
if [ "$UI" == "UAL_UI" ] || [ "$UI" == "NCURSES_UI" ]; then
    cp compile_commands.json ../compile_commands_ncurses.json
else
    cp compile_commands.json ../compile_commands_notcurses.json
fi
# Using GNU Makefile
#
# Edit the Makefile and uncomment:

# to build with the NCurses library

# UI=UAL_UI

# to build with the Notcurses library

# UI=NOTCURSES_UI

# Then run:

# make
# make install
