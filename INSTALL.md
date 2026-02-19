# Installation Guide

<!-- mtoc-start -->

* [To Install C-Menu](#to-install-c-menu)
  * [Build C-Menu](#build-c-menu)
    * [RSH Static Linking](#rsh-static-linking)
    * [Prerequisites](#prerequisites)
    * [Option 1 - Build C-Menu Using CMake Directly](#option-1---build-c-menu-using-cmake-directly)
    * [Option 2 - Build C-Menu with Provided Scripts](#option-2---build-c-menu-with-provided-scripts)
    * [Option 3 - Build C-Menu Using Makefile](#option-3---build-c-menu-using-makefile)
  * [Finish the installation](#finish-the-installation)

<!-- mtoc-end -->

## To Install C-Menu

1. Clone the C-Menu repository:

```bash
gh repo clone BillWaller/C-Menu
```

2. Copy the menuapp directory to your desired location:

```bash
cp -r C-Menu/src/menuapp /home/yourusername/
```

### Build C-Menu

#### RSH Static Linking

C-Menu uses dynamic linking by default, but if you plan to use rsh in a rescue environment where dynamic linking may not be available, you can statically
link rsh during the build. To do this, set the `RSH_LD` environment variable to `-static` before building C-Menu:

```bash
export RSH_LD="-static"
```

CMake or Makefile will strip symbols from the executable once it has been copied
to its destination directory. This is done to reduce the size of the executable and improve performance. If you want to keep the symbols for debugging purposes.

***NOTE*** If you choose to statically link rsh, make sure that your C compiler and linker support static linking and that you have the necessary static libraries,
specifically, libc.a, installed on your system. Static linking can increase the size of the executable and may have implications for compatibility and security, so be sure to test the statically linked version of C-Menu in your target environment.

Most distributions provide static libraries for the GNU C Library (glibc) as part of their development packages. You may need to install additional packages to obtain these static libraries, such as `glibc-static` or `libc6-dev` for glibc.

#### Prerequisites

- One of:
    - CMake 3.20 or higher, or
    - GNU Make 4.3 or higher
- A C compiler that supports C23 or later (e.g., GCC 13 or later, Clang 15 or later)
- NCursesw development libraries 6.5 or later
- GNU GLIBC development files
- GNU Math Library (libm) development files

#### Option 1 - Build C-Menu Using CMake Directly

1. Navigate to the C-Menu/src directory, create a build directory, and
cd into it:

```bash
cd C-Menu/src
mkdir build
cd build
```

2. Configure the project using CMake, specifying the installation prefix and build type:

```bash
cmake -DCMAKE_INSTALL_PREFIX="$HOME"/menuapp -DCMAKE_BUILD_TYPE=Release ..
```

3. Build the project using:

** Note ** Make sure to replace `/home/yourusername/menuapp` with the actual path where you want to install C-Menu.

```bash
make
```

4. If you want to use rsh in setuid mode, you must install C-Menu with root
   privileges.

```bash
sudo make install
```

go to [Finish the installation](#finish-the-installation) below to complete the installation process.

#### Option 2 - Build C-Menu with Provided Scripts

1. Navigate to the C-Menu/build directory and run the provided build script:

```bash
cd C-Menu/build
./build.sh
```

2. Assume root privileges to install C-Menu:

```bash
sudo ./install.sh
```

go to [Finish the installation](#finish-the-installation) below to complete the installation process.

#### Option 3 - Build C-Menu Using Makefile

1. Navigate to the C-Menu/src directory and edit the provided Makefile
 to set the installation PREFIX to your desired location (e.g., /home/yourusername/menuapp):

```bash
cd C-Menu/src
```

```Makefile
USER=yourusername
GROUP=yourgroup
HOME=/home/$(USER)
PREFIX=/home/$(USER)/menuapp
```

2. Build the project using:

```bash
make
```

3. Assume root privileges to install C-Menu:

```bash
sudo make install
```

Continue with [Finish the installation](#finish-the-installation) below to complete the installation process.

### Finish the installation

1. Register the C-Menu libraries with the dynamic linker by running the following command:

```bash
sudo ldconfig -v "$HOME"/menuapp/lib64
```

2. Add the C-Menu bin directory to your PATH environment variable by adding the following line to your shell profile (e.g., ~/.bashrc or ~/.zshrc):

```bash
export PATH="/home/yourusername/menuapp/bin:"$PATH"
```

3. Copy the sample minitrc from the C-Menu/menuapp directory to your home directory:

```bash
cp "$HOME"/menuapp/minitrc "$HOME"/.minitrc
```

3. Edit the ~/.minitrc file to customize your C-Menu configuration as needed.
4. Start C-Menu by running the following command in your terminal:

```bash
vi ~/.minitrc
```

🐸  Enjoy using C-Menu! If you encounter any issues or have questions, feel free to open an issue on the C-Menu GitHub repository.
