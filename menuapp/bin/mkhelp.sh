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
$name --version >"$name"_help
$name -? >>"$name"_help
$name --usage >>"$name"_help
echo created "$name"_help

bat --theme ansi -l Crystal -f "$name"_help >"$name".help
echo created "$name".help

if [ "$name" == "lf" ]; then
    lf -pxwrsg -u bill -L -H -S -t f -a 2026-04-01T00:00:00 -b 2026-05-01T00:00:00 -D18 -e '.*\.[ch]$' 2>"$name"_debug
    echo created "$name"_debug
    bat --theme ansi -l Crystal -f "$name"_debug >"$name".debug
    echo created "$name".debug
fi
