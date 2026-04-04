#!/bin/bash
U=$(id -un)
G=$(id -gn)
fonts_dir="$HOME"/.config/ghostty/fonts
rsh -c "chown -R $U:$G $fonts_dir"
cd "$fonts_dir"
for font in $(lf -d 1 -t f -e '.*default_font$' . '.*' | sort); do
    echo $(basename "$font")
done
