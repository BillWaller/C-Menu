#!/bin/bash
U=$(id -un)
G=$(id -gn)
themes_dir="$HOME"/.config/kitty/themes
rsh -c "chown -R $U:$G $themes_dir"
cd "$themes_dir"
for theme in $(lf -d 1 -t f -e '.*default_theme$' . '.*' | sort); do
    echo $(basename "$theme")
done
