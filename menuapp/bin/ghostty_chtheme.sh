#!/bin/bash

U=$(id -un)
G=$(id -gn)
themes_dir="$HOME"/.config/ghostty/themes
if [ ! -d "$themes_dir" ]; then
    echo "$0 ghostty themes directory, $themes_dir not found."
    echo "Please run ghostty at least once to create the directory and
    echo "install theme configurations in $themes_dir."
    enterchr "Press any key to continue..."
    exit 1
fi
rsh -c "chown -R $U:$G $themes_dir"
cd "$themes_dir"
ln -sf "$1" default_theme
