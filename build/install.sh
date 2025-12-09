#!/bin/bash

export CMAKE_INSTALL_PREFIX="$HOME/menuapp"
cmake --install .
./post_install.sh
