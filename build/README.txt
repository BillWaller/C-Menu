2025-11-06T20:59:41-06:00 Bill Waller billxwaller@gmail.com

CMAKE build instructions:

1. Install CMake (version 3.10 or higher) from https://cmake.org/download/
2. Navigate to the directory where you found this README.txt file
3. Build the project:

   cmake --build . >make.out 2>&1

[  2%] Building C object CMakeFiles/cmenu.dir/curskeys.c.o
[  5%] Building C object CMakeFiles/cmenu.dir/dwin.c.o
[  8%] Building C object CMakeFiles/cmenu.dir/exec.c.o
[ 11%] Building C object CMakeFiles/cmenu.dir/fform.c.o
[ 13%] Building C object CMakeFiles/cmenu.dir/fields.c.o
[ 16%] Building C object CMakeFiles/cmenu.dir/futil.c.o
[ 19%] Building C object CMakeFiles/cmenu.dir/init.c.o
[ 22%] Building C object CMakeFiles/cmenu.dir/init_view.c.o
[ 25%] Building C object CMakeFiles/cmenu.dir/mem.c.o
[ 27%] Building C object CMakeFiles/cmenu.dir/menu_engine.c.o
[ 30%] Building C object CMakeFiles/cmenu.dir/mview.c.o
[ 33%] Building C object CMakeFiles/cmenu.dir/opts.c.o
[ 36%] Building C object CMakeFiles/cmenu.dir/parse_menu_desc.c.o
[ 38%] Building C object CMakeFiles/cmenu.dir/pick_engine.c.o
[ 41%] Building C object CMakeFiles/cmenu.dir/scriou.c.o
[ 44%] Building C object CMakeFiles/cmenu.dir/sig.c.o
[ 47%] Building C object CMakeFiles/cmenu.dir/view_engine.c.o
[ 50%] Linking C static library libcmenu.a
[ 50%] Built target cmenu
[ 52%] Building C object CMakeFiles/ckeys.dir/ckeys.c.o
[ 55%] Linking C executable ckeys
[ 55%] Built target ckeys
[ 58%] Building C object CMakeFiles/form.dir/form.c.o
[ 61%] Linking C executable form
[ 61%] Built target form
[ 63%] Building C object CMakeFiles/iloan.dir/iloan.c.o
[ 66%] Linking C executable iloan
[ 66%] Built target iloan
[ 69%] Building C object CMakeFiles/menu.dir/menu.c.o
[ 72%] Linking C executable menu
[ 72%] Built target menu
[ 75%] Building C object CMakeFiles/pick.dir/pick.c.o
[ 77%] Linking C executable pick
[ 77%] Built target pick
[ 80%] Building C object CMakeFiles/view.dir/view.c.o
[ 83%] Linking C executable view
[ 83%] Built target view
[ 86%] Building C object CMakeFiles/whence.dir/whence.c.o
[ 88%] Linking C executable whence
[ 88%] Built target whence
[ 91%] Building C object CMakeFiles/enterchr.dir/enterchr.c.o
[ 94%] Linking C executable enterchr
[ 94%] Built target enterchr
[ 97%] Building C object CMakeFiles/enterstr.dir/enterstr.c.o
[100%] Linking C executable enterstr
[100%] Built target enterstr
