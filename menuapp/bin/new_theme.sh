#!/bin/bash

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 <theme-file>"
    enterchr "Press any key to continue"
    exit 1
fi
cd "$CMENU_HOME/themes" || exit 1
if [ ! -f "$1" ]; then
    echo "Error: File '$1' not found."
    enterchr "Press any key to continue"
    exit 1
fi
TMPFILE=$(mktemp New.XXXXX) || exit 1
cp "$1" "$TMPFILE" || exit 1
if [ "$EDITOR" != "" ]; then
    $EDITOR "$TMPFILE"
else
    if [ "$(which nvim)" ]; then
        nvim "$TMPFILE"
    else
        if [ "$(which vim)" ]; then
            vim "$TMPFILE"
        else
            if [ "$(which nano)" ]; then
                nano "$TMPFILE"
            else
                echo Cannot find EDITOR "($EDITOR)"
                echo Cannot find nvim, vim, or nano.
                echo Please install an editor and try again.
                enterchr Press any key to exit...
                exit 1
            fi
        fi
    fi
fi
tput smcup
