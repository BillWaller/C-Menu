# lf_tests.md

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

## Empty Directory

: PASS

## Small Directory

Running lf
lf complete, found 307 files

Running find
find complete, found 307 files

no differences: PASS

---

## Large directory 501863 files

Running lf
Errors: 7
Command exited with non-zero status 1
17.49
lf complete, found 501856 valid files
plus 7 files with invalid inodes

lf -H -D458 lists only directory entries that are invalid
It listed the missing files as follows:

```files
STAT_FAIL,/home/bill/.thunderbird/z1w8j69z.default-esr/lock,No such file or directory
STAT_FAIL,/home/bill/.config/google-chrome/SingletonCookie,No such file or directory
STAT_FAIL,/home/bill/.config/microsoft-edge-dev/SingletonCookie,No such file or directory
STAT_FAIL,/home/bill/.config/google-chrome/SingletonLock,No such file or directory
STAT_FAIL,/home/bill/.config/microsoft-edge-dev/SingletonLock,No such file or directory
STAT_FAIL,/home/bill/.config/mozilla/firefox/CFmctgAX.Profile 2/lock,No such file or directory
STAT_FAIL,/home/bill/.config/mozilla/firefox/yqxm5v5q.default-release-1/lock,No such file or directory
```

----------------------------------------------------------------

Running find
11.65
find complete, found 501863 files
----------------------------------------------------------------
differences found: FAIL

find listed the following files that lf rejected bacause of invalid inodes:

```files
find2:  /home/bill/.config/google-chrome/SingletonCookie
find2:  /home/bill/.config/google-chrome/SingletonLock
find2:  /home/bill/.config/microsoft-edge-dev/SingletonCookie
find2:  /home/bill/.config/microsoft-edge-dev/SingletonLock
find2:  /home/bill/.config/mozilla/firefox/CFmctgAX.Profile 2/lock
find2:  /home/bill/.config/mozilla/firefox/yqxm5v5q.default-release-1/lock
find2:  /home/bill/.thunderbird/z1w8j69z.default-esr/lock
```

reported 7 more files than lf, but lf rejected them because of invalid inodes

----------------------------------------------------------------

output files lf2.out and find2.out
termination_status: FAIL

----------------------------------------------------------------

Known problems

1 - lf is not as fast as find at 17.49 seconds compared to find at 11.65 seconds
This is no doubt, at least in part, due to the cyclic tracking

2 - lf considers files that point to nowhere an error, though these files no 
doubt use some unknown scheme for using their metadata to find network files, but it 
is not linux.

3 - lf doesn't report the top-level directory provided as the starting point for 
the find operation.
