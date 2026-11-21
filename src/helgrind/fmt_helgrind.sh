#!/usr/bin/env bash

# NAME:        fmt_helgrnd.sh
# DESCRIPTION: format helgrind output
# AUTHOR:      Bill Waller <billxwaller@gmail.com>
# DATE:        2026-11-07
# VERSION:     1.0.0
#
# This script formats the output of Valgrind's Helgrind tool into a more
# readable format. It processes the helgrind.out file and generates a condensed
# report, optionally appending error counts or unique sequential file numbers to
# the output file name.
#
# USAGE:
#  ./fmt_helgrnd.sh [OPTIONS] [ARGUMENT]
#
# Options:
#   -h, --help     Display this help message and exit
#   -v, --version  Output version information and exit
#   -s, --seq      Append sequential number to file name
#   -e, --err      Append number of errors to file name
#   -t, --target   Specify the target program name
#
# INSTRUCTIONS:
#
# 1. Ensure you have Valgrind installed.
# 2. Run run your program with Helgrind:
#
#    valgrind --tool=helgrind --log-file=helgrind.out -s ./lf3 -H
#    or
#    make helgrind if you have a Makefile target for it.
#
# 3. Run this script to format the helgrind.out file:
#
#    fmt_helgrind.sh
#
# 4. The formatted output will be saved in a new file named helgrind_sN.out or
#    helgrind_eN_sN.out, where N is a sequential number or the number of errors,
#    respectively.
#
# ---------------------------------------------------------------
#
prepend_path() {
    case ":${PATH}:" in
    *:"$1":*) ;;
    *)
        PATH="$1:$PATH"
        ;;
    esac
}
prepend_path "./helgrind"
export PATH
#
usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] [ARGUMENT]

format helgrind.out

Options:
  -h, --help     Display this help message and exit
  -v, --version  Output version information and exit
  -e, --err      Append number of errors to file name
  -s, --seq      Append sequential number to file name

    Sequential is the default.

Example:
    $(basename "$0")
EOF
    exit 0
}

version() {
    echo "$(basename "$0") version 1.0.0"
    exit 0
}

parse_params() {
    paths=""
    re=""
    f_err=0
    f_seq=1
    while [[ $# -gt 0 ]]; do
        case "$1" in
        -h | --help)
            usage
            ;;
        -e | --err)
            f_err=1
            ;;
        -s | --seq)
            f_seq=1
            ;;
        -v | --version)
            version
            ;;
        -t | --target)
            target=$2
            ;;
        -*)
            echo "Error: Invalid option $1" >&2
            usage
            ;;
        *)
            shift
            ;;
        esac
    done
}

main() {
    parse_params "$@"
}

main "$@"
if [ ! -f helgrind.out ]; then
    echo "helgrind.out file not found!"
    exit 1
fi
target=$(sed -n '/Command:/s/^.*Command: \.\///; s/ -H$//p' helgrind.out)
newfile=helgrind
if [ "$f_err" -eq 1 ]; then
    enumber=$(sed -n 's/^.*ERROR SUMMARY: \([0-9]*\).*$/\1/p' helgrind.out | sed -n '1p')
    newfile="$newfile"_e"${snumber}"
fi
if [ "$f_seq" -eq 1 ]; then
    seq_old=0
    /bin/ls helgrind_s*.out >/dev/null 2>&1
    rc=$?
    if [ $rc -ne 0 ]; then
        seq_old=0
    else
        seq_old=$(/bin/ls helgrind_s*.out | sed 's/^helgrind_s//;s/\.out$//' | sort -n |
            tail -n 1)
    fi
    snumber=$((seq_old + 1))
    newfile="$newfile"_s"${snumber}"
fi
newfile="$newfile.out"
if [ "$target" = "" ]; then
    target=lf3
fi
target=$(cut -d. -f1 <<<"$target")
if [ "$enumber" != "" ]; then
    newbase=$target"_"$enumber
fi
if [ "$snumber" != "" ]; then
    newbase=$target"_"$snumber
fi
head -n 4 helgrind.out | sed 's/^==[0-9]*== //' >"$newfile"
echo "$(iso8601) Condensed Format by Bill Waller" >>"$newfile"
echo >>"$newfile"
echo "cntxt errors action size  at_addr by_addr fn                          thread  line" >>"$newfile"
echo "----- ------ ------ ---- -------- ------- ------------------------------ --- -----" >>"$newfile"
TMP_PASS1=h_pass1.tmp
TMP_RPT=h_rpt.tmp
TMP_ERRS=h_errs.tmp
TMP_CONFLICT1=h_conflict1.tmp
TMP_CONFLICT2=h_conflict2.tmp
TMP_INTERNAL=h_internal.tmp
grep -v -e "vg_replace" -e "hg_intercepts" -e "Locks held:" helgrind.out |
    sed -n '6,$p' |
    sed '/^$/d
    s/^..........//  
    /^---Thread.*/,/^--*$/d
    s/^   by 0x//
    s/^----------*$//
    s/Possible data race during //
    s/This conflicts with a previous //
    s/ of size / /
    s/ at // 
    s/ by thread \#/ /
    s/   by 0x//
    s/0x/ /
    s/://
    s/^ *//
    s/('$target'\.c:/ /
    s/)$//' |
    sed -n '/^read/,+1p
    /^write/,+1p
    /errors in context/p' >$TMP_PASS1
awk -f fh_rpt.awk $TMP_PASS1 >$TMP_RPT
awk -f fh_internal.awk $TMP_PASS1 >$TMP_INTERNAL
awk 'BEGIN {
        while (getline) {
            if ($0 == "" || $9 == "line" || $9 == "-----" || $9 == "") {
                continue
            }
            printf("%s %s\n", $9, $2)
        }
}' $TMP_INTERNAL | sort -n | awk -f fh_errs.awk >$TMP_ERRS
awk -f fh_conflict1.awk $TMP_PASS1 | sort -n | uniq >$TMP_CONFLICT1
sort -n $TMP_CONFLICT1 | awk -f fh_conflict2.awk >$TMP_CONFLICT2
echo >>"$TMP_CONFLICT2"
cat $TMP_RPT >>"$newfile"
echo >>"$newfile"
paste $TMP_ERRS $TMP_CONFLICT2 | sed 's/\t/ /g' >>"$newfile"
echo created "$newfile"
rm -f $TMP_PASS1 $TMP_RPT $TMP_ERRS $TMP_CONFLICT1 $TMP_CONFLICT2 $TMP_INTERNAL
cd ..
cp $target.c "$newbase.c"
echo copied "$target".c to "$newbase".c
cp "$target".h "$newbase".h
echo copied "$target".h to "$newbase".h
