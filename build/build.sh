#!/bin/bash

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../src
# -DCMAKE_CXX_COMPILER=/usr/bin/clang++
ln -s "$PWD"/compile_commands.json /usr/local/src/cmenu/src
