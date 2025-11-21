#!/bin/bash

rm -f CMakeCache.txt
rm -f ../src/compile_commands.json
rm -rf CMakeFiles
cmake ../src --install-prefix /home/bill/menuapp/bin \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# -DCMAKE_CXX_COMPILER=/usr/bin/clang++
ln -s "$PWD"/compile_commands.json /usr/local/src/C-Menu-0.2.4/src
