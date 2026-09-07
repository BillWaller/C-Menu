# lf_tests.md
---

## Introduction

The following tests objectively compare the output of ls with find. The fact that there are only minor differences in the output of lf and find, on a directory structure with more than half a million files is a testament to the reliability of both lf and find. 

The differences in the output arise from the fact that lf is more stringent in
handling file metadata than find. lf rejects files with invalid inodes, while
find does not. lf also does not report the top-level directory provided as the
starting point for the find operation, while find does. If that capability is
desired, we may need to add it.

The files with invalid inodes are not necessarily bad files, but they are files that
point seemingly to nowhere. Some applications, such as Google, Mozilla, and
Microsoft use this type of file to store metadata about network files, but they are
not valid linux files. We may want to come up with a white list for such files,
but until we do, it's better that lf rejects files it cannot verify.

The following information is not verbatim output from lf_tests.sh, but edited
slightly for readability. The actual output is in lf_tests.txt.

---

## Empty Directory

```
empty directory: PASS
```

---

## Small Directory

```
========================== . ==========================
Running lf
lf complete, found 522 files
----------------------------------------------------------------
Running find
find complete, found 522 files
----------------------------------------------------------------
no differences: PASS

```

---

## Large Directory

```
================== /home/bill 507305 =================
Running lf
Errors: 3
Command exited with non-zero status 1
15.60
lf complete, found 507302 valid files
    plus 3 pseudo-files with invalid inodes
STAT_FAIL,/home/bill/.thunderbird/z1w8j69z.default-esr/lock,No such file or directory
STAT_FAIL,/home/bill/.config/mozilla/firefox/CFmctgAX.Profile 2/lock,No such file or directory
STAT_FAIL,/home/bill/.config/mozilla/firefox/yqxm5v5q.default-release-1/lock,No such file or directory
----------------------------------------------------------------
Running find
11.68
find complete, found 507305 files
----------------------------------------------------------------
differences found: FAIL
find found 3 more files than lf
lf rejected 3 files with invalid inodes
find2:  /home/bill/.config/mozilla/firefox/CFmctgAX.Profile 2/lock
find2:  /home/bill/.config/mozilla/firefox/yqxm5v5q.default-release-1/lock
find2:  /home/bill/.thunderbird/z1w8j69z.default-esr/lock
----------------------------------------------------------------

output files are lf2.out and find2.out
termination_status: FAIL

```

---
