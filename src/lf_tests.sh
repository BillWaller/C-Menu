#!/bin/bash
# test lf

directory="/home/bill"
echo "## Large Directory - 500,000+ files"
echo
# ------------------------------------------------------------
echo "### find"
echo find
/bin/time -o time.out find "$directory" >find.out 2>/dev/null
grep -v "^Command" time.out
echo sort
/bin/time -o time.out sh -c 'LC_ALL=C sort --parallel=7 -S 4G find.out' | sed 's/^\///
/^\.$/d' >findb.out
grep -v "^Command" time.out
found=$(wc -l findb.out | sed 's/ .*//')
echo "find found $found files"
echo
echo "---"
echo
# ------------------------------------------------------------
echo "### fd"
echo
echo fd
/bin/time -o time.out fd . -H -I "$directory" >fd.out 2>/dev/null
grep -v "^Command" time.out
echo sort
/bin/time -o time.out sh -c 'LC_ALL=C sort --parallel=7 -S 4G fd.out' | sed 's/^\///
    s/\/$//' >fdb.out
grep -v "^Command" time.out
found=$(wc -l fdb.out | sed 's/ .*//')
echo "fd found $found files"
echo
echo "---"
echo
# ------------------------------------------------------------
echo "### lf4"
echo
echo lf4
/bin/time -o time.out ./lf4 -H -T7 "$directory" >lf4.out 2>/dev/null
grep -v "^Command" time.out
echo sort
/bin/time -o time.out sh -c 'LC_ALL=C sort --parallel=7 -S 4G lf4.out' |
    sed 's/^\///
        s/\/$//' >lf4b.out
grep -v "^Command" time.out
found=$(wc -l lf4b.out | sed 's/ .*//')
echo "lf4 found $found files"
echo
echo "---"
echo
