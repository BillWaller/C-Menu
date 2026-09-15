
# lf4 test - using find and fd for baseline comparison

## directory: /home/bill

### find
find /home/bill
0.37user 0.48system 0:00.86elapsed 99%CPU (0avgtext+0avgdata 35076maxresident)k
0inputs+105400outputs (0major+12106minor)pagefaults 0swaps
find found 517821 files

---

### fd

fd . -H -I /home/bill
0.69user 0.61system 0:00.15elapsed 874%CPU (0avgtext+0avgdata 159452maxresident)k
0inputs+105464outputs (0major+4233minor)pagefaults 0swaps
fd found 517821 files

---

### lf4

lf4 -H -T7 /home/bill
0.15user 0.34system 0:00.08elapsed 607%CPU (0avgtext+0avgdata 38288maxresident)k
0inputs+105464outputs (0major+1482minor)pagefaults 0swaps
lf4 found 517821 files

---

