#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <command>"
    exit 1
fi
if [ ! -x "$1" ]; then
    echo "Error: Command '$1' not found"
    exit 1
fi
name="$1"
if [ ! -f "$name"_help ]; then
    if [ -f "$name".help ]; then
        echo copying "$name".help to "$name"_help
        cp "$name".help "$name"_help
    else
        echo "Error: '$name'_help not found"
        exit 1
    fi
fi
bat --theme ansi -l Crystal -f "$name"_help >"$name".help
echo created "$name".help
