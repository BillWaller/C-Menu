#!/bin/bash
# @file sddm_bg.sh
# @brief sddm_bg.sh is part of the C-Menu selection, "Select SDDM Background".
# It provides a list of available background images for the sddm login screen.
# @details The script is designed to be called from a C-Menu application, to
# provide a list of available SDDM backgrounds as input for C-Menu Pick.
# @code
# # ~/menuapp/msrc/workstation-config.m
# :   Select SDDM Background
# !pick -n 1 -T "Select SDDM Background" -S sddm_bg.sh -c "rsh -c ln -sf %%
# #endcode
# @note First, it changes the ownership of the background directory to the
# current user and group, which may be necessary to read the directory and link
# default.png to the selected background.
#
# Upon completion of the pick command, another bash script, sddm_chbg.sh will
# be called with the name of the selected background file, which it will
# link to default.png.
#

U=$(id -un)
G=$(id -gn)
preview_dir="/usr/share/sddm/themes/sddm-corporate-theme/Previews"
if [ ! -d "$preview_dir" ]; then
    echo "$0 can't find Preview directory: $preview_dir"
    echo "You must have sddm-corporate-theme installed to use this script."
    echo "Download from:"
    echo https://github.com/BillWaller/sddm-corporate-theme.git
    enterchr "Press any key to continue..."
    exit 1
fi
cd "$preview_dir"
for preview in $(lf -e '.*default\.png$' . '.*\.png$' | sort); do
    sxiv -g 930x540 $(basename "$preview") &
done
