#!/bin/bash
# test lf

termination_status="PASS"

echo "# Test lf4 vs find"
echo
echo "## Empty Directory"
echo
mkdir -p /tmp/lf_tests
./lf4 /tmp/lf_tests
rc="$?"
if [ "$rc" = "0" ]; then
    echo "empty directory: PASS"
    echo
else
    echo "empty directory: FAIL"
    echo
fi
echo "---"
rm lf.out lf1.out lf2.out lf3.out find1.out find2.out \
    diff1.out diff2.out tmp.out tmp1.out tmp2.out >/dev/null 2>&1
directory="."
echo
echo "## Small Directory - 500+ files"
echo
touch find1.out
./lf4 -H "$directory" | sort | sed 's/\/$//' >lf1.out 2>/dev/null
lf4_found=$(wc -l lf1.out | sed 's/ .*//')
eval $(./lf4 -H -T6 "$directory" >/dev/null | sed 's/: /=/')
echo
echo "lf4 found $lf4_found files"
echo
find "$directory" | sed 's/^\.\///
    /^\.$/d' | sort >find1.out 2>/dev/null
find_found=$(wc -l find1.out | sed 's/ .*//')
echo
echo "find found $find_found files"
echo
diff lf1.out find1.out >tmp1.out 2>/dev/null
rc="$?"
grep "^[<>]" tmp1.out | sed 's/^</lf1: /; s/^>/find1: /' >diff1.out 2>&1
rm -f tmp1.out
echo
if [ "$rc" = "0" ]; then
    echo "no differences"
    echo
else
    diffs_found=$(wc -l diff1.out | sed 's/ .*//')
    echo "differences found $diffs_found"
    cat diff1.out
    echo "output files lf1.out and find1.out"
    echo
fi
echo "---"
echo
directory="/home/bill"
echo "## Large Directory - 500,000+ files"
echo
echo "### lf4"
echo
/bin/time -o time.out ./lf4 -H -T7 "$directory" 2>/dev/null | sort | sed 's/\/$//' >lf4.out
grep -v "^Command" time.out
echo
lf_found=$(wc -l lf4.out | sed 's/ .*//')
echo "lf4 found $lf_found valid files"
./lf4 -H -D458 $directory 2>tmp3.out
lfRejected="$(grep -v Errors tmp3.out | wc -l)"
echo "plus $lfRejected files with invalid inodes:"
grep -v "Errors" tmp3.out
echo
echo "### find"
/bin/time -o time.out find "$directory" | sed 's/^\.\///
    /^\.$/d' | sort | grep -v "^$directory$" >find2.out
echo
grep -v "^Command" time.out
echo
find_found=$(wc -l find2.out | sed 's/ .*//')
echo "find found $find_found files"
echo
echo "---"
echo
echo "## Summary lf4 vs find"
diff lf4.out find2.out >tmp4.out 2>&1
rc="$?"
echo
if [ "$rc" = "0" ]; then
    echo "#### no differences"
    echo
else
    echo "#### differences found"
    echo
    grep "^[<>]" tmp4.out | sed 's/^</lf3: /; s/^>/find2: /' >diff2.out 2>&1
    diffs_found=$(wc -l diff2.out | sed 's/ .*//')
    Found=$(wc -l diff2.out | sed 's/ .*//')
    if [ "$lf_found" -gt "$find_found" ]; then
        difference=$((lf_found - find_found))
        echo "lf4 found $difference more files than find"
    else
        difference=$((find_found - lf_found))
        echo "find found $difference more files than lf"
        echo lf4 rejected "$lfRejected" files with invalid inodes
    fi
    cat diff2.out
    echo
fi
echo "---"
echo
# lf4 -H -D458 -S
#     -H  Show hidden files
#     -S  Sort output
#     -D4 Errors
#       5 Bad Links
#       8 Only Report Errors
echo "## End of Test"
echo
echo "output files are lf3.out and find2.out"
