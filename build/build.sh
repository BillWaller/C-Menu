#!/bin/bash

# If you already have a Makefile, you can just run:
# cmake --build .
# But if you need to reconfigure, run this script.
#
user=$(id -un)
group=$(id -gn)
rootuser=$(id -un 0)
rootgroup=$(id -gn 0)
export CMAKE_INSTALL_PREFIX="$HOME/menuapp"

rm -f CMakeCache.txt
rm -f ../src/compile_commands.json
rm -rf CMakeFiles
cmake ../src --install-prefix "$CMAKE_INSTALL_PREFIX" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=/usr/bin/clang
cmake --build .
cmake -S ../src -B .
echo "chown $user:$group $CMAKE_INSTALL_PREFIX/bin/*" >post_install.sh
echo "chown $rootuser:$rootgroup $CMAKE_INSTALL_PREFIX/bin/rsh" >>post_install.sh
echo "chmod 4711 $CMAKE_INSTALL_PREFIX/bin/rsh" >>post_install.sh
chmod a+x post_install.sh
echo "Build complete."
echo "To install: sudo ./install.sh"
