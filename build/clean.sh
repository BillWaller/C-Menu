#!/bin/bash

cmake --build . --target clean
rm -f CMakeCache.txt ../src/compile_commands.json compile_commands.json \
    cmake_install.cmake install_manifest.txt post_install.sh Makefile
rm -rf CMakeFiles .cmake
