# Test lf4 vs find

## Empty Directory

empty directory: PASS

---

## Small Directory - 500+ files


lf4 found 558 files


find found 558 files


no differences

---

## Large Directory - 500,000+ files

### lf4

7.11user 5.66system 0:12.11elapsed 105%CPU (0avgtext+0avgdata 37660maxresident)k
0inputs+0outputs (0major+1170minor)pagefaults 0swaps

lf4 found 516898 valid files
plus 3 files with invalid inodes:
STAT_FAIL,/home/bill/.thunderbird/z1w8j69z.default-esr/lock,No such file or directory
STAT_FAIL,/home/bill/.config/mozilla/firefox/CFmctgAX.Profile 2/lock,No such file or directory
STAT_FAIL,/home/bill/.config/mozilla/firefox/yqxm5v5q.default-release-1/lock,No such file or directory

### find

0.42user 0.48system 0:13.22elapsed 6%CPU (0avgtext+0avgdata 35252maxresident)k
0inputs+0outputs (0major+11919minor)pagefaults 0swaps

find found 516901 files

---

## Summary lf4 vs find

#### differences found

find found 3 more files than lf
lf4 rejected 3 files with invalid inodes
find2:  /home/bill/.config/mozilla/firefox/CFmctgAX.Profile 2/lock
find2:  /home/bill/.config/mozilla/firefox/yqxm5v5q.default-release-1/lock
find2:  /home/bill/.thunderbird/z1w8j69z.default-esr/lock

---

## End of Test

output files are lf3.out and find2.out
