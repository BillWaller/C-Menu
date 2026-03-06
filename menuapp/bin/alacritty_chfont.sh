#!/bin/bash

U=$(id -un)
G=$(id -gn)
fonts_dir="$HOME"/.config/alacritty/fonts
rsh -c "chown -R $U:$G $fonts_dir"
cd "$fonts_dir"
ln -sf "$1" default_font
