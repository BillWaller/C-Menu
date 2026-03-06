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
rsh -c "chown -R $U:$G $bkgd_dir"
cd "$bkgd_dir"
ln -sf "$1" default.png
echo "ln -sf $1 default.png" >/home/bill/xx
