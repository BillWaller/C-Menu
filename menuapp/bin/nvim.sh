#!/bin/bash

if [ "$EDITOR" != "" ]; then
    $EDITOR "$@"
else
    if [ "$(which nvim)" ]; then
        nvim "$@"
    else
        if [ "$(which vim)" ]; then
            vim "$@"
        else
            if [ "$(which nano)" ]; then
                nano "$@"
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
