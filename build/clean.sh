#!/bin/bash

if [ -f CMakeCache.txt ]; then
    cmake --build . --target clean
fi
rm -f CMakeCache.txt ../src/compile_commands.json compile_commands.json \
    cmake_install.cmake install_manifest.txt post_install.sh Makefile \
    .prefix CMenu.conf
rm -rf CMakeFiles .cmake
