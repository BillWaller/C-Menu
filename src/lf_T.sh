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
echo "### lf"
echo
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
    echo lf -H -T"$i" "$directory"
    /bin/time -o time.out ./lf -H -T"$i" "$directory" | wc -l >lf.out 2>/dev/null
    grep -v "^Command" time.out
    echo "lf found "$(cat lf.out)" files"
    echo
    echo ------------------------------------
done
