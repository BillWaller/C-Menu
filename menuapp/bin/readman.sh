#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: $0 <manpage>"
    exit 1
fi
cd "$HOME"/menuapp/man
echo "1- view -L60 -C80 -Nf $1" >/home/bill/x
if [ -f "$1" ]; then
    echo "2- view -L60 -C80 -Nf $1" >>/home/bill/x
    view -L60 -C80 -Nf "$1"
fi
exit 0
