#!/bin/bash
# @name colorize.sh
# @brief Enclose six-digit hex color strings in ANSI colorizing sequences
#
if
    [ "$(which awk)" ] && [ -r "$HOME"/menuapp/bin/colorize.awk ]
then
    awk -f "$HOME"/menuapp/bin/colorize.awk "$1" >"$1".out
    mv "$1".out "$1"
else
    if [ ! "$(which awk)" ]; then
        echo "can't locate awk"
    fi
    if [ ! -f "$HOME"/menuapp/bin/colorize.awk ]; then
        echo cannot locate "$HOME"/menuapp/bin/colorize.awk
    fi
    enterchr "Press any key to continue..."
fi
