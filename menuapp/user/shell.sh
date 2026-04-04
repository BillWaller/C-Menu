#!/bin/bash

# This script sets up the user environment for the menu application.
# It configures necessary environment variables and sources required scripts.
# It should be executed in the user's shell session.
#
# Usage:
#   source /path/to/menuapp/user/shell.sh

prepend_path() {
    case ":${PATH}:" in
    *:"$1":*) ;;
    *)
        PATH="$1:$PATH"
        ;;
    esac
}
[ -d "$HOME/menuapp/bin" ] && prepend_path "$HOME/menuapp/bin"
