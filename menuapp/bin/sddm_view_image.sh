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
cd "$preview_dir"
magick display -size 600x600 "$1"
