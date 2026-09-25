#!/bin/bash
# lf_tests.sh

directory="/home/bill"
echo
echo "# lf test - using find and fd for baseline comparison"
echo "## directory: $directory"
echo
echo "### find"
echo find "$directory"
/bin/time -o time.out find "$directory" | wc -l >find.out 2>/dev/null
grep -v "^Command" time.out
echo "find found "$(cat find.out)" files"
echo
echo "### fd"
echo
echo fd . -H -I "$directory"
/bin/time -o time.out fd . -H -I "$directory" | wc -l >fd.out 2>/dev/null
grep -v "^Command" time.out
echo "fd found "$(cat fd.out)" files"
echo
echo "### lf"
echo
echo ./lf -H "$directory"
/bin/time -o time.out ./lf -H -T8 "$directory" | wc -l >lf.out 2>/dev/null
grep -v "^Command" time.out
echo "lf found "$(cat lf.out)" files"
