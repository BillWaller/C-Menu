# CMENU BUILD INSTRUCTIONS FOR CMAKE:

by Bill Waller
billxwaller@gmail.com

1. Install CMake (version 3.10 or higher) from:

   [Cmake Website](https://cmake.org/download/)

2. Navigate to the directory where you found this README.txt file
   (C-Menu-0.2.5)

3. Build the project:

   cd build

   ./build.sh >build.out 2>&1

   sudo ./install.sh >install.out 2>&1

   ./clean.sh

4. After running the build.sh script, check the make.out file for build output.
   If the build was successful, you should see messages indicating that
   various targets were built successfully.

You should get the following output in make.out:

```
-- The C compiler identification is Clang 21.1.5
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/clang - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: /usr/local/src/C-Menu-0.2.5/build
[  2%] Building C object CMakeFiles/cmenu.dir/curskeys.c.o
[  5%] Building C object CMakeFiles/cmenu.dir/dwin.c.o
[  7%] Building C object CMakeFiles/cmenu.dir/exec.c.o
[ 10%] Building C object CMakeFiles/cmenu.dir/form_engine.c.o
[ 13%] Building C object CMakeFiles/cmenu.dir/fields.c.o
[ 15%] Building C object CMakeFiles/cmenu.dir/futil.c.o
[ 18%] Building C object CMakeFiles/cmenu.dir/init.c.o
[ 21%] Building C object CMakeFiles/cmenu.dir/init_view.c.o
[ 23%] Building C object CMakeFiles/cmenu.dir/mem.c.o
[ 26%] Building C object CMakeFiles/cmenu.dir/menu_engine.c.o
[ 28%] Building C object CMakeFiles/cmenu.dir/mview.c.o
[ 31%] Building C object CMakeFiles/cmenu.dir/opts.c.o
[ 34%] Building C object CMakeFiles/cmenu.dir/parse_menu_desc.c.o
[ 36%] Building C object CMakeFiles/cmenu.dir/pick_engine.c.o
[ 39%] Building C object CMakeFiles/cmenu.dir/scriou.c.o
[ 42%] Building C object CMakeFiles/cmenu.dir/sig.c.o
[ 44%] Building C object CMakeFiles/cmenu.dir/view_engine.c.o
[ 47%] Linking C static library libcmenu.a
[ 47%] Built target cmenu
[ 50%] Building C object CMakeFiles/ckeys.dir/ckeys.c.o
[ 52%] Linking C executable ckeys
[ 52%] Built target ckeys
[ 55%] Building C object CMakeFiles/form.dir/form.c.o
[ 57%] Linking C executable form
[ 57%] Built target form
[ 60%] Building C object CMakeFiles/enterchr.dir/enterchr.c.o
[ 63%] Linking C executable enterchr
[ 63%] Built target enterchr
[ 65%] Building C object CMakeFiles/enterstr.dir/enterstr.c.o
[ 68%] Linking C executable enterstr
[ 68%] Built target enterstr
[ 71%] Building C object CMakeFiles/iloan.dir/iloan.c.o
[ 73%] Linking C executable iloan
[ 73%] Built target iloan
[ 76%] Building C object CMakeFiles/menu.dir/menu.c.o
[ 78%] Linking C executable menu
[ 78%] Built target menu
[ 81%] Building C object CMakeFiles/pick.dir/pick.c.o
[ 84%] Linking C executable pick
[ 84%] Built target pick
[ 86%] Building C object CMakeFiles/rsh.dir/rsh.c.o
[ 89%] Linking C executable rsh
[ 89%] Built target rsh
[ 92%] Building C object CMakeFiles/view.dir/view.c.o
[ 94%] Linking C executable view
[ 94%] Built target view
[ 97%] Building C object CMakeFiles/whence.dir/whence.c.o
[100%] Linking C executable whence
[100%] Built target whence
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /usr/local/src/C-Menu-0.2.5/build
Build complete.
To install: sudo ./install.sh
```

5.  After running the install.sh script, the compiled binaries and
    library files will be installed to the appropriate system directories.
    You may need superuser (root) privileges to run the install.sh script.

You should get the following output in install.out:

```
-- Install configuration: ""
-- Installing: /uhome/uname/menuapp/bin/ckeys
-- Installing: /uhome/uname/menuapp/bin/form
-- Installing: /uhome/uname/menuapp/bin/enterchr
-- Installing: /uhome/uname/menuapp/bin/enterstr
-- Installing: /uhome/uname/menuapp/bin/iloan
-- Installing: /uhome/uname/menuapp/bin/menu
-- Installing: /uhome/uname/menuapp/bin/pick
-- Installing: /uhome/uname/menuapp/bin/rsh
-- Installing: /uhome/uname/menuapp/bin/view
-- Installing: /uhome/uname/menuapp/bin/whence
```

You should see the following files installed in ~/menuapp/bin:

```
.rwxr-xr-x. uname users 139 KB Sat Nov 29 23:35:38 2025 ckeys
.rwxr-xr-x. uname users 308 KB Sat Nov 29 23:35:39 2025 menu
.rwxr-xr-x. uname users  29 KB Sat Nov 29 23:35:39 2025 iloan
.rwxr-xr-x. uname users 236 KB Sat Nov 29 23:35:39 2025 form
.rwxr-xr-x. uname users  73 KB Sat Nov 29 23:35:39 2025 enterstr
.rwxr-xr-x. uname users  23 KB Sat Nov 29 23:35:39 2025 enterchr
.rwxr-xr-x. uname users  24 KB Sat Nov 29 23:35:40 2025 whence
.rwxr-xr-x. uname users 196 KB Sat Nov 29 23:35:40 2025 view
.rws--x--x. root  root   22 KB Sat Nov 29 23:35:40 2025 rsh
.rwxr-xr-x. uname users 222 KB Sat Nov 29 23:35:40 2025 pick
```

6.  To clean up the build files, run the clean.sh script. This will remove the
    build directory and most of its contents excluding the build.sh, install.sh,
    and clean.sh, scripts.
