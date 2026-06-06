#!/bin/bash
# @file cmenu_themes.sh
# @brief list cmenu theme configuration files
#
U=$(id -un)
G=$(id -gn)
themes_dir="$CMENU_HOME"/config
if [ ! -d "$themes_dir" ]; then
    echo "$0"
    echo "can't find $themes_dir"
    echo "make sure you have cmenu installed and run it at least once"
    echo "put your cmenu theme configuration files in $themes_dir and then"
    echo "run this script with the name of the theme you want to use as an argument"
    enterchr "Press any key to continue..."
    exit 1
fi
# rsh -c "chown -R $U:$G $themes_dir"
cd "$themes_dir" || exit
for theme in $(lf -t f | sort); do
    echo $(basename "$theme")
done
