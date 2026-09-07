#!/bin/bash
# test lf

termination_status="PASS"

# Empty directory
mkdir -p /tmp/lf_tests
./lf /tmp/lf_tests
rc="$?"
if [ "$rc" = "0" ]; then
    echo "empty directory: PASS"
else
    echo "empty directory: FAIL"
    termination_status="FAIL"
fi
rm lf.out lf1.out lf2.out find1.out find2.out \
    diff1.out diff2.out tmp.out tmp1.out tmp2.out >/dev/null 2>&1
# Compare Output to Find - small directory (<300 files)
directory="."
echo "========================== $directory =========================="
echo Running lf
touch find1.out
./lf -H -S "$directory" | sed 's/\/$//' >lf1.out 2>/dev/null
lf_found=$(wc -l lf1.out | sed 's/ .*//')
eval $(./lf -H -T6 "$directory" >/dev/null | sed 's/: /=/')
echo "lf complete, found $lf_found files"
echo "----------------------------------------------------------------"
echo Running find
find "$directory" | sed 's/^\.\///
    /^\.$/d' | sort >find1.out 2>/dev/null
find_found=$(wc -l find1.out | sed 's/ .*//')
echo "find complete, found $find_found files"
echo "----------------------------------------------------------------"
diff lf1.out find1.out >tmp1.out 2>/dev/null
rc="$?"
grep "^[<>]" tmp1.out | sed 's/^</lf1: /; s/^>/find1: /' >diff1.out 2>&1
rm -f tmp1.out
if [ "$rc" = "0" ]; then
    echo "no differences: PASS"
else
    echo "differences found: FAIL"
    diffs_found=$(wc -l diff1.out | sed 's/ .*//')
    echo "differences found $diffs_found"
    cat diff1.out
    echo "output files lf1.out and find1.out"
    termination_status="FAIL"
fi
directory="/home/bill"
./lf -H -c -T6 "$directory" >/dev/null 2>tmp2.out
eval "$(sed 's/: /=/' tmp2.out)"
Total=$(($Files + $Errors))
echo "================== $directory $Total ================="
echo Running lf
/bin/time -f "%e" ./lf -H -S -T7 "$directory" | sed 's/\/$//' >lf2.out
lf_found=$(wc -l lf2.out | sed 's/ .*//')
echo "lf complete, found $lf_found valid files"
./lf -H -D458 -S $directory 2>tmp3.out
grep -v "^Errors" tmp3.out >tmp4.out
pseudo_files="$(grep -v "^Errors" tmp4.out | wc -l)"
echo "    plus $pseudo_files pseudo_files with no valid location"
cat tmp4.out
echo "----------------------------------------------------------------"
echo Running find
/bin/time -f "%e" find "$directory" | sed 's/^\.\///
    /^\.$/d' | sort | grep -v "^$directory$" >find2.out
find_found=$(wc -l find2.out | sed 's/ .*//')
echo "find complete, found $find_found files"
echo "----------------------------------------------------------------"
diff lf2.out find2.out >tmp4.out 2>&1
rc="$?"
if [ "$rc" = "0" ]; then
    echo "no differences: PASS"
else
    echo "differences found: FAIL"
    grep "^[<>]" tmp4.out | sed 's/^</lf2: /; s/^>/find2: /' >diff2.out 2>&1
    diffs_found=$(wc -l diff2.out | sed 's/ .*//')
    cat diff2.out
    Found=$(wc -l diff2.out | sed 's/ .*//')
    if [ "$lf_found" -gt "$find_found" ]; then
        difference=$((lf_found - find_found))
        echo "lf found $difference more files than find"
    else
        difference=$((find_found - lf_found))
        echo "find found $difference more files than lf - listed above"
    fi
    termination_status="FAIL"
fi
# lf -H -D458 -S
#     -H  Show hidden files
#     -S  Sort output
#     -D4 Errors
#       5 Bad Links
#       8 Only Report Errors
echo lf rejected "$Errors because the file location was invalid"
pseudo_files=$(wc -l diff2.out)
echo "----------------------------------------------------------------"
echo
echo "output files lf2.out and find2.out"
echo "termination_status: $termination_status"
