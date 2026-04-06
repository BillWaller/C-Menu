#!/bin/bash
# @brief This script displays a Preview image of the sddm login screen
# @param $1 The file name of the preview image.
#
# @code
# # ~/menuapp/msrc/workstation-config.m
# :   Preview SDDM Background
# !pick -n 1 -T "Preview SDDM Background" -S sddm_preview.sh -c
# "sddm_view_image.sh %%"
# @endcode

U=$(id -un)
G=$(id -gn)
preview_dir="/usr/share/sddm/themes/sddm-corporate-theme/Backgrounds"
if [ ! -d "$preview_dir" ]; then
    echo "$0"
    echo "can't find Preview directory: $preview_dir"
    echo "You must have sddm-corporate-theme installed to use this script."
    echo "Download from:"
    echo https://github.com/BillWaller/sddm-corporate-theme.git
    enterchr "Press any key to continue..."
    exit 1
fi
cd "$preview_dir"
magick display -size 600x600 "$1"
