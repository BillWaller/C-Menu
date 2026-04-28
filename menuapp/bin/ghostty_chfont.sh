#!/bin/bash
# @file ghostty_chfont.sh
# @brief link selected font configuration file to default_font

U=$(id -un)
G=$(id -gn)
fonts_dir="$HOME"/.config/ghostty/fonts
if [ ! -d "$fonts_dir" ]; then
    echo "$0"
    echo "can't find $fonts_dir"
    echo "make sure you have ghostty installed and run it at least once"
    echo "put your font configuration files in $fonts_dir and then"
    echo "run this script with the name of the font you want to use as an argument"
    exit 1
fi
rsh -c "chown -R $U:$G $fonts_dir"
cd "$fonts_dir"
ln -sf "$1" default_font
