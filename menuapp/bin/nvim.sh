#!/bin/bash

if [ $(which nvim) ]; then
    nvim "$@"
else
    if [ $(which vim) ]; then
        vim "$@"
    else
        echo "Comeon man! Neither nvim nor vim is installed."
        enterchr "Press any key to exit..."
        exit 1
    fi
fi
tput smcup
