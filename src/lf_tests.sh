#!/bin/bash
# test lf

# The -S option of lf is only recommended for smaller lists of files. It
# actually invokes the linux sort command. However, by piping the output of lf
# through sort, you have the opportunity to specify options such as those below,
# tailoring sort to your specific needs and hardware. Performance gains by doing
# so are substantial, especially for large lists of files, and there is no
# downside.
#
# BEWARE: I have noted wierd behavior of diff recently, where it will report
# spurious differences in files that are actually identical according to "cmp".
#

directory="/home/bill"
echo
echo "# lf test - using find and fd for baseline comparison"
echo
echo "## directory: $directory"
echo
# ------------------------------------------------------------
echo "### find"
echo find "$directory"
/bin/time -o time.out find "$directory" >find.out 2>/dev/null
grep -v "^Command" time.out
LC_ALL=C sort --parallel=7 -S 4G find.out | sed '1,1d
    s/^\///
    /^\.$/d' >findb.out
found=$(wc -l findb.out | sed 's/ .*//')
echo "find found $found files"
echo
echo "---"
echo
# ------------------------------------------------------------
echo "### fd"
echo
echo fd . -H -I "$directory"
/bin/time -o time.out fd . -H -I "$directory" >fd.out 2>/dev/null
grep -v "^Command" time.out
LC_ALL=C sort --parallel=7 -S 4G fd.out | sed 's/^\///
    s/\/$//' >fdb.out
found=$(wc -l fdb.out | sed 's/ .*//')
echo "fd found $found files"
echo
echo "---"
echo
# ------------------------------------------------------------
echo "### lf"
echo
echo lf -H -T8 "$directory"
/bin/time -o time.out ./lf -H -T8 "$directory" >lf.out 2>/dev/null
grep -v "^Command" time.out
LC_ALL=C sort --parallel=7 -S 4G lf.out | sed 's/^\///
        s/\/$//' >lfb.out
found=$(wc -l lfb.out | sed 's/ .*//')
echo "lf found $found files"
echo
echo "---"
echo
