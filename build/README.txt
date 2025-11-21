Bill Waller
billxwaller@gmail.com

CMAKE build instructions:

1. Install CMake (version 3.10 or higher) from https://cmake.org/download/
2. Navigate to the directory where you found this README.txt file
3. Build the project:

   cmake --build . >make.out 2>&1

You should get the following output in make.out:

[  2%] Building C object CMakeFiles/cmenu.dir/curskeys.c.o
[  5%] Building C object CMakeFiles/cmenu.dir/dwin.c.o
[  7%] Building C object CMakeFiles/cmenu.dir/exec.c.o
[ 10%] Building C object CMakeFiles/cmenu.dir/fform.c.o
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
