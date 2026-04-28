#!/bin/bash
# @file kitty_chtheme.sh
# @brief link selected theme configuration file to default_theme

U=$(id -un)
G=$(id -gn)
themes_dir="$HOME"/.config/kitty/themes
ln -sf "$1" default_theme
if [ ! -d "$themes_dir" ]; then
    echo "$0"
    echo "can't find $themes_dir"
    echo "make sure you have kitty installed and run it at least once"
    echo "put your font configuration files in $themes_dir and then"
    echo "run this script with the name of the font you want to use as an argument"
    enterchr "Press any key to continue..."
    exit 1
fi
rsh -c "chown -R $U:$G $themes_dir"
cd "$themes_dir"
