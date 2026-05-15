# lf notes

```bash
#!/bin/bash
# @name findperf
# @desc Compare the performance of fd, find, and lf with -H -L options
# @usage findperf <directory>

if [ -z "$1" ]; then
    echo "Usage: $0 <directory>"
    exit 1
fi
echo
echo
echo fd: -H inlude hidden files, -L follow links, -I no ignore files
files="$(/usr/bin/time -f "%C (%euser)" fd . --full-path $1 -H -L -I | wc -l)"
echo "$files" files
echo
echo find: -H inlude hidden files, -L follow links
files="$(/usr/bin/time -f "%C (%euser)" find -H -L $1 | wc -l)"
echo "$files" files \(may need to subtract 1 for base path\)
echo
echo lf: -H inlude hidden files, -L follow links, -T threads
files="$(/usr/bin/time -f '%C (%euser)' lf $1 -H -L -T 6 | wc -l)"
echo "$files" files
echo
```

## lf Without MMAP

### Test 1 Without MMAP

```bash
bill@bw1(1)src▶findperf /home/bill
```

Output:

fd: -H inlude hidden files, -L follow links, -I no ignore files
fd . --full-path /home/bill -H -L -I (0.10user)
347608 files

find: -H inlude hidden files, -L follow links
find -H -L /home/bill (0.70user)
347609 files (may need to subtract 1 for base path)

lf: -H inlude hidden files, -L follow links, -T threads
lf /home/bill -H -L -T 6 (0.21user)
347608 files

### Test 2 Without MMAP

```bash
bill@bw1(1)src▶findperf rustlings
```

fd: -H inlude hidden files, -L follow links, -I no ignore files
fd . --full-path rustlings -H -L -I (0.01user)
4648 files

find: -H inlude hidden files, -L follow links
find -H -L rustlings (0.01user)
4649 files (may need to subtract 1 for base path)

lf: -H inlude hidden files, -L follow links, -T threads
lf rustlings -H -L -T 6 (0.00user)
4648 files

## lf With MMAP

### Test 1

```bash
bill@bw1(1)src▶findperf /home/bill
```

fd: -H inlude hidden files, -L follow links, -I no ignore files
fd . --full-path /home/bill -H -L -I (0.10user)
347615 files

find: -H inlude hidden files, -L follow links
find -H -L /home/bill (0.70user)
347616 files (may need to subtract 1 for base path)

lf: -H inlude hidden files, -L follow links, -T threads
lf /home/bill -H -L -T 6 (0.26user)
324825 files

```
lf is short 22790 files
```

```bash
bill@bw1(2)src▶lf rustlings -H -L -T 6 | sort >lf-without-mmap.s
```

### Test 2

```bash
bill@bw1(1)src▶findperf rustlings
```

fd: -H inlude hidden files, -L follow links, -I no ignore files
fd . --full-path rustlings -H -L -I (0.01user)
4648 files

find: -H inlude hidden files, -L follow links
find -H -L rustlings (0.01user)
4649 files (may need to subtract 1 for base path)

lf: -H inlude hidden files, -L follow links, -T threads
lf rustlings -H -L -T 6 (0.00user)
4637 files

```
lf -s short 11 files
```

```bash
bill@bw1(2)src▶lf rustlings -H -L -T 6 | sort >lf-with-mmap.s
```

## lf rustlings

```bash

```

```diff
--- lf-without-mmap.s 2026-05-14 22:33:33.730073453 -0500
+++ lf-with-mmap.s 2026-05-14 22:30:08.726299272 -0500
@@ -308,2 +307,0 @@
-rustlings/target/debug/clippy1
-rustlings/target/debug/clippy1.d
@@ -631 +628,0 @@
-rustlings/target/debug/enums2
@@ -637 +633,0 @@
-rustlings/target/debug/errors4
@@ -4564 +4559,0 @@
-rustlings/target/debug/intro1
@@ -4570 +4564,0 @@
-rustlings/target/debug/iterators3
@@ -4597 +4590,0 @@
-rustlings/target/debug/primitive_types5.d
@@ -4610 +4602,0 @@
-rustlings/target/debug/strings2
@@ -4618 +4609,0 @@
-rustlings/target/debug/structs2
@@ -4632 +4622,0 @@
-rustlings/target/debug/variables2
@@ -4642 +4631,0 @@
-rustlings/target/debug/vecs1
```

```bash
cd rustlings/target/debug
ls -l
```

Output

```
drwxr-xr-x bill bill 4.0 KB Mon Apr  6 16:08:43 2026  build
.rwxr-xr-x bill bill 4.2 MB Fri Apr 24 15:28:29 2026  clippy1
.rw-r--r-- bill bill  95 B  Fri Apr 24 15:28:29 2026  clippy1.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:29 2026  clippy2
.rw-r--r-- bill bill  95 B  Fri Apr 24 15:28:29 2026  clippy2.d
.rwxr-xr-x bill bill 4.2 MB Fri Apr 24 15:28:29 2026  clippy3
.rw-r--r-- bill bill  95 B  Fri Apr 24 15:28:29 2026  clippy3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:28 2026  cow1
.rw-r--r-- bill bill  97 B  Fri Apr 24 15:28:28 2026  cow1.d
drwxr-xr-x bill bill  36 KB Thu Apr 30 17:12:40 2026  deps
.rwxr-xr-x bill bill 4.1 MB Sun Apr 26 12:42:42 2026  enums1
.rw-r--r-- bill bill  92 B  Sun Apr 26 12:42:42 2026  enums1.d
.rwxr-xr-x bill bill 4.2 MB Sun Apr 26 12:45:42 2026  enums2
.rw-r--r-- bill bill  92 B  Sun Apr 26 12:45:25 2026  enums2.d
.rwxr-xr-x bill bill 4.1 MB Sun Apr 26 22:21:15 2026  enums3
.rw-r--r-- bill bill  92 B  Fri Apr 24 15:28:23 2026  enums3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:25 2026  errors1
.rw-r--r-- bill bill 103 B  Fri Apr 24 15:28:25 2026  errors1.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:25 2026  errors4
.rw-r--r-- bill bill 103 B  Fri Apr 24 15:28:25 2026  errors4.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:26 2026  errors6
.rw-r--r-- bill bill 103 B  Fri Apr 24 15:28:26 2026  errors6.d
drwxr-xr-x bill bill 4.0 KB Mon Apr  6 16:08:43 2026  examples
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:49:56 2026  functions1
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:49:56 2026  functions1.d
.rwxr-xr-x bill bill 4.2 MB Wed Apr  8 12:50:33 2026  functions2
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:50:33 2026  functions2.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:51:30 2026  functions3
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:51:30 2026  functions3.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:53:58 2026  functions4
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:53:58 2026  functions4.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:54:41 2026  functions5
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:54:41 2026  functions5.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:26 2026  generics2
.rw-r--r-- bill bill 101 B  Fri Apr 24 15:28:26 2026  generics2.d
.rwxr-xr-x bill bill 4.1 MB Mon Apr 27 16:25:47 2026  hashmaps1
.rw-r--r-- bill bill 101 B  Mon Apr 27 16:24:52 2026  hashmaps1.d
.rwxr-xr-x bill bill 4.1 MB Thu Apr 30 12:16:25 2026  hashmaps2
.rw-r--r-- bill bill 101 B  Fri Apr 24 15:28:25 2026  hashmaps2.d
.rwxr-xr-x bill bill 4.1 MB Thu Apr 30 17:02:57 2026  hashmaps3
.rw-r--r-- bill bill 101 B  Fri Apr 24 15:28:25 2026  hashmaps3.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:55:41 2026  if1
.rw-r--r-- bill bill  83 B  Wed Apr  8 12:55:41 2026  if1.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:58:45 2026  if2
.rw-r--r-- bill bill  83 B  Wed Apr  8 12:57:09 2026  if2.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:49:33 2026  if3
.rw-r--r-- bill bill  83 B  Fri Apr 17 07:49:33 2026  if3.d
drwxr-xr-x bill bill  12 KB Thu Apr 30 17:11:34 2026  incremental
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:23 2026  intro1
.rw-r--r-- bill bill  92 B  Mon Apr  6 16:08:43 2026  intro1.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:23 2026  intro2
.rw-r--r-- bill bill  92 B  Mon Apr  6 16:11:06 2026  intro2.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:28 2026  iterators1
.rw-r--r-- bill bill 104 B  Fri Apr 24 15:28:28 2026  iterators1.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:28 2026  iterators3
.rw-r--r-- bill bill 104 B  Fri Apr 24 15:28:28 2026  iterators3.d
.rwxr-xr-x bill bill 4.2 MB Mon Apr 27 16:09:29 2026  modules1
.rw-r--r-- bill bill  98 B  Mon Apr 27 16:09:29 2026  modules1.d
.rwxr-xr-x bill bill 4.1 MB Mon Apr 27 16:20:40 2026  modules2
.rw-r--r-- bill bill  98 B  Mon Apr 27 16:20:08 2026  modules2.d
.rwxr-xr-x bill bill 4.2 MB Mon Apr 27 16:23:08 2026  modules3
.rw-r--r-- bill bill  98 B  Mon Apr 27 16:22:52 2026  modules3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 08:11:29 2026  move_semantics1
.rw-r--r-- bill bill 119 B  Fri Apr 17 08:11:29 2026  move_semantics1.d
.rwxr-xr-x bill bill 4.1 MB Sat Apr 18 07:22:27 2026  move_semantics2
.rw-r--r-- bill bill 119 B  Sat Apr 18 07:18:49 2026  move_semantics2.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:23 2026  move_semantics3
.rw-r--r-- bill bill 119 B  Fri Apr 24 13:54:27 2026  move_semantics3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 14:23:59 2026  move_semantics4
.rw-r--r-- bill bill 119 B  Fri Apr 24 13:54:31 2026  move_semantics4.d
.rwxr-xr-x bill bill 4.2 MB Fri Apr 24 14:27:50 2026  move_semantics5
.rw-r--r-- bill bill 119 B  Fri Apr 24 14:27:23 2026  move_semantics5.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:51:36 2026  primitive_types1
.rw-r--r-- bill bill 122 B  Fri Apr 17 07:51:36 2026  primitive_types1.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:53:39 2026  primitive_types2
.rw-r--r-- bill bill 122 B  Fri Apr 17 07:53:34 2026  primitive_types2.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:54:16 2026  primitive_types3
.rw-r--r-- bill bill 122 B  Fri Apr 17 07:54:16 2026  primitive_types3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:55:01 2026  primitive_types4
.rw-r--r-- bill bill 122 B  Fri Apr 17 07:54:21 2026  primitive_types4.d
.rwxr-xr-x bill bill 4.2 MB Fri Apr 17 07:56:04 2026  primitive_types5
.rw-r--r-- bill bill 122 B  Fri Apr 17 07:56:04 2026  primitive_types5.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:56:55 2026  primitive_types6
.rw-r--r-- bill bill 122 B  Fri Apr 17 07:56:15 2026  primitive_types6.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:50:39 2026  quiz1
.rw-r--r-- bill bill  89 B  Fri Apr 17 07:49:41 2026  quiz1.d
.rwxr-xr-x bill bill 4.1 MB Thu Apr 30 17:12:39 2026  quiz2
.rw-r--r-- bill bill  89 B  Fri Apr 24 15:28:25 2026  quiz2.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:27 2026  quiz3
.rw-r--r-- bill bill  89 B  Fri Apr 24 15:28:27 2026  quiz3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:28 2026  rc1
.rw-r--r-- bill bill  95 B  Fri Apr 24 15:28:28 2026  rc1.d
.rwxr-xr-x bill bill 4.2 MB Mon Apr 27 11:31:51 2026  strings1
.rw-r--r-- bill bill  98 B  Mon Apr 27 11:31:51 2026  strings1.d
.rwxr-xr-x bill bill 4.2 MB Mon Apr 27 11:37:34 2026  strings2
.rw-r--r-- bill bill  98 B  Mon Apr 27 11:37:30 2026  strings2.d
.rwxr-xr-x bill bill 4.1 MB Mon Apr 27 12:40:48 2026  strings3
.rw-r--r-- bill bill  98 B  Mon Apr 27 12:40:48 2026  strings3.d
.rwxr-xr-x bill bill 4.4 MB Mon Apr 27 12:45:46 2026  strings4
.rw-r--r-- bill bill  98 B  Mon Apr 27 12:45:46 2026  strings4.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 22:42:23 2026  structs1
.rw-r--r-- bill bill  98 B  Fri Apr 24 15:25:51 2026  structs1.d
.rwxr-xr-x bill bill 4.1 MB Sun Apr 26 09:54:58 2026  structs2
.rw-r--r-- bill bill  98 B  Fri Apr 24 15:28:24 2026  structs2.d
.rwxr-xr-x bill bill 4.1 MB Sun Apr 26 12:41:48 2026  structs3
.rw-r--r-- bill bill  98 B  Fri Apr 24 15:28:24 2026  structs3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:27 2026  tests1
.rw-r--r-- bill bill  92 B  Fri Apr 24 15:28:27 2026  tests1.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:27 2026  tests2
.rw-r--r-- bill bill  92 B  Fri Apr 24 15:28:27 2026  tests2.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:27 2026  tests3
.rw-r--r-- bill bill  92 B  Fri Apr 24 15:28:27 2026  tests3.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 24 15:28:26 2026  traits2
.rw-r--r-- bill bill  95 B  Fri Apr 24 15:28:26 2026  traits2.d
.rwxr-xr-x bill bill 4.1 MB Mon Apr  6 16:33:15 2026  variables1
.rw-r--r-- bill bill 104 B  Mon Apr  6 16:12:11 2026  variables1.d
.rwxr-xr-x bill bill 4.1 MB Mon Apr  6 17:48:52 2026  variables2
.rw-r--r-- bill bill 104 B  Mon Apr  6 17:48:52 2026  variables2.d
.rwxr-xr-x bill bill 4.1 MB Mon Apr  6 17:49:47 2026  variables3
.rw-r--r-- bill bill 104 B  Mon Apr  6 17:49:47 2026  variables3.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:46:18 2026  variables4
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:46:18 2026  variables4.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:47:37 2026  variables5
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:47:37 2026  variables5.d
.rwxr-xr-x bill bill 4.1 MB Wed Apr  8 12:49:12 2026  variables6
.rw-r--r-- bill bill 104 B  Wed Apr  8 12:49:08 2026  variables6.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 07:59:28 2026  vecs1
.rw-r--r-- bill bill  89 B  Fri Apr 17 07:59:28 2026  vecs1.d
.rwxr-xr-x bill bill 4.1 MB Fri Apr 17 08:10:02 2026  vecs2
.rw-r--r-- bill bill  89 B  Fri Apr 17 08:10:02 2026  vecs2.d
```
