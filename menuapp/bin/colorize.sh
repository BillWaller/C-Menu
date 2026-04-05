#!/bin/bashbug

if [ $(which awk) ] -a [ -r "$HOME"/menuapp/msrc/colorize.awk" ]; then
    awk -f "$HOME"/menuapp/msrc/colorize.awk "$1" >"$1".out
    mv "$1".out "$1"
else
    if [ ! $(which awk) ]; then
        echo "can't locate awk"
    fi
    if [ ! -f "$HOME"/menuapp/msrc/colorize.awk ]; then
        echo "can't locate "$HOME"/menuapp/msrc/colorize.awk"
    fi
    enterchr "Press any key to continue..."
fi
