#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: $0 <manpage>"
    exit 1
fi

man -Tutf8 "$1" | sed -f ~/menuapp/msrc/man.sed | view
