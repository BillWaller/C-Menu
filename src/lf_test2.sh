#!/usr/bin/env bash
# lf_tests.sh

directory="/home/bill"
echo
echo "### lf"
echo
echo ./lf -H "$directory"
/bin/time -o time.out ./lf "$directory" | wc -l >lf.out 2>/dev/null
grep -v "^Command" time.out
echo "lf found "$(cat lf.out)" files"
echo
echo ./lf3 -H "$directory"
/bin/time -o time.out ./lf3 "$directory" | wc -l >lf3.out 2>/dev/null
grep -v "^Command" time.out
echo "lf3 found "$(cat lf3.out)" files"
