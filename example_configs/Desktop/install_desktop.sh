#!/bin/bash
# Uncomment the desktop files you want to install below
# Make sure you have the proper privilege level
if [ ! -d /usr/share/pixmaps ]; then
    mkdir -p /usr/share/pixmaps
fi
cp ../../screenshots/Decision.svg /usr/share/pixmaps
# cp cmenu_ghostty.desktop "$HOME"/Desktop
# cp cmenu_kitty.desktop "$HOME"/Desktop
# cp cmenu_Alacritty.desktop "$HOME"/Desktop
