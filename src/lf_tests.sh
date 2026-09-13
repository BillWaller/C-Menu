#!/bin/bash
# test lf

directory="/home/bill"
echo "## Large Directory - 500,000+ files"
echo
# ------------------------------------------------------------
echo "### lf4"
echo
/bin/time -o time.out ./lf4 -H -T7 "$directory" >lf4.out 2>/dev/null
sort lf4.out | sed 's/\/$//' >lf4b.out
grep -v "^Command" time.out
echo
lf_found=$(wc -l lf4b.out | sed 's/ .*//')
echo "lf4 found $lf_found files"
echo
echo "---"
echo
# ------------------------------------------------------------
echo "### fd"
echo
/bin/time -o time.out fd . -H -I "$directory" >fd4.out 2>/dev/null
sort fd4.out | sed 's/^\///
    s/\/$//' >fd4b.out
grep -v "^Command" time.out
echo
fd_found=$(wc -l fd4b.out | sed 's/ .*//')
echo "fd found $fd_found files"
echo
echo "---"
echo
# ------------------------------------------------------------
echo "### find"
/bin/time -o time.out find "$directory" >find2.out
sed 's/^\.\///
    /^\.$/d' find2.out | sort | grep -v "^$directory$" >find2b.out
echo
grep -v "^Command" time.out
echo
find_found=$(wc -l find2b.out | sed 's/ .*//')
echo "find found $find_found files"
echo
echo "---"
echo
