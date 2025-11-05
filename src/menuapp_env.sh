#!/bin/bash

# This script sets up the user environment for the menu application.
# It configures necessary environment variables and sources required scripts.
# It should be executed in the user's shell session.
#
# Usage:
#   source /path/to/menuapp/user/shell.sh
echo
echo
cat <<'EOF' >"$HOME/.menuapp_env"
prepend_path() {
    case ":${PATH}:" in
    *:"$1":*) ;;
    *)
        PATH="$1:$PATH"
        ;;
    esac
}
[ -d "$MAPP_HOME/bin" ] && prepend_path "$HOME/menuapp/bin"
export "$HOME"/.minitrc
export MAPP_DATA="$MAPP_HOME"/data
export MAPP_USER="$MAPP_HOME"/user
export MAPP_HELP="$MAPP_HOME"/help
EOF
cat "$HOME"/.menuapp_env
echo
echo "  Menu application environment setup complete."
echo
echo "  To apply the changes, run:"
echo
echo "      source "\"$HOME\""/.menuapp_env"
echo
./enterchr "Press any key to continue..."
echo
