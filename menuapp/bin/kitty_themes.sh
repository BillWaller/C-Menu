#!/bin/bash
U=$(id -un)
G=$(id -gn)
themes_dir="$HOME"/.config/kitty/themes
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
for theme in $(lf -d 1 -t f -e '.*default_theme$' . '.*' | sort); do
    echo $(basename "$theme")
done
