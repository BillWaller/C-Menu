#!/bin/bash

U=$(id -un)
G=$(id -gn)
themes_dir="$HOME"/.config/alacritty/themes
rsh -c "chown -R $U:$G $themes_dir"
cd "$themes_dir"
ln -sf "$1" default_theme
