#!/bin/bash -i
# @file cmenu_chtheme.sh
# @brief link selected theme configuration file to default

U=$(id -un)
G=$(id -gn)
themes_dir="$CMENU_HOME"/themes
if [ ! -d "$themes_dir" ]; then
    echo "$0"
    echo "can't find $themes_dir"
    echo "make sure you have cmenu installed and run it at least once"
    echo "put your theme configuration files in $themes_dir and then"
    echo "run this script with the name of the theme you want to use as an argument"
    enterchr "Press any key to continue..."
    exit 1
fi
cd "$themes_dir" || exit 1
ln -sf "$1" default
