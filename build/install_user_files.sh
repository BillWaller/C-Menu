#!/bin/bash

# This script installs user-specific configuration files and directories.
# It creates necessary directories and copies configuration files to the user's home directory.
#
# Usage: ./install_user.sh
# Make sure to run this script with appropriate permissions.
#
export PREFIX="$HOME"/menuapp
# cp -Rpdu ../menuapp "$HOME"
# cp ../menuapp/minitrc "$HOME"/.minitrc

echo "Installation complete."
echo "You can start the application by running 'menu' from your terminal."
echo ""
echo "If you have installed RSH with setuid privileges, you should consider"
echo "installing the following lines in your ~/.bashrc."
echo "--------------------------------------------------------------------"
sed '1,4d' cmenu_bashrc.sh
echo "--------------------------------------------------------------------"
echo
echo "You can do so by running: ./cmenu_bashrc.sh"
