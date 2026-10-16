#!/usr/bin/env bash
# lf3_test.sh

directory="/home/bill"
echo
echo ./lf -H "$directory"
./lf -H "$directory" | LC_ALL="C" sort --parallel=8 -S 4G >lf.out 2>/dev/null
echo "lf found "$(wc -l lf.out)" files"
echo
echo ./lf3 -D3 -H "$directory"
./lf3 -H -D3 "$directory" >lf3.out 2>lf3.err
LC_ALL="C" sort --parallel=8 -S 4G lf3.out >x
mv x lf3.out
echo "lf3 found "$(wc -l lf3.out)" files"
