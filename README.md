### C-Menu - A User Interface Toolkit

![](screenshots/installation-guide.png)

# Introduction

The following is a brief introduction to the C-Menu Toolkit covering a fraction of its features and capabilities. Who is C-Menu for? Anyone who demands performance with a professional look and feel for their applications, without the overhead of a graphical user interface. C-Menu is especially well suited for rescue, embedded, server, development, and other resource-constrained environments, but it also also appeals to the purist who wants powerful, efficient, and user-friendly applications without the bloat of a GUI.

For more detailed information, please refer to the html documentation at:

[decision-inc.com](https://decision-inc.com)

With the C-Menu Toolkit, you can quickly and easily develop menu-driven user
interfaces to give your applications a professional look and feel. The main
components are:

**_Menu_** - Hierarchical menus

![Hierarchical Menus](screenshots/workstation_config2.png)

**_Form_** - On-screen forms for entering, editing, validating, processing, and submitting data. Notice the chyron at the bottom of the screen, which provides helpful instructions and feedback to the user. Of course, all C-Menu components provide navigation by mouse and keyboard, and in many cases by the standard h, j, k, and l keys that programmers are accustomed to.

![On-Screen Forms](screenshots/iloan.png)

**_Pick_** - Lists objects for user selection

**_View_** - A pager for viewing files. View has Unicode support, line numbering, regular expression searching, and a large virtual pad for horizontal scrolling. View works great with tree-sitter, source-highlight, pygments, bat, manual pages, and other syntax highlighters. View doesn't alter the file you are viewing. It uses the highlighter in a pipe, and reads the output, so the original file is never changed. And, if you happen to have a file that has been highlighted by another application, view can strip the ANSI codes for convenient editing. View is lightning fast, especially with huge log files.

Why is View so fast? Even if an application has a super-fast buffering scheme, it still has to wait on the kernel to provide data, and then copy data into it's own buffers, duplicating work the Kernel has already done. Why waste the time and memory? To be fair, it may be that many applications were written before direct accesses to the Kernel's demand paged virtual address space was available. Whatever the reason, View takes advantage of direct-to-Kernel memory mapped files to achieve maximum performance, reliability, and resource economy. If you work with large datasets, you will love view. No fluff, no bloat, no nonsense, just blazing fast performance.

![C-Menu View with Syntax Highlighting](screenshots/tree-sitter5.png)

**_RSH_** - RSH provides an alternative to su and sudo for executing commands with elevated privileges. It allows developers and system administrators to get in and out of root shells and execute commands with root privileges without the need for a password, for example, by authenticating with an ssh key as you would on gethub.

In the following example, make install requires root privilege, so the user types xx, is authenticated with an ssh key, and then types make install. When the make install is finished, the user types x to exit the root shell and relinquish root privilege.

![RSH SSH Authentication](screenshots/Makefile-out.png)

- The Green prompt indicates user privilege, and Red indicates root privilege.

**_lf_** - A "regular expression" file finder that's a smaller, easier-to-use, and much faster alternative to the Unix find command.

![lf help](screenshots/lf-help.png)

The screenshot above is the help output of lf piped through bat and displayed in
View.

![lf File Finder](screenshots/lf-dates.png)

The screenshot above is an example of how you might use the date-time options of lf to list files between two date-times (after and before) and the sample output. We hope you find the format format to be intuitive and easy to use.

The following is an actual benchmark of execution times for lf and find. The find and lf commands, approximate common usage, and produce identical results.

```bash
time find . -maxdepth 9 -type f -regex '.*\.[ch]$' > find.out

real    0m0.975s
user    0m0.728s
sys     0m0.243s

time lf -a -d 9 -t f '.*\.[ch]$' > lf.out

real    0m0.632s
user    0m0.425s
sys     0m0.207s
```

Verify that the output files are identical:

```bash
wc -l find.out lf.out

  2489 find.out
  2489 lf.out
  4978 total
```

The results show that lf is about 35% faster than find in this benchmark, and the output files are identical.

Next, we will add "-exec ls -l {} \;", a common use of find, and see how that affects performance. The resulting benchmarks are so extreme, they may strain credulity at first, but they are real and easily reproducible. Just run lf and find commands on your system in a variety of directories.

Note 1: The -a (List hidden files) has been replaced with -n (Don't list hidden files), so that the default behavior is more like find, thus mitigating the probability of confusion.

Note 2: The default maximum depth for lf was 3 as that was convenient for development, but otherwise it didn't make sense. The default maximum depth for lf is now 0, which means no limit, and thus more consistent with find.

```bash
time find . -maxdepth 5 -type f -exec -l {} \; >find.out
time lf -a -d 5 -t f | xargs ls -l >lf.out
```

```bash
wc -l find.out lf.out
```

![lf File Finder](screenshots/lf-vs-find2.png)

**_API_** - C-Menu provides a simple and consistent API for creating menu-driven user interfaces in C. The API includes tools specific to C-Menu, but also many general purpose tools that can be used in a wide range of applications. The API documentation is available in html and integrated into Neovim's completion engine, making it easy for developers to learn and use the API effectively.

![C-Menu Completions in Neovim](screenshots/api-help1.png)

All of the C-Menu binaries, including executables and libcm.so are less than 350k, a tiny footprint for such powerful tools, and no GUI is required. The only dependencies are the GNU C Library, GNU Math Library, NCursesw, and a terminal emulator.

Oh, and C-Menu is free, distributed under the MIT License.

Are you ready to get started? Below, you will find several options for installing C-Menu on your Linux system. I haven't yet provided a packaged binary distribution, but that will be coming soon in version 0.3.0.

Choose the option that best suits your needs and follow the instructions to get C-Menu up and running on your system.

---

## Other documentation

- [Comprehensive HTML Documentation](https://decision-inc.com)

- [API Reference](docs/API.md)
- [Augmentation](docs/extras.md)
- [CHANGELOG](docs/CHANGELOG.md)
- [Exercises](docs/exercises.md)
- [Frequently Asked Questions](docs/FAQ.md)
- [Overview](docs/OVERVIEW.md)
- [ROADMAP](docs/ROADMAP.md)
- [User Guide](docs/C-Menu-UG.md)

---

## C-Menu Binaries for Linux x86_64

1. Download the binary distribution, C-Menu-0.2.9-Linux-x86_64.xz.

2. cd to the directory where you downloaded the file and extract it using the following command:

```bash
tar -xf C-Menu-0.2.9-Linux-x86_64.xz
```

This will create a directory named menuapp containing the extracted files.

3. Configure your environment to use the C-Menu binaries and libraries:

Prepend the C-Menu bin directory to your PATH environment variable by adding the following line to your shell profile (e.g., ~/.bashrc or ~/.zshrc). Assuming you extracted the menuapp directory to your home directory, the line would look like this:

```bash
export PATH="$HOME"/menuapp/bin:"$PATH"
```

4. Start C-Menu by running the following command in your terminal:

```bash
menu
```

## Build C-Menu from Source

```bash
gh repo clone BillWaller/C-Menu
```

2️⃣ Copy the menuapp directory to your desired location:

```bash
cp -r C-Menu/src/menuapp /home/yourusername/
```

---

#### RSH Static Linking

C-Menu uses dynamic linking by default, but if you plan to use rsh in a rescue environment where dynamic linking may not be available, you can statically
link rsh during the build. To do this, set the `RSH_LD` environment variable to `-static` before building C-Menu:

```bash
export RSH_LD="-static"
```

CMake or Makefile will strip symbols from the executable once it has been copied
to its destination directory. This is done to reduce the size of the executable and improve performance.

**_NOTE_** If you choose to statically link rsh, make sure that your C compiler and linker support static linking and that you have the necessary static libraries,
specifically, libc.a, installed on your system. Static linking can increase the size of the executable and may have implications for compatibility and security, so be sure to test the statically linked version of C-Menu in your target environment.

Most distributions provide static libraries for the GNU C Library (glibc) as part of their development packages. You may need to install additional packages to obtain these static libraries, such as `glibc-static` or `libc6-dev` for glibc.

---

#### Prerequisites

- One of:
  - CMake 3.20 or higher, or
  - GNU Make 4.3 or higher
- A C compiler that supports C23 or later (e.g., GCC 13 or later, Clang 15 or later)
- NCursesw development libraries 6.5 or later
- GNU GLIBC development files
- GNU Math Library (libm) development files

---

#### Option 1 - Build C-Menu Using CMake Directly

1️⃣ Navigate to the C-Menu/src directory, create a build directory, and
cd into it:

```bash
cd C-Menu/src
mkdir build
cd build
```

2️⃣ Configure the project using CMake, specifying the installation prefix and build type:

```bash
cmake -DCMAKE_INSTALL_PREFIX="$HOME"/menuapp -DCMAKE_BUILD_TYPE=Release ..
```

3️⃣ Build the project using:

```bash
make
```

4️⃣ . If you want to use rsh in setuid mode, you must install C-Menu with root
privileges.

```bash
sudo make install
```

go to [Finish the installation](#finish-the-installation) below to complete the installation process.

---

#### Option 2 - Build C-Menu with Provided Scripts

1️⃣ Navigate to the C-Menu/build directory and run the provided build script:

```bash
cd C-Menu/build
./build.sh
```

2️⃣ Assume root privileges to install C-Menu:

```bash
sudo ./install.sh
```

go to [Finish the installation](#finish-the-installation) below to complete the installation process.

---

#### Option 3 - Build C-Menu Using Makefile

1️⃣ . Navigate to the C-Menu/src directory and edit the provided Makefile
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

2️⃣ Build the project using:

```bash
make
```

3️⃣ Assume root privileges to install C-Menu:

```bash
sudo make install
```

Continue with [Finish the installation](#finish-the-installation) below to complete the installation process.

---

### Finish the installation

1️⃣ Vrify that the C-Menu libraries and binaries have been installed to the correct directories (e.g., /home/yourusername/menuapp/lib64 and /home/yourusername/menuapp/bin) and that the permissions are set correctly.

```bash
ls -l "$HOME"/menuapp/lib64 "$HOME"/menuapp/bin
```

![Directory Listing](screenshots/postmakels.png)

2️⃣ Register the C-Menu libraries with the dynamic linker by running the following command:

```bash
sudo ldconfig -v "$HOME"/menuapp/lib64
```

3️⃣ Add the C-Menu bin directory to your PATH environment variable by adding the following line to your shell profile (e.g., ~/.bashrc or ~/.zshrc):

```bash
export PATH="/home/yourusername/menuapp/bin:"$PATH"
```

(replace /home/yourusername with the actual path to your menuapp directory) and save the file. 😆

4️⃣ Copy the sample minitrc from the C-Menu/menuapp directory to your home directory:

```bash
cp "$HOME"/menuapp/minitrc "$HOME"/.minitrc
```

5️⃣ Edit the ~/.minitrc file to customize your C-Menu configuration as needed.

```bash
vi ~/.minitrc
```

6️⃣ Source your shell profile to apply the changes to your PATH:

```bash
source ~/.bashrc
```

7️⃣ Start C-Menu by running the following command in your terminal:

```baah
menu
```

![C-Menu Running](screenshots/mainmenu.png)

## 🐸 Enjoy using C-Menu! If you encounter any issues or have questions, feel free to open an issue on the C-Menu GitHub repository.
