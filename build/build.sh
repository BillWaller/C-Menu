#!/bin/bash
# If you already have a Makefile, you can just run:
# cmake --build .
# But if you need to reconfigure, run this script.
#
user=$(id -un)
group=$(id -gn)

export USER=bill
export GROUP=bill
HOME=/home/$USER

rootuser=$(id -un 0)
rootgroup=$(id -gn 0)
echo "$HOME/menuapp" >.prefix
export PREFIX=$HOME/menuapp
export CMAKE_INSTALL_PREFIX="$PREFIX"
rm -f CMakeCache.txt
rm -f ../src/compile_commands.json
rm -rf CMakeFiles
cmake ../src \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_INSTALL_PREFIX="$PREFIX"
cmake --build .
cmake -S ../src -B .
echo "chown $user:$group $PREFIX/bin/*" >post_install.sh
echo "chown $rootuser:$rootgroup $PREFIX/bin/rsh" >>post_install.sh
echo "chmod 4711 $PREFIX/bin/rsh" >>post_install.sh
echo "LS=\$(cat install_manifest.txt)" >>post_install.sh
echo "which lsg >/dev/null 2>&1 && lsd -l \$LS || ls --color=always -l \$LS" >>post_install.sh
echo "ldconfig $PREFIX/lib64" >>post_install.sh
chmod a+x post_install.sh
echo "Build complete."
echo "To install: sudo ./install.sh"
