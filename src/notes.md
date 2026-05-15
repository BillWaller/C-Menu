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
