# ![C-Menu](screenshots/C-Menu.png)
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

#### Prerequisites

- One of:
    - CMake 3.20 or higher, or
    - GNU Make 4.3 or higher
- A C++ compiler that supports C++17 or later
- NCursesw development libraries 6.5 or later
- GNU GLIBC development files
- GNU Math Library (libm) development files

#### Option 1 - Build C-Menu Using CMake Directly

1. Create a build directory and navigate into it:

```bash
cd C-Menu/src
mkdir build
cd build
cmake ..
```

2. Build the project using:

** Note ** Make sure to replace `/home/yourusername/menuapp` with the actual path where you want to install C-Menu.

```bash
cmake -DCMAKE_INSTALL_PREFIX=/home/yourusername/menuapp \
    -DCMAKE_BUILD_TYPE=Release ..
make
```

3. Install the built binaries and resources:

```bash
make install
```

4. If you want to use rsh in setuid mode, you need to change its owner and
   permissions:

```bash
sudo chown root:root /home/yourusername/menuapp/bin/rsh
sudo chmod 4711 /home/yourusername/menuapp/bin/rsh
```

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

### Finish the installation

1. Add the C-Menu bin directory to your PATH environment variable by adding the following line to your shell profile (e.g., ~/.bashrc or ~/.zshrc):

```bash
export PATH="/home/yourusername/menuapp/bin:$PATH"
```

2. Copy the sample minitrc from the C-Menu/menuapp directory to your home directory:

```bash
cp /home/yourusername/menuapp/share/minitrc /home/yourusername/.minitrc
```

3. Edit the ~/.minitrc file to customize your C-Menu configuration as needed.
4. Start C-Menu by running the following command in your terminal:

```bash
menu
```

 🐸  Enjoy using C-Menu! If you encounter any issues or have questions, feel free to open an issue on the C-Menu GitHub repository.
