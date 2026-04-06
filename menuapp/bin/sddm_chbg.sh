#!/bin/bash
# @brief This script changes the background of the sddm login screen
# @param $1 The file name of the new background image, which should be located
# in the sddm backgrounds directory.
#
# @code
# # ~/menuapp/msrc/workstation-config.m
# :   Select SDDM Background
# !pick -n 1 -T "Select SDDM Background" -S sddm_bg.sh -c "sddm_chbg.sh %%"
# @endcode

U=$(id -un)
G=$(id -gn)
bkgd_dir="/usr/share/sddm/themes/sddm-corporate-theme/Backgrounds"
if [ ! -d "$bkgd_dir" ]; then
    echo "$0"
    echo "can't find Background directory: $bkgd_dir"
    echo "You must have sddm-corporate-theme installed to use this script."
    echo "Download from:"
    echo https://github.com/BillWaller/sddm-corporate-theme.git
    enterchr "Press any key to continue..."
    exit 1
fi
rsh -c "chown -R $U:$G $bkgd_dir"
cd "$bkgd_dir"
ln -sf "$1" default.png
