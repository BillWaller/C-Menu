#!/bin/bash

IFS="
"

for line in $(echo "BLACK    000000
RED      FF3f3f
GREEN    4ff07f
YELLOW   FFef4f
BLUE     5faFff
MAGENTA  f077f0
CYAN     8fdFfF
WHITE    FF8f5f
BBLACK   bfbfGf
BRED     FF7f00
BGREEN   00FFa0
BYELLOW  FFcf00
BBLUE    005fFF
BMAGENTA FF00FF
BCYAN    00ffff
BWHITE   e0d0d0
BG 000720
FG e0d0d0
cursor f0f0f0
selbg e0d0d0
selfg 000000"); do
    set -- $line
    echo hextoint "$1" "$2"
done
